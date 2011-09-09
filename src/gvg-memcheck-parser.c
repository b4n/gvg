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

#include "gvg-memcheck-parser.h"

#include <glib.h>
#include <glib-object.h>
#include <string.h>

#include "gvg-xml-parser.h"
#include "gvg-memcheck-store.h"


#define set_ptr(ptr, val) \
  G_STMT_START {          \
    gpointer tmp = ptr;   \
    ptr = val;            \
    g_free (tmp);         \
  } G_STMT_END

#define STREQ(t, n) (strcmp ((t), (n)) == 0)


typedef struct _GvgMemcheckFrame GvgMemcheckFrame;

struct _GvgMemcheckFrame
{
  guint64 ip;
  gchar  *obj;
  gchar  *func;
  gchar  *dir;
  gchar  *file;
  guint   line;
};

struct _GvgMemcheckParserPrivate
{
  GtkTreeIter   parent_iter;
  GtkTreeIter   root_parent_iter;
  GtkTreeStore *store;
  
  guint             stack_len;
  GvgMemcheckFrame  frame;
};


G_DEFINE_TYPE (GvgMemcheckParser,
               gvg_memcheck_parser,
               GVG_TYPE_XML_PARSER)


static void     gvg_memcheck_parser_finalize        (GObject *object);
static void     gvg_memcheck_parser_get_property    (GObject    *object,
                                                     guint       prop_id,
                                                     GValue     *value,
                                                     GParamSpec *pspec);
static void     gvg_memcheck_parser_set_property    (GObject      *object,
                                                     guint         prop_id,
                                                     const GValue *value,
                                                     GParamSpec   *pspec);
static void     gvg_memcheck_parser_element_start   (GvgXmlParser *parser,
                                                     const gchar  *name,
                                                     const gchar **atts,
                                                     const gchar  *path);
static void     gvg_memcheck_parser_element_end     (GvgXmlParser  *parser,
                                                     const gchar   *name,
                                                     const gchar   *content,
                                                     const gchar   *path);


enum
{
  PROP_0,
  PROP_STORE
};


static void
gvg_memcheck_parser_class_init (GvgMemcheckParserClass *klass)
{
  GObjectClass       *object_class      = G_OBJECT_CLASS (klass);
  GvgXmlParserClass  *xml_parser_class  = GVG_XML_PARSER_CLASS (klass);
  
  object_class->finalize          = gvg_memcheck_parser_finalize;
  object_class->set_property      = gvg_memcheck_parser_set_property;
  object_class->get_property      = gvg_memcheck_parser_get_property;
  
  xml_parser_class->element_start = gvg_memcheck_parser_element_start;
  xml_parser_class->element_end   = gvg_memcheck_parser_element_end;
  
  g_object_class_install_property (object_class,
                                   PROP_STORE,
                                   g_param_spec_object ("store",
                                                        "Store",
                                                        "The GvgMemcheckStore to fill",
                                                        GVG_TYPE_MEMCHECK_STORE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_CONSTRUCT_ONLY));
  
  g_type_class_add_private (klass, sizeof (GvgMemcheckParserPrivate));
}

static void
gvg_memcheck_parser_init (GvgMemcheckParser *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_MEMCHECK_PARSER,
                                            GvgMemcheckParserPrivate);
  
  self->priv->store       = NULL;
  self->priv->stack_len   = 0u;
  self->priv->frame.dir   = NULL;
  self->priv->frame.file  = NULL;
  self->priv->frame.func  = NULL;
  self->priv->frame.ip    = 0x0u;
  self->priv->frame.line  = 0u;
  self->priv->frame.obj   = NULL;
}

static void
gvg_memcheck_parser_finalize (GObject *object)
{
  GvgMemcheckParser *self = GVG_MEMCHECK_PARSER (object);
  
  g_object_unref (self->priv->store);
  set_ptr (self->priv->frame.dir, NULL);
  set_ptr (self->priv->frame.file, NULL);
  set_ptr (self->priv->frame.func, NULL);
  set_ptr (self->priv->frame.obj, NULL);
  
  G_OBJECT_CLASS (gvg_memcheck_parser_parent_class)->finalize (object);
}

