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

#include "gvg-memcheck.h"

#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "gvg.h"
#include "gvg-args-builder.h"


const gchar *
gvg_memcheck_leak_check_mode_to_string (GvgMemcheckLeakCheckMode mode)
{
  switch (mode) {
    case GVG_MEMCHECK_LEAK_CHECK_NO:      return "no";
    case GVG_MEMCHECK_LEAK_CHECK_SUMMARY: return "summary";
    case GVG_MEMCHECK_LEAK_CHECK_YES:     return "yes";
    case GVG_MEMCHECK_LEAK_CHECK_FULL:    return "full";
  }
  g_return_val_if_reached (NULL);
}

const gchar *
gvg_memcheck_leak_resolution_mode_to_string (GvgMemcheckLeakResolutionMode mode)
{
  switch (mode) {
    case GVG_MEMCHECK_LEAK_RESOLUTION_LOW:    return "low";
    case GVG_MEMCHECK_LEAK_RESOLUTION_MEDIUM: return "med";
    case GVG_MEMCHECK_LEAK_RESOLUTION_HIGH:   return "high";
  }
  g_return_val_if_reached (NULL);
}

void
gvg_memcheck_add_options (GvgArgsBuilder           *args,
                          const GvgMemcheckOptions *opts)
{
  gvg_args_builder_add_string (args, "tool", "memcheck");
  gvg_args_builder_add_string (args, "leak-check",
                               gvg_memcheck_leak_check_mode_to_string (opts->leak_check));
  gvg_args_builder_add_string (args, "leak-resolution",
                               gvg_memcheck_leak_resolution_mode_to_string (opts->leak_resolution));
  gvg_args_builder_add_ssize_checked (args, "freelist-vol", opts->freelist_vol);
  gvg_args_builder_add_byte_checked (args, "malloc-fill", opts->malloc_fill);
  gvg_args_builder_add_byte_checked (args, "free-fill", opts->free_fill);
  /* this option is not supported by Valgrind < 3.6.1 */
  /*gvg_args_builder_add_bool (args, "show-possibly-lost",
                             opts->show_possibly_lost);*/
  gvg_args_builder_add_bool (args, "show-reachable", opts->show_reachable);
  gvg_args_builder_add_bool (args, "undef-value-errors",
                             opts->undef_value_errors);
  gvg_args_builder_add_bool (args, "partial-loads-ok", opts->partial_loads_ok);
  gvg_args_builder_add_bool (args, "workaround-gcc296-bugs",
                             opts->workaround_gcc296_bugs);
}

static void
get_memcheck_options (GvgArgsBuilder *bld,
                      gpointer        data)
{
  const GvgMemcheckOptions *opts = data;
  
  gvg_add_options (bld, &opts->parent);
  gvg_memcheck_add_options (bld, opts);
}



typedef enum {
  XML_PARENT_NONE   = 0,
  XML_PARENT_ERROR  = 1 << 0,
  XML_PARENT_STACK  = 1 << 1,
  XML_PARENT_XWHAT  = 1 << 2,
  XML_PARENT_FRAME  = 1 << 3
} XmlParentType;


typedef struct _XmlState XmlState;
typedef struct _GvgMemcheckFrame GvgMemcheckFrame;

struct _GvgMemcheckFrame {
  gsize   ip;
  gchar  *obj;
  gchar  *func;
  gchar  *dir;
  gchar  *file;
  guint   line;
};

struct _XmlState {
  GtkTreeIter   parent_iter;
  GtkTreeIter   root_parent_iter;
  GtkTreeStore *store;
  
  XmlParentType parent;
  
  struct {
    GvgMemcheckErrorKind  kind;
    gchar                *what;
  } error;
  guint             stack_len;
  GvgMemcheckFrame  frame;
};

#define TAGEQ(t, n) (strcmp (t, n) == 0)

static void
xml_start_tag_func (const gchar  *name,
                    const gchar **atts,
                    const gchar  *path,
                    gpointer      data)
{
  XmlState *xs = data;
  
  //~ g_debug ("got element start %s", name);
  
  if        (xs->parent == XML_PARENT_NONE && TAGEQ (name, "error")) {
    xs->parent |= XML_PARENT_ERROR;
    
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    xs->root_parent_iter = xs->parent_iter;
    xs->error.kind  = 0u;
    xs->error.what  = NULL;
  } else if (xs->parent & XML_PARENT_ERROR && TAGEQ (name, "stack")) {
    xs->parent |= XML_PARENT_STACK;
    xs->stack_len = 0;
  } else if (xs->parent & XML_PARENT_ERROR && TAGEQ (name, "xwhat")) {
    xs->parent |= XML_PARENT_XWHAT;
  } else if (xs->parent & XML_PARENT_STACK && TAGEQ (name, "frame")) {
    xs->parent |= XML_PARENT_FRAME;
    xs->frame.ip    = 0u;
    xs->frame.obj   = NULL;
    xs->frame.func  = NULL;
    xs->frame.dir   = NULL;
    xs->frame.file  = NULL;
    xs->frame.line  = 0u;
    xs->stack_len ++;
  }
}

