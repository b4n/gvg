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
#include <glib-object.h>
#include <gio/gio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "gvg-xml-parser.h"
#include "gvg-args-builder.h"
#include "gvg-options.h"


#ifdef G_OS_WIN32
# define INVALID_PID NULL
#else
# define INVALID_PID -1
#endif


struct _GvgPrivate
{
  GPid          pid;
  gint          xml_pipe;
  GSource      *pipe_source;
  GIOChannel   *pipe_channel;
  
  GvgXmlParser *parser;
  GvgOptions   *options;
};


G_DEFINE_ABSTRACT_TYPE (Gvg,
                        gvg,
                        G_TYPE_OBJECT)


static void     gvg_finalize        (GObject *object);
static void     gvg_get_property    (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec);
static void     gvg_set_property    (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec);

static void     cleanup_child       (Gvg     *self,
                                     gboolean kill_it);
static void     cleanup_pipe        (Gvg *self);



enum
{
  PROP_0,
  PROP_PARSER,
  PROP_OPTIONS
};


static void
gvg_class_init (GvgClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  
  object_class->finalize      = gvg_finalize;
  object_class->set_property  = gvg_set_property;
  object_class->get_property  = gvg_get_property;
  
  g_object_class_install_property (object_class,
                                   PROP_PARSER,
                                   g_param_spec_object ("parser",
                                                        "Parser",
                                                        "The XML parser for Valgrind's output",
                                                        GVG_TYPE_XML_PARSER,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class,
                                   PROP_OPTIONS,
                                   g_param_spec_object ("options",
                                                        "Options",
                                                        "The Valgrind options",
                                                        GVG_TYPE_OPTIONS,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_CONSTRUCT_ONLY));
    
  g_type_class_add_private (klass, sizeof (GvgPrivate));
}

static void
gvg_init (Gvg *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_GVG, GvgPrivate);
  
  self->priv->pid           = INVALID_PID;
  self->priv->xml_pipe      = -1;
  self->priv->pipe_source   = NULL;
  self->priv->pipe_channel  = NULL;
  self->priv->parser        = NULL;
}

static void
gvg_finalize (GObject *object)
{
  Gvg *self = GVG (object);
  
  cleanup_child (self, TRUE);
  cleanup_pipe (self);
  if (self->priv->parser) {
    /* ensure the parser terminated */
    gvg_xml_parser_push (self->priv->parser, NULL, 0, TRUE);
    g_object_unref (self->priv->parser);
    self->priv->parser = NULL;
  }
  
  G_OBJECT_CLASS (gvg_parent_class)->finalize (object);
}