static void
gvg_memcheck_parser_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  GvgMemcheckParser *self = GVG_MEMCHECK_PARSER (object);
  
  switch (prop_id) {
    case PROP_STORE:
      g_value_set_object (value, self->priv->store);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gvg_memcheck_parser_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  GvgMemcheckParser *self = GVG_MEMCHECK_PARSER (object);
  
  switch (prop_id) {
    case PROP_STORE:
      if (self->priv->store) {
        g_object_unref (self->priv->store);
      }
      self->priv->store = g_value_dup_object (value);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gvg_memcheck_parser_element_start (GvgXmlParser *parser,
                                   const gchar  *name,
                                   const gchar **atts,
                                   const gchar  *path)
{
  GvgMemcheckParser *self = (GvgMemcheckParser *) parser;
  
  //~ g_debug ("element start");
  
  if        (STREQ (path, "/valgrindoutput/error")) {
    gtk_tree_store_append (self->priv->store, &self->priv->parent_iter, NULL);
    self->priv->root_parent_iter = self->priv->parent_iter;
  } else if (STREQ (path, "/valgrindoutput/error/stack")) {
    self->priv->stack_len = 0;
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame")) {
    set_ptr (self->priv->frame.obj,   NULL);
    set_ptr (self->priv->frame.func,  NULL);
    set_ptr (self->priv->frame.dir,   NULL);
    set_ptr (self->priv->frame.file,  NULL);
    self->priv->frame.ip    = 0u;
    self->priv->frame.line  = 0u;
    self->priv->stack_len ++;
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

static guint64
str_to_uint64 (const gchar *str)
{
  gchar  *endptr;
  guint64 result;
  
  result = g_ascii_strtoull (str, &endptr, 0);
  if (*endptr != 0) {
    g_warning ("invalid data at end of numeric value: %s", str);
  }
  
  return result;
}

static guint
str_to_uint (const gchar *str)
{
  guint64 result;
  
  result = str_to_uint64 (str);
  if (result > G_MAXUINT) {
    g_warning ("value %"G_GUINT64_FORMAT" too big for uint, truncating",
               result);
    result = G_MAXUINT;
  }
  
  return (guint) result;
}

static void
gvg_memcheck_parser_element_end (GvgXmlParser  *parser,
                                 const gchar   *name,
                                 const gchar   *content,
                                 const gchar   *path)
{
  GvgMemcheckParser *self = (GvgMemcheckParser *) parser;
  
  //~ g_debug ("element end");
  
  if        (STREQ (path, "/valgrindoutput")) {
    gtk_tree_store_append (self->priv->store, &self->priv->parent_iter, NULL);
    gtk_tree_store_set (self->priv->store, &self->priv->parent_iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, "== END ==", -1);
  } else if (STREQ (path, "/valgrindoutput/tool")) {
    g_assert (STREQ (content, "memcheck"));
  } else if (STREQ (path, "/valgrindoutput/status")) {
    gtk_tree_store_append (self->priv->store, &self->priv->parent_iter, NULL);
    gtk_tree_store_set (self->priv->store, &self->priv->parent_iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, "STATUS", -1);
  } else if (STREQ (path, "/valgrindoutput/errorcounts")) {
    gtk_tree_store_append (self->priv->store, &self->priv->parent_iter, NULL);
    gtk_tree_store_set (self->priv->store, &self->priv->parent_iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, "ERRORCOUNTS", -1);
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame")) {
    GtkTreeIter iter;
    gchar      *text;
    
    text = get_frame_display (&self->priv->frame, self->priv->stack_len);
    gtk_tree_store_append (self->priv->store, &iter, &self->priv->parent_iter);
    gtk_tree_store_set (self->priv->store, &iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, text,
                        GVG_MEMCHECK_STORE_COLUMN_IP, self->priv->frame.ip,
                        GVG_MEMCHECK_STORE_COLUMN_DIR, self->priv->frame.dir,
                        GVG_MEMCHECK_STORE_COLUMN_FILE, self->priv->frame.file,
                        GVG_MEMCHECK_STORE_COLUMN_LINE, self->priv->frame.line,
                        -1);
    g_free (text);
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame/ip")) {
    self->priv->frame.ip = str_to_uint64 (content);
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame/obj")) {
    set_ptr (self->priv->frame.obj, g_strdup (content));
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame/fn")) {
    set_ptr (self->priv->frame.func, g_strdup (content));
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame/dir")) {
    set_ptr (self->priv->frame.dir, g_strdup (content));
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame/file")) {
    set_ptr (self->priv->frame.file, g_strdup (content));
  } else if (STREQ (path, "/valgrindoutput/error/stack/frame/line")) {
    self->priv->frame.line = str_to_uint (content);
  } else if (STREQ (path, "/valgrindoutput/error/xwhat/text") ||
             STREQ (path, "/valgrindoutput/error/what")) {
    gtk_tree_store_set (self->priv->store, &self->priv->root_parent_iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, content, -1);
  } else if (STREQ (path, "/valgrindoutput/error/kind")) {
    /* TODO: parse kind */
  } else if (STREQ (path, "/valgrindoutput/error/auxwhat")) {
    gtk_tree_store_append (self->priv->store, &self->priv->parent_iter,
                           &self->priv->parent_iter);
    gtk_tree_store_set (self->priv->store, &self->priv->parent_iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, content, -1);
  }
}

GvgXmlParser *
gvg_memcheck_parser_new (GvgMemcheckStore *store)
{
  return g_object_new (GVG_TYPE_MEMCHECK_PARSER, "store", store, NULL);
}