static void
xml_start_tag_func2 (const gchar  *name,
                     const gchar **atts,
                     const gchar  *path,
                     gpointer      data)
{
  XmlState *xs = data;
  
  //~ g_debug ("got element start %s", name);
  
  if        (TAGEQ (path, "/valgrindoutput/error")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    xs->root_parent_iter = xs->parent_iter;
    xs->error.kind  = 0u;
    xs->error.what  = NULL;
  } else if (TAGEQ (path, "/valgrindoutput/error/stack")) {
    xs->stack_len = 0;
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame")) {
    xs->frame.ip    = 0u;
    xs->frame.obj   = NULL;
    xs->frame.func  = NULL;
    xs->frame.dir   = NULL;
    xs->frame.file  = NULL;
    xs->frame.line  = 0u;
    xs->stack_len ++;
  }
}

static gchar *
get_frame_display (GvgMemcheckFrame  *frame,
                   guint              nth)
{
  GString *str = g_string_new (NULL);
  
  g_string_append (str, nth < 2 ? "at" : "by");
  /*g_string_append_printf (str, " %#x: ", frame->ip);*/
  g_string_append (str, " ");
  g_string_append (str, frame->func ? frame->func : "???");
  if (frame->file) {
    g_string_append_printf (str, " (%s:%u)", frame->file, frame->line);
  } else {
    g_string_append_printf (str, " (in %s)", frame->obj);
  }
  
  return g_string_free (str, FALSE);
}

static void
xml_end_tag_func (const gchar  *name,
                  const gchar  *content,
                  const gchar  *path,
                  gpointer      data)
{
  XmlState *xs = data;
  
  //~ g_debug ("got element end %s (%s)", name, content);
  
  if        (xs->parent & XML_PARENT_ERROR && TAGEQ (name, "error")) {
    xs->parent ^= XML_PARENT_ERROR;
    
    gtk_tree_store_set (xs->store, &xs->root_parent_iter,
                        0, xs->error.what, -1);
  } else if (xs->parent == XML_PARENT_NONE && TAGEQ (name, "valgrindoutput")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, "== END ==", -1);
  } else if (xs->parent == XML_PARENT_NONE && TAGEQ (name, "status")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, "STATUS", -1);
  } else if (xs->parent == XML_PARENT_NONE && TAGEQ (name, "errorcounts")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, "ERRORCOUNTS", -1);
  } else if (xs->parent & XML_PARENT_STACK && TAGEQ (name, "stack")) {
    xs->parent ^= XML_PARENT_STACK;
  } else if (xs->parent & XML_PARENT_XWHAT && TAGEQ (name, "xwhat")) {
    xs->parent ^= XML_PARENT_XWHAT;
  } else if (xs->parent & XML_PARENT_FRAME && TAGEQ (name, "frame")) {
    GtkTreeIter iter;
    gchar      *text;
    
    xs->parent ^= XML_PARENT_FRAME;
    
    /*g_debug ("[frame %x]", xs->frame.ip);
    g_debug ("  obj:  %s", xs->frame.obj);
    g_debug ("  func: %s", xs->frame.func);
    g_debug ("  at %s/%s:%u", xs->frame.dir, xs->frame.file, xs->frame.line);*/
    
    text = get_frame_display (&xs->frame, xs->stack_len);
    gtk_tree_store_append (xs->store, &iter, &xs->parent_iter);
    gtk_tree_store_set (xs->store, &iter,
                        0, text,
                        1, xs->frame.ip,
                        2, xs->frame.dir,
                        3, xs->frame.file,
                        4, xs->frame.line,
                        -1);
    g_free (text);
  }
  
  if (xs->parent & XML_PARENT_FRAME) {
    if        (TAGEQ (name, "ip")) {
      xs->frame.ip = g_ascii_strtoull (content, NULL, 0);
    } else if (TAGEQ (name, "obj")) {
      xs->frame.obj = g_strdup (content);
    } else if (TAGEQ (name, "fn")) {
      xs->frame.func = g_strdup (content);
    } else if (TAGEQ (name, "dir")) {
      xs->frame.dir = g_strdup (content);
    } else if (TAGEQ (name, "file")) {
      xs->frame.file = g_strdup (content);
    } else if (TAGEQ (name, "line")) {
      xs->frame.line = g_ascii_strtoull (content, NULL, 0);
    }
  } else if (xs->parent & XML_PARENT_XWHAT) {
    if (TAGEQ (name, "text")) {
      xs->error.what = g_strdup (content);
    }
  } else if (xs->parent & XML_PARENT_STACK) {
  } else if (xs->parent & XML_PARENT_ERROR) {
    if        (TAGEQ (name, "kind")) {
    } else if (TAGEQ (name, "what")) {
      xs->error.what = g_strdup (content);
    } else if (TAGEQ (name, "auxwhat")) {
      gtk_tree_store_append (xs->store, &xs->parent_iter, &xs->parent_iter);
      gtk_tree_store_set (xs->store, &xs->parent_iter, 0, content, -1);
    }
  }
}

