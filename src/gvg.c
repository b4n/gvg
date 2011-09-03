/*
 * Copyright 2011 Colomban Wendling <ban@herbesfolles.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "gvg.h"

#include <glib.h>
#include <gio/gio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "gvg-xml-parser.h"
#include "gvg-args-builder.h"


void
gvg_add_options (GvgArgsBuilder    *args,
                 const GvgOptions  *opts)
{
  gvg_args_builder_add_bool (args, "demangle", opts->demangle);
  gvg_args_builder_add_bool (args, "error-limit", opts->error_limit);
  gvg_args_builder_add_bool (args, "show-below-main", opts->show_below_main);
  gvg_args_builder_add_bool (args, "track-fds", opts->track_fds);
  gvg_args_builder_add_bool (args, "time-stamp", opts->time_stamp);
  gvg_args_builder_add_uint (args, "num-callers", opts->num_callers);
  gvg_args_builder_add_ssize_checked (args, "max-stackframe", opts->max_stackframe);
  gvg_args_builder_add_ssize_checked (args, "main-stacksize", opts->main_stacksize);
}

static gint
close_and_invalidate (gint *fd)
{
  gint ret = -1;
  
  if (*fd >= 0) {
    ret = close (*fd);
    *fd = -1;
  }
  
  return ret;
}

typedef struct _GvgChildData GvgChildData;

struct _GvgChildData {
  gint        ref_count;
  
  GPid        pid;
  gint        xml_pipe;
  GSource    *pipe_source;
  GIOChannel *pipe_channel;
  
  GvgXmlParser *parser;
};

static GvgChildData *
gvg_child_data_new (void)
{
  GvgChildData *cdata;
  
  cdata = g_slice_alloc (sizeof *cdata);
  cdata->ref_count    = 1;
  cdata->pid          = -1;
  cdata->xml_pipe     = -1;
  cdata->pipe_source  = NULL;
  cdata->pipe_channel = NULL;
  cdata->parser       = NULL;
  
  return cdata;
}

static GvgChildData *
gvg_child_data_ref (GvgChildData *cdata)
{
  g_atomic_int_inc (&cdata->ref_count);
  return cdata;
}

#ifdef G_OS_WIN32
# define INVALID_PID NULL
#else
# define INVALID_PID -1
#endif

/* terminates a process, giving it 5 seconds to terminate or kill it */
static void
terminate_child (GPid pid_)
{
#ifdef G_OS_WIN32
  /* FIXME: */
  g_warning ("don't know how to kill child on Windows");
#else
  pid_t pid = pid_;
  gint  pass;
  
  /* first, try to terminate the child.  if it fails, it may mean the child
   * already terminated, and anyway we can't recover the error so give up */
  if (kill (pid, SIGTERM) < 0) {
    return;
  }
  /* wait for up to 5 seconds for the process to die */
  for (pass = 0; pass < 10; pass ++) {
    if (waitpid (pid, NULL, WNOHANG) != 0) {
      return;
    }
    g_usleep (500000);
  }
  /* finally destroy the child if it still exists */
  g_debug ("child won't exit, killing it");
  kill (pid, SIGKILL);
#endif
}

static GIOChannel *
create_io_channel (gint fd)
{
  GIOChannel *channel;
  
#ifdef G_OS_WIN32
  channel = g_io_channel_win32_new_fd (fd);
#else
  channel = g_io_channel_unix_new (fd);
#endif
  g_io_channel_set_encoding (channel, NULL, NULL);
  
  return channel;
}

/* wrapper around g_poll() on an IO channel */
static gint
io_channel_poll (GIOChannel  *channel,
                 GIOCondition events,
                 gint         timeout)
{
  GPollFD fd = { -1, 0, 0 };
  
#ifdef G_OS_WIN32
  g_io_channel_win32_make_pollfd (channel, events, &fd);
#else
  fd.events = events;
  fd.fd = g_io_channel_unix_get_fd (channel);
#endif
  
  return g_poll (&fd, 1, timeout);
}

static gboolean
read_xml_pipe (GvgChildData  *cdata,
               gboolean       check)
{
  gchar buf[BUFSIZ];
  gsize len;
  
  do {
    GIOStatus status;
    
    /* we're asked to check whether the channel has available data */
    if (check) {
      gint ret;
      
      ret = io_channel_poll (cdata->pipe_channel, G_IO_IN, 0);
      if (ret < 0) {
        return FALSE;
      } else if (ret == 0) {
        return TRUE;
      }
    }
    
    do {
      len = 0u;
      status = g_io_channel_read_chars (cdata->pipe_channel, buf, sizeof buf,
                                        &len, NULL);
    } while (status == G_IO_STATUS_AGAIN);
    
    if (status == G_IO_STATUS_NORMAL) {
      gvg_xml_parser_push (cdata->parser, buf, len, FALSE);
      if (len == sizeof (buf)) {
        /* if we read a whole packet, we'll try again but we also need to
         * ensure we didn't read *all* the data, so we need to poll() */
        check = TRUE;
      }
    }
  } while (len == sizeof (buf));
  
  if (len == 0) {
    g_debug ("pipe end");
  }
  
  return len > 0;
}

