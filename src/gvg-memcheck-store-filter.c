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

#include "gvg-memcheck-store-filter.h"

#include <gtk/gtk.h>
#include <string.h>

#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-store.h"
#include "gvg-enum-types.h"


struct _GvgMemcheckStoreFilterPrivate
{
  GvgMemcheckErrorKind  kind;
  gchar                *text;
  gboolean              invert;
  
  GSource              *timeout_source;
};


G_DEFINE_TYPE (GvgMemcheckStoreFilter,
               gvg_memcheck_store_filter,
               GTK_TYPE_TREE_MODEL_FILTER)


static void     gvg_memcheck_store_filter_get_property  (GObject    *object,
                                                         guint       prop_id,
                                                         GValue     *value,
                                                         GParamSpec *pspec);
static void     gvg_memcheck_store_filter_set_property  (GObject       *object,
                                                         guint          prop_id,
                                                         const GValue  *value,
                                                         GParamSpec    *pspec);
static void     gvg_memcheck_store_filter_constructed   (GObject *object);
static void     gvg_memcheck_store_filter_finalize      (GObject *object);

enum
{
  PROP_0,
  PROP_KIND,
  PROP_TEXT,
  PROP_INVERT
};



static void
gvg_memcheck_store_filter_class_init (GvgMemcheckStoreFilterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->get_property  = gvg_memcheck_store_filter_get_property;
  object_class->set_property  = gvg_memcheck_store_filter_set_property;
  object_class->constructed   = gvg_memcheck_store_filter_constructed;
  object_class->finalize      = gvg_memcheck_store_filter_finalize;
  
  g_object_class_install_property (object_class,
                                   PROP_KIND,
                                   g_param_spec_enum ("kind",
                                                      "Kind",
                                                      "The error kind to show",
                                                      GVG_TYPE_MEMCHECK_ERROR_KIND,
                                                      GVG_MEMCHECK_ERROR_KIND_ANY,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "Text",
                                                        "A text the error should contain",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_INVERT,
                                   g_param_spec_boolean ("invert",
                                                         "Invert",
                                                         "Whether to invert the match",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private ((gpointer) klass,
                            sizeof (GvgMemcheckStoreFilterPrivate));
}

static void
gvg_memcheck_store_filter_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  GvgMemcheckStoreFilter *self = GVG_MEMCHECK_STORE_FILTER (object);
  
  switch (prop_id) {
    case PROP_KIND:
      g_value_set_enum (value, self->priv->kind);
      break;
    
    case PROP_TEXT:
      g_value_set_string (value, self->priv->text);
      break;
    
    case PROP_INVERT:
      g_value_set_boolean (value, self->priv->invert);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_memcheck_store_filter_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  GvgMemcheckStoreFilter *self = GVG_MEMCHECK_STORE_FILTER (object);
  
  switch (prop_id) {
    case PROP_KIND:
      gvg_memcheck_store_filter_set_kind (self, g_value_get_enum (value));
      break;
    
    case PROP_TEXT:
      gvg_memcheck_store_filter_set_text (self, g_value_get_string (value));
      break;
    
    case PROP_INVERT:
      gvg_memcheck_store_filter_set_invert (self, g_value_get_boolean (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static gboolean
gvg_memcheck_store_filter_filter_kind (GvgMemcheckStoreFilter *self,
                                       GtkTreeModel           *model,
                                       GtkTreeIter            *iter)
{
  GvgMemcheckErrorKind  kind;
  GtkTreeIter           parent;
  
  /* always make a decision using a toplevel */
  if (gtk_tree_model_iter_parent (model, &parent, iter)) {
    return gvg_memcheck_store_filter_filter_kind (self, model, &parent);
  }
  
  gtk_tree_model_get (model, iter, GVG_MEMCHECK_STORE_COLUMN_KIND, &kind, -1);
  if (kind == GVG_MEMCHECK_ERROR_KIND_ANY) {
    return TRUE;
  }
  
  return (self->priv->kind == GVG_MEMCHECK_ERROR_KIND_ANY ||
          self->priv->kind == kind);
}

static gboolean
filter_text_matches (const gchar *data,
                     const gchar *filter)
{
  if (! data) {
    return ! filter || ! *filter;
  } else if (! filter) {
    return ! data || ! *data;
  }
  /* FIXME: be case insensitive? */
  return strstr (data, filter) != NULL;
}

static gboolean
filter_text_iter_matches (GvgMemcheckStoreFilter *self,
                          GtkTreeModel           *model,
                          GtkTreeIter            *iter)
{
  GtkTreeIter child;
  gboolean    match = TRUE;
  
  if (self->priv->text && *self->priv->text) {
    gchar  *label;
    gchar  *dir;
    gchar  *file;
    
    gtk_tree_model_get (model, iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, &label,
                        GVG_MEMCHECK_STORE_COLUMN_DIR, &dir,
                        GVG_MEMCHECK_STORE_COLUMN_FILE, &file,
                        -1);
    match = (filter_text_matches (label, self->priv->text) ||
             filter_text_matches (dir, self->priv->text) ||
             filter_text_matches (file, self->priv->text));
    g_free (label);
    g_free (dir);
    g_free (file);
  }
  
  if (! match) {
    if (gtk_tree_model_iter_children (model, &child, iter)) {
      match = filter_text_iter_matches (self, model, &child);
      while (! match && gtk_tree_model_iter_next (model, &child)) {
        match = filter_text_iter_matches (self, model, &child);
      }
    }
  }
  
  return match;
}

/* FIXME: optimize this filter, it's quite slow */
static gboolean
gvg_memcheck_store_filter_filter_text (GvgMemcheckStoreFilter  *self,
                                       GtkTreeModel            *model,
                                       GtkTreeIter             *iter_)
{
  gboolean    match = FALSE;
  GtkTreeIter parent;
  GtkTreeIter iter = *iter_;
  
  while (gtk_tree_model_iter_parent (model, &parent, &iter)) {
    iter = parent;
  }
  
  /* never filter out toplevels without children, they are no entries */
  if (! gtk_tree_model_iter_has_child (model, &iter)) {
    return TRUE;
  }
  
  match = filter_text_iter_matches (self, model, &iter);
  if (self->priv->invert) {
    match = ! match;
  }
  
  return match;
}

static gboolean
gvg_memcheck_store_filter_filter_func (GtkTreeModel *model,
                                       GtkTreeIter  *iter,
                                       gpointer      data)
{
  return (gvg_memcheck_store_filter_filter_kind (data, model, iter) &&
          gvg_memcheck_store_filter_filter_text (data, model, iter));
}

static gboolean
filter_timeout_func (gpointer data)
{
  GvgMemcheckStoreFilter *self = data;
  
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (self));
  self->priv->timeout_source = NULL;
  
  return FALSE;
}

/* @now: whether to allow a timeout or not */
static void
gvg_memcheck_store_filter_refilter (GvgMemcheckStoreFilter *self,
                                    gboolean                now)
{
  if (self->priv->timeout_source) {
    g_source_destroy (self->priv->timeout_source);
  }
  if (now) {
    self->priv->timeout_source = NULL;
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (self));
  } else {
    self->priv->timeout_source = g_timeout_source_new (250);
    g_source_set_callback (self->priv->timeout_source, filter_timeout_func,
                           self, NULL);
    
    g_source_attach (self->priv->timeout_source, NULL);
    g_source_unref (self->priv->timeout_source);
  }
}

static void
gvg_memcheck_store_filter_init (GvgMemcheckStoreFilter *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GVG_TYPE_MEMCHECK_STORE_FILTER,
                                            GvgMemcheckStoreFilterPrivate);
  
  self->priv->kind            = GVG_MEMCHECK_ERROR_KIND_ANY;
  self->priv->text            = NULL;
  self->priv->invert          = FALSE;
  self->priv->timeout_source  = NULL;
  
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (self),
                                          gvg_memcheck_store_filter_filter_func,
                                          self, NULL);
}

/* emits a fake change on the parent of an inserted or change row for the
 * whole entry to be refiltered, since we generally need the whole entry to
 * make our choice */
static void
force_refresh_entry (GtkTreeModel *model,
                     GtkTreePath  *path_,
                     GtkTreeIter  *iter_,
                     gpointer      data)
{
  GtkTreeIter parent;
  GtkTreeIter iter = *iter_;
  gboolean    refresh = FALSE; /* only refresh children */
  
  while (gtk_tree_model_iter_parent (model, &parent, &iter)) {
    iter = parent;
    refresh = TRUE;
  }
  if (refresh) {
    GtkTreePath  *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    /* actually there's no real need to protect ourselves to be called by the
     * fake change since we'll quit immediately since it's emitted on a root
     * element.  maybe event the cost of blocking and unblocking is higher than
     * the recursion */
    /*g_signal_handlers_block_by_func (model, force_refresh_entry, data);*/
    gtk_tree_model_row_changed (model, path, &iter);
    /*g_signal_handlers_unblock_by_func (model, force_refresh_entry, data);*/
  }
}

static void
gvg_memcheck_store_filter_constructed (GObject *self)
{
  GtkTreeModel *model;
  
  if (G_OBJECT_CLASS (gvg_memcheck_store_filter_parent_class)->constructed) {
    G_OBJECT_CLASS (gvg_memcheck_store_filter_parent_class)->constructed (self);
  }
  
  model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (self));
  
  /* WTF? when I connect the signal below (row-inserted), I get the first child
   * duplicated as the last child... really, WTF? */
  /*g_signal_connect (model, "row-inserted",
                    G_CALLBACK (force_refresh_entry), self);*/
  g_signal_connect (model, "row-changed",
                    G_CALLBACK (force_refresh_entry), self);
}

static void
gvg_memcheck_store_filter_finalize (GObject *object)
{
  GvgMemcheckStoreFilter *self = GVG_MEMCHECK_STORE_FILTER (object);
  
  if (self->priv->timeout_source) {
    g_source_destroy (self->priv->timeout_source);
    self->priv->timeout_source = NULL;
  }
  g_free (self->priv->text);
  
  G_OBJECT_CLASS (gvg_memcheck_store_filter_parent_class)->finalize (object);
}


GtkTreeModel *
gvg_memcheck_store_filter_new (GvgMemcheckStore  *model,
                               GtkTreePath       *root)
{
  g_return_val_if_fail (model == NULL || GVG_IS_MEMCHECK_STORE (model), NULL);
  
  if (! model) {
    model = gvg_memcheck_store_new ();
  }
  
  return g_object_new (GVG_TYPE_MEMCHECK_STORE_FILTER,
                       "child-model", model,
                       "virtual-root", root,
                       NULL);
}

GvgMemcheckErrorKind
gvg_memcheck_store_filter_get_kind (GvgMemcheckStoreFilter *self)
{
  g_return_val_if_fail (GVG_IS_MEMCHECK_STORE_FILTER (self),
                        GVG_MEMCHECK_ERROR_KIND_ANY);
  
  return self->priv->kind;
}

void
gvg_memcheck_store_filter_set_kind (GvgMemcheckStoreFilter *self,
                                    GvgMemcheckErrorKind    kind)
{
  g_return_if_fail (GVG_IS_MEMCHECK_STORE_FILTER (self));
  
  self->priv->kind = kind;
  gvg_memcheck_store_filter_refilter (self, TRUE);
  g_object_notify (G_OBJECT (self), "kind");
}

const gchar *
gvg_memcheck_store_filter_get_text (GvgMemcheckStoreFilter *self)
{
  g_return_val_if_fail (GVG_IS_MEMCHECK_STORE_FILTER (self), NULL);
  
  return self->priv->text;
}

void
gvg_memcheck_store_filter_set_text (GvgMemcheckStoreFilter *self,
                                    const gchar            *text)
{
  g_return_if_fail (GVG_IS_MEMCHECK_STORE_FILTER (self));
  
  g_free (self->priv->text);
  self->priv->text = g_strdup (text);
  gvg_memcheck_store_filter_refilter (self, FALSE);
  g_object_notify (G_OBJECT (self), "text");
}

gboolean
gvg_memcheck_store_filter_get_invert (GvgMemcheckStoreFilter *self)
{
  g_return_val_if_fail (GVG_IS_MEMCHECK_STORE_FILTER (self), FALSE);
  
  return self->priv->invert;
}

void
gvg_memcheck_store_filter_set_invert (GvgMemcheckStoreFilter *self,
                                      gboolean                invert)
{
  g_return_if_fail (GVG_IS_MEMCHECK_STORE_FILTER (self));
  
  self->priv->invert = invert;
  gvg_memcheck_store_filter_refilter (self, TRUE);
  g_object_notify (G_OBJECT (self), "invert");
}