static void
xml_end_tag_func2 (const gchar *name,
                   const gchar *content,
                   const gchar *path,
                   gpointer     data)
{
  XmlState *xs = data;
  
  //~ g_debug ("got element end %s (%s)", name, content);
  
  if        (TAGEQ (path, "/valgrindoutput")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, "== END ==", -1);
  } else if (TAGEQ (path, "/valgrindoutput/tool")) {
    g_assert (strcmp (content, "memcheck") == 0);
  } else if (TAGEQ (path, "/valgrindoutput/error")) {
    gtk_tree_store_set (xs->store, &xs->root_parent_iter,
                        0, xs->error.what, -1);
  } else if (TAGEQ (path, "/valgrindoutput/status")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, "STATUS", -1);
  } else if (TAGEQ (path, "/valgrindoutput/errorcounts")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, NULL);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, "ERRORCOUNTS", -1);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame")) {
    GtkTreeIter iter;
    gchar      *text;
    
    /*g_debug ("[frame %x]", xs->frame.ip);
    g_debug ("  obj:  %s", xs->frame.obj);
    g_debug ("  func: %s", xs->frame.func);
    g_debug ("  at %s/%s:%u", xs->frame.dir, xs->frame.file, xs->frame.line);*/
    
    text = get_frame_display (&xs->frame, xs->stack_len);
    gtk_tree_store_append (xs->store, &iter, &xs->parent_iter);
    gtk_tree_store_set (xs->store, &iter,
                        0, text,
                        1, xs->frame.ip,
                        2, xs->frame.dir,
                        3, xs->frame.file,
                        4, xs->frame.line,
                        -1);
    g_free (text);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame/ip")) {
    xs->frame.ip = g_ascii_strtoull (content, NULL, 0);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame/obj")) {
    xs->frame.obj = g_strdup (content);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame/fn")) {
    xs->frame.func = g_strdup (content);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame/dir")) {
    xs->frame.dir = g_strdup (content);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame/file")) {
    xs->frame.file = g_strdup (content);
  } else if (TAGEQ (path, "/valgrindoutput/error/stack/frame/line")) {
    xs->frame.line = g_ascii_strtoull (content, NULL, 0);
  } else if (TAGEQ (path, "/valgrindoutput/error/xwhat/text") ||
             TAGEQ (path, "/valgrindoutput/error/what")) {
    xs->error.what = g_strdup (content);
  } else if (TAGEQ (path, "/valgrindoutput/error/kind")) {
  } else if (TAGEQ (path, "/valgrindoutput/error/auxwhat")) {
    gtk_tree_store_append (xs->store, &xs->parent_iter, &xs->parent_iter);
    gtk_tree_store_set (xs->store, &xs->parent_iter, 0, content, -1);
  }
}

gboolean
gvg_memcheck (const gchar             **program_argv,
              const GvgMemcheckOptions *options,
              GtkTreeStore             *store,
              GError                  **error)
{
  XmlState *xs;
  
  xs = g_malloc0 (sizeof *xs);
  xs->parent  = XML_PARENT_NONE;
  xs->store   = g_object_ref (store);
  
  if (! gvg (program_argv,
             get_memcheck_options, (gpointer) options,
             xml_start_tag_func2, xml_end_tag_func2, xs, g_free,
             error)) {
    g_object_unref (xs->store);
    g_free (xs);
    return FALSE;
  }
  
  return TRUE;
}