static void
gvg_child_data_unref (GvgChildData *cdata)
{
  if (g_atomic_int_dec_and_test (&cdata->ref_count)) {
    if (cdata->pid != INVALID_PID) {
      terminate_child (cdata->pid);
      g_spawn_close_pid (cdata->pid);
      cdata->pid = INVALID_PID;
    }
    read_xml_pipe (cdata, TRUE);
    if (cdata->pipe_channel) {
      g_io_channel_shutdown (cdata->pipe_channel, TRUE, NULL);
      g_io_channel_unref (cdata->pipe_channel);
      cdata->pipe_channel = NULL;
    }
    if (cdata->pipe_source) {
      g_source_destroy (cdata->pipe_source);
      cdata->pipe_source = NULL;
    }
    close_and_invalidate (&cdata->xml_pipe);
    if (cdata->parser) {
      /* ensure the parser terminated */
      gvg_xml_parser_push (cdata->parser, NULL, 0, TRUE);
      gvg_xml_parser_free (cdata->parser);
    }
    
    g_slice_free1 (sizeof *cdata, cdata);
  }
}

static gboolean
xml_fd_in_ready (GIOChannel  *channel,
                 GIOCondition cond,
                 gpointer     data)
{
  GvgChildData *cdata = data;
  gboolean      keep  = TRUE;
  
  g_assert (cdata->pipe_channel == channel);
  
  if (cond & (G_IO_IN | G_IO_PRI)) {
    keep = read_xml_pipe (cdata, FALSE);
  }
  if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
    keep = FALSE;
  }
  
  if (! keep) {
    g_debug ("removing pipe watch");
  }
  return keep;
}

static void
watch_child (GPid     pid,
             gint     status,
             gpointer data)
{
  GvgChildData *cdata = data;
  
  g_debug ("child terminated");
  g_spawn_close_pid (pid);
  cdata->pid = INVALID_PID;
}

static gchar **
build_argv (const gchar **program_argv,
            gint          xml_fd,
            void        (*get_args) (GvgArgsBuilder *builder,
                                     gpointer        data),
            gpointer      data)
{
  GvgArgsBuilder *args = gvg_args_builder_new ();
  
  gvg_args_builder_add (args, "valgrind");
  /* FIXME: this options isn't supported by Valgrind < 3.6.1 */
  /*gvg_args_builder_add_string (args, "fullpath-after", "");*/
  gvg_args_builder_add_bool (args, "xml", TRUE);
  gvg_args_builder_add_arg (args, "xml-fd", "%d", xml_fd);
  gvg_args_builder_add (args, "-q");
  /* this avoids having wrong XML because of forked children (see Valgrind
   * manual man) */
  gvg_args_builder_add_string (args, "child-silent-after-fork", "yes");
  
  get_args (args, data);
  
  /* add program and NULL terminator */
  gvg_args_builder_add_args (args, program_argv);
  gvg_args_builder_add (args, NULL);
  
  return gvg_args_builder_free (args, FALSE);
}

static gboolean
make_pipe (gint     p[2],
           GError **error)
{
  if (pipe (p) < 0) {
    gint errsv = errno;
    
    g_set_error (error, G_IO_ERROR, g_io_error_from_errno (errsv),
                 "failed to create pipe for communication with child process (%s)",
                 g_strerror (errsv));
    
    return FALSE;
  }
  
  return TRUE;
}

gboolean
gvg (const gchar          **program_argv,
     GvgGetArgsFunc         get_args,
     gpointer               get_args_data,
     GvgXmlStartElementFunc xml_start_element_func,
     GvgXmlEndElementFunc   xml_end_element_func,
     gpointer               xml_data,
     GDestroyNotify         xml_data_destroy,
     GError               **error)
{
  gboolean  success     = FALSE;
  gint      xml_pipe[2] = { -1, -1 };
  
  if (make_pipe (xml_pipe, error)) {
    GPid    pid;
    gchar **argv;
    
    argv = build_argv (program_argv, xml_pipe[1], get_args, get_args_data);
    if (! g_spawn_async_with_pipes (NULL, argv, NULL,
                                    G_SPAWN_CHILD_INHERITS_STDIN |
                                    G_SPAWN_DO_NOT_REAP_CHILD |
                                    G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
                                    G_SPAWN_SEARCH_PATH,
                                    NULL, NULL, &pid, NULL, NULL, NULL,
                                    error)) {
      close_and_invalidate (&xml_pipe[0]);
    } else {
      GvgChildData *cdata;
      
      cdata = gvg_child_data_new ();
      cdata->pid = pid;
      cdata->parser = gvg_xml_parser_new (xml_start_element_func,
                                          xml_end_element_func,
                                          xml_data, xml_data_destroy);
      
      g_child_watch_add_full (G_PRIORITY_DEFAULT, cdata->pid, watch_child,
                              gvg_child_data_ref (cdata),
                              (GDestroyNotify) gvg_child_data_unref);
      /* pipe channel watch */
      cdata->xml_pipe = xml_pipe[0];
      cdata->pipe_channel = create_io_channel (cdata->xml_pipe);
      cdata->pipe_source = g_io_create_watch (cdata->pipe_channel,
                                              G_IO_IN | G_IO_PRI |
                                              G_IO_ERR | G_IO_HUP);
      g_source_set_callback (cdata->pipe_source, (GSourceFunc) xml_fd_in_ready,
                             gvg_child_data_ref (cdata),
                             (GDestroyNotify) gvg_child_data_unref);
      g_source_attach (cdata->pipe_source, NULL);
      
      gvg_child_data_unref (cdata);
      success = TRUE;
    }
    close_and_invalidate (&xml_pipe[1]);
    g_strfreev (argv);
  }
  
  return success;
}