static void
gvg_get_property (GObject    *object,
                  guint       prop_id,
                  GValue     *value,
                  GParamSpec *pspec)
{
  Gvg *self = GVG (object);
  
  switch (prop_id) {
    case PROP_PARSER:
      g_value_set_object (value, self->priv->parser);
      break;
    
    case PROP_OPTIONS:
      g_value_set_object (value, self->priv->options);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_set_property (GObject      *object,
                  guint         prop_id,
                  const GValue *value,
                  GParamSpec   *pspec)
{
  Gvg *self = GVG (object);
  
  switch (prop_id) {
    case PROP_PARSER:
      if (self->priv->parser) {
        g_object_unref (self->priv->parser);
      }
      self->priv->parser = g_value_dup_object (value);
      break;
    
    case PROP_OPTIONS:
      if (self->priv->options) {
        g_object_unref (self->priv->options);
      }
      self->priv->options = g_value_dup_object (value);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
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
  /* we don't want to either buffer or do any conversion */
  g_io_channel_set_encoding (channel, NULL, NULL);
  g_io_channel_set_buffered (channel, FALSE);
  
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
read_xml_pipe (Gvg     *self,
               gboolean check)
{
  gchar buf[BUFSIZ];
  gsize len;
  
  do {
    GIOStatus status;
    
    /* we're asked to check whether the channel has available data */
    if (check) {
      gint ret;
      
      ret = io_channel_poll (self->priv->pipe_channel, G_IO_IN, 0);
      if (ret < 0) {
        return FALSE;
      } else if (ret == 0) {
        return TRUE;
      }
    }
    
    do {
      len = 0u;
      status = g_io_channel_read_chars (self->priv->pipe_channel,
                                        buf, sizeof buf, &len, NULL);
    } while (status == G_IO_STATUS_AGAIN);
    
    if (status == G_IO_STATUS_NORMAL) {
      gvg_xml_parser_push (self->priv->parser, buf, len, FALSE);
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
cleanup_child (Gvg     *self,
               gboolean kill_it)
{
  if (self->priv->pid != INVALID_PID) {
    if (kill_it) {
      terminate_child (self->priv->pid);
    }
    g_spawn_close_pid (self->priv->pid);
    self->priv->pid = INVALID_PID;
  }
}

static void
cleanup_pipe (Gvg *self)
{
  /* make sure we read everything up to now */
  if (self->priv->pipe_source) {
    g_source_destroy (self->priv->pipe_source);
    self->priv->pipe_source = NULL;
  }
  if (self->priv->pipe_channel) {
    read_xml_pipe (self, TRUE);
    g_io_channel_shutdown (self->priv->pipe_channel, TRUE, NULL);
    g_io_channel_unref (self->priv->pipe_channel);
    self->priv->pipe_channel = NULL;
  }
  close_and_invalidate (&self->priv->xml_pipe);
}

static gboolean
xml_fd_in_ready (GIOChannel  *channel,
                 GIOCondition cond,
                 gpointer     data)
{
  Gvg      *self = data;
  gboolean  keep  = TRUE;
  
  g_assert (self->priv->pipe_channel == channel);
  
  /*if (cond & G_IO_IN) g_debug ("in");
  if (cond & G_IO_OUT) g_debug ("out");
  if (cond & G_IO_PRI) g_debug ("pri");
  if (cond & G_IO_ERR) g_debug ("err");
  if (cond & G_IO_HUP) g_debug ("hup");
  if (cond & G_IO_NVAL) g_debug ("nval");*/
  
  if (cond & (G_IO_IN | G_IO_PRI)) {
    keep = read_xml_pipe (self, FALSE);
  }
  if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
    keep = FALSE;
  }
  
  if (! keep) {
    g_debug ("removing pipe watch");
    cleanup_pipe (self);
  }
  return keep;
}

static void
watch_child (GPid     pid,
             gint     status,
             gpointer data)
{
  Gvg *self = data;
  
  g_debug ("child terminated");
  cleanup_child (self, FALSE);
}

static gchar **
build_argv (Gvg          *self,
            const gchar **program_argv,
            gint          xml_fd)
{
  GvgArgsBuilder *args  = gvg_args_builder_new ();
  
  gvg_args_builder_add (args, "valgrind");
  /* FIXME: this options isn't supported by Valgrind < 3.6.1 */
  /*gvg_args_builder_add_string (args, "fullpath-after", "");*/
  gvg_args_builder_add_bool (args, "xml", TRUE);
  gvg_args_builder_add_arg (args, "xml-fd", "%d", xml_fd);
  gvg_args_builder_add (args, "-q");
  /* this avoids having wrong XML because of forked children (see Valgrind
   * manual man) */
  gvg_args_builder_add_string (args, "child-silent-after-fork", "yes");
  
  if (self->priv->options) {
    gvg_options_to_args (self->priv->options, args);
  }
  
  /* add program and NULL terminator */
  gvg_args_builder_add_args (args, program_argv);
  gvg_args_builder_add (args, NULL);
  
  /*{
    guint i;
    
    for (i = 0; i < args->len; i++) {
      g_debug ("args[%u] = %s", i, g_ptr_array_index (args, i));
    }
  }*/
  
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
gvg_run (Gvg           *self,
         const gchar  **program_argv,
         GError       **error)
{
  gboolean  success     = FALSE;
  gint      xml_pipe[2] = { -1, -1 };
  
  g_return_val_if_fail (GVG_IS_GVG (self), FALSE);
  g_return_val_if_fail (self->priv->parser != NULL, FALSE);
  g_return_val_if_fail (! gvg_is_busy (self), FALSE);
  
  if (make_pipe (xml_pipe, error)) {
    GPid    pid;
    gchar **argv;
    
    argv = build_argv (self, program_argv, xml_pipe[1]);
    if (! g_spawn_async_with_pipes (NULL, argv, NULL,
                                    G_SPAWN_CHILD_INHERITS_STDIN |
                                    G_SPAWN_DO_NOT_REAP_CHILD |
                                    G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
                                    G_SPAWN_SEARCH_PATH,
                                    NULL, NULL, &pid, NULL, NULL, NULL,
                                    error)) {
      close_and_invalidate (&xml_pipe[0]);
    } else {
      self->priv->pid = pid;
      g_child_watch_add_full (G_PRIORITY_DEFAULT, self->priv->pid, watch_child,
                              self, NULL);
      /* pipe channel watch */
      self->priv->xml_pipe = xml_pipe[0];
      self->priv->pipe_channel = create_io_channel (self->priv->xml_pipe);
      self->priv->pipe_source = g_io_create_watch (self->priv->pipe_channel,
                                                   G_IO_IN | G_IO_PRI |
                                                   G_IO_ERR | G_IO_HUP);
      g_source_set_callback (self->priv->pipe_source,
                             (GSourceFunc) xml_fd_in_ready, self, NULL);
      g_source_attach (self->priv->pipe_source, NULL);
      
      success = TRUE;
    }
    close_and_invalidate (&xml_pipe[1]);
    g_strfreev (argv);
  }
  
  return success;
}

gboolean
gvg_is_busy (Gvg *self)
{
  g_return_val_if_fail (GVG_IS_GVG (self), FALSE);
  
  return self->priv->pid != INVALID_PID || self->priv->xml_pipe >= 0;
}
