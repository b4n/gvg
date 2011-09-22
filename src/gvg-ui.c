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

#include "gvg-ui.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-store.h"
#include "gvg-enum-types.h"


enum
{
  KIND_COL_KIND,
  KIND_COL_LABEL,
  KIND_N_COLUMNS
};

struct _GvgUIPrivate
{
  GvgMemcheckStore *store;
  GtkTreeModel     *filter;
  GtkWidget        *view;
  
  GType             kind_gtype;
  GtkListStore     *kind_store;
  GtkWidget        *kind_combo;
  
  GtkWidget        *filter_entry;
  GtkWidget        *filter_invert;
  GSource          *filter_timeout_source;
};


G_DEFINE_TYPE (GvgUI,
               gvg_ui,
               GTK_TYPE_VBOX)


enum
{
  PROP_0,
  PROP_MODEL
};


/*static void     gvg_ui_finalize       (GObject *object);*/


static void
gvg_ui_finalize (GObject *object)
{
  GvgUI *self = GVG_UI (object);
  
  if (self->priv->filter_timeout_source) {
    g_source_destroy (self->priv->filter_timeout_source);
    self->priv->filter_timeout_source = NULL;
  }
  
  G_OBJECT_CLASS (gvg_ui_parent_class)->finalize (object);
}

static void
gvg_ui_get_property (GObject    *object,
                     guint       prop_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
  GvgUI *self = GVG_UI (object);
  
  switch (prop_id) {
    case PROP_MODEL:
      g_value_set_object (value, self->priv->store);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_ui_set_property (GObject      *object,
                     guint         prop_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  GvgUI *self = GVG_UI (object);
  
  switch (prop_id) {
    case PROP_MODEL:
      gvg_ui_set_model (self, g_value_get_object (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static void
gvg_ui_class_init (GvgUIClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize      = gvg_ui_finalize;
  object_class->get_property  = gvg_ui_get_property;
  object_class->set_property  = gvg_ui_set_property;
  
  g_object_class_install_property (object_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model",
                                                        "Model",
                                                        "The model to display",
                                                        GVG_TYPE_MEMCHECK_STORE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private ((gpointer) klass, sizeof (GvgUIPrivate));
}

static gboolean
gvg_ui_filter_kind (GvgUI        *self,
                    GtkTreeModel *model,
                    GtkTreeIter  *iter)
{
  GtkTreeIter           combo_iter;
  GvgMemcheckErrorKind  selected_kind = GVG_MEMCHECK_ERROR_KIND_ANY;
  GvgMemcheckErrorKind  kind;
  GtkTreeIter           parent;
  
  /* always make a decision using a toplevel */
  if (gtk_tree_model_iter_parent (model, &parent, iter)) {
    return gvg_ui_filter_kind (self, model, &parent);
  }
  
  gtk_tree_model_get (model, iter, GVG_MEMCHECK_STORE_COLUMN_KIND, &kind, -1);
  if (kind == GVG_MEMCHECK_ERROR_KIND_ANY) {
    return TRUE;
  }
  
  /* fetch the selected kind */
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (self->priv->kind_combo),
                                     &combo_iter)) {
    gtk_tree_model_get (GTK_TREE_MODEL (self->priv->kind_store), &combo_iter,
                        KIND_COL_KIND, &selected_kind, -1);
  }
  
  return selected_kind == GVG_MEMCHECK_ERROR_KIND_ANY || selected_kind == kind;
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
filter_text_iter_matches (GvgUI        *self,
                          GtkTreeModel *model,
                          GtkTreeIter  *iter)
{
  GtkTreeIter   child;
  const gchar  *filter_text;
  gboolean      match = TRUE;
  
  filter_text = gtk_entry_get_text (GTK_ENTRY (self->priv->filter_entry));
  if (filter_text && *filter_text) {
    gchar  *label;
    gchar  *dir;
    gchar  *file;
    
    gtk_tree_model_get (model, iter,
                        GVG_MEMCHECK_STORE_COLUMN_LABEL, &label,
                        GVG_MEMCHECK_STORE_COLUMN_DIR, &dir,
                        GVG_MEMCHECK_STORE_COLUMN_FILE, &file,
                        -1);
    match = (filter_text_matches (label, filter_text) ||
             filter_text_matches (dir, filter_text) ||
             filter_text_matches (file, filter_text));
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
gvg_ui_filter_text (GvgUI        *self,
                    GtkTreeModel *model,
                    GtkTreeIter  *iter_)
{
  const gchar  *filter_text;
  gboolean      match = FALSE;
  GtkTreeIter   parent;
  GtkTreeIter   iter = *iter_;
  
  while (gtk_tree_model_iter_parent (model, &parent, &iter)) {
    iter = parent;
  }
  
  /* never filter out toplevels without children, they are no entries */
  if (! gtk_tree_model_iter_has_child (model, &iter)) {
    return TRUE;
  }
  
  match = filter_text_iter_matches (self, model, &iter);
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self->priv->filter_invert))) {
    match = ! match;
  }
  
  return match;
}

static gboolean
gvg_ui_filter_func (GtkTreeModel *model,
                    GtkTreeIter  *iter,
                    gpointer      data)
{
  return (gvg_ui_filter_kind (data, model, iter) &&
          gvg_ui_filter_text (data, model, iter));
}

static GtkListStore *
gvg_ui_create_kind_store (void)
{
  GtkListStore *store;
  GEnumClass   *enum_class;
  guint         i;
  
  store = gtk_list_store_new (KIND_N_COLUMNS,
                              GVG_TYPE_MEMCHECK_ERROR_KIND,
                              G_TYPE_STRING);
  enum_class = g_type_class_ref (GVG_TYPE_MEMCHECK_ERROR_KIND);
  for (i = 0; i < enum_class->n_values; i++) {
    GtkTreeIter iter;
    
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        0, enum_class->values[i].value,
                        1, enum_class->values[i].value_nick,
                        -1);
  }
  g_type_class_unref (enum_class);
  
  return store;
}

static GtkWidget *
gvg_ui_create_kind_combo (GtkListStore *store)
{
  GtkWidget        *combo;
  GtkCellRenderer  *cell;
  
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell,
                                 "text", KIND_COL_LABEL);
  
  /* select first row by default */
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  
  return combo;
}

static gboolean
tree_view_toggle_row_expansion (GtkTreeView  *view,
                                GtkTreePath  *path,
                                gboolean      expand_all)
{
  if (gtk_tree_view_row_expanded (view, path)) {
    return gtk_tree_view_collapse_row (view, path);
  } else {
    return gtk_tree_view_expand_row (view, path, expand_all);
  }
}

static void
gvg_ui_view_row_activated (GtkTreeView       *view,
                           GtkTreePath       *path,
                           GtkTreeViewColumn *column,
                           GvgUI             *self)
{
  GtkTreeModel *model = gtk_tree_view_get_model (view);
  GtkTreeIter   iter;
  
  if (gtk_tree_model_get_iter (model, &iter, path)) {
    gchar  *dir;
    gchar  *file;
    guint   line;
    
    gtk_tree_model_get (model, &iter,
                        GVG_MEMCHECK_STORE_COLUMN_DIR, &dir,
                        GVG_MEMCHECK_STORE_COLUMN_FILE, &file,
                        GVG_MEMCHECK_STORE_COLUMN_LINE, &line,
                        -1);
    if (! file) {
      if (gtk_tree_model_iter_has_child (model, &iter)) {
        /* toggle row expansion if it doesn't has a file */
        tree_view_toggle_row_expansion (view, path, FALSE);
      } else {
        g_debug ("cannot open nothing!");
      }
    } else {
      g_debug ("open file %s/%s:%u", dir, file, line);
    }
    g_free (dir);
    g_free (file);
  }
}

static void
gvg_ui_view_ip_column_set_data (GtkCellLayout   *cell_layout,
                                GtkCellRenderer *cell,
                                GtkTreeModel    *model,
                                GtkTreeIter     *iter,
                                gpointer         data)
{
  GvgUI  *self = data;
  guint64 ip;
  gchar  *text;
  
  gtk_tree_model_get (model, iter, GVG_MEMCHECK_STORE_COLUMN_IP, &ip, -1);
  text = ip > 0 ? g_strdup_printf ("%#" G_GINT64_MODIFIER "x", ip) : NULL;
  g_object_set (cell, "text", text, "visible", text != NULL, NULL);
  g_free (text);
}

static gboolean
filter_timeout_func (gpointer data)
{
  GvgUI *self = data;
  
  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (self->priv->filter));
  self->priv->filter_timeout_source = NULL;
  
  return FALSE;
}

/* @now: whether to allow a timeout or not */
static void
gvg_ui_refilter (GvgUI   *self,
                 gboolean now)
{
  if (self->priv->filter_timeout_source) {
    g_source_destroy (self->priv->filter_timeout_source);
  }
  if (now) {
    self->priv->filter_timeout_source = NULL;
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (self->priv->filter));
  } else {
    self->priv->filter_timeout_source = g_timeout_source_new (250);
    g_source_set_callback (self->priv->filter_timeout_source,
                           filter_timeout_func, self, NULL);
    
    g_source_attach (self->priv->filter_timeout_source, NULL);
    g_source_unref (self->priv->filter_timeout_source);
  }
}

static void
gvg_ui_kind_combo_changed_hanlder (GtkComboBox *combo,
                                   GvgUI       *self)
{
  gvg_ui_refilter (self, TRUE);
}

static void
gvg_ui_filter_entry_notify_text_hanlder (GObject     *object,
                                         GParamSpec  *pspec,
                                         GvgUI       *self)
{
  gvg_ui_refilter (self, FALSE);
}

static void
gvg_ui_filter_invert_toggled_hanlder (GtkToggleButton  *button,
                                      GvgUI            *self)
{
  gvg_ui_refilter (self, TRUE);
}

static void
gvg_ui_init (GvgUI *self)
{
  GtkWidget          *hbox;
  GtkWidget          *hbox2;
  GtkWidget          *label;
  GtkWidget          *scroll;
  GtkTreeViewColumn  *col;
  GtkCellRenderer    *cell;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_UI, GvgUIPrivate);
  
  self->priv->store = NULL;
  self->priv->filter = NULL;
  
  /* Top bar */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (self), hbox, FALSE, TRUE, 0);
  
  /* kind filter */
  /* TODO: make unsensitive types that don't exist actually */
  self->priv->kind_store = gvg_ui_create_kind_store ();
  self->priv->kind_combo = gvg_ui_create_kind_combo (self->priv->kind_store);
  gtk_widget_set_tooltip_text (self->priv->kind_combo,
                               _("The error kind to show"));
  g_signal_connect (self->priv->kind_combo, "changed",
                    G_CALLBACK (gvg_ui_kind_combo_changed_hanlder), self);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->kind_combo, FALSE, TRUE, 0);
  
  /* filter entry */
  self->priv->filter_entry = gtk_entry_new ();
  gtk_widget_set_tooltip_text (self->priv->filter_entry,
                               _("A text the error should contain"));
  g_signal_connect (self->priv->filter_entry, "notify::text",
                    G_CALLBACK (gvg_ui_filter_entry_notify_text_hanlder), self);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->filter_entry, TRUE, TRUE, 0);
  self->priv->filter_invert = gtk_check_button_new_with_label (_("Reverse match"));
  g_signal_connect (self->priv->filter_invert, "toggled",
                    G_CALLBACK (gvg_ui_filter_invert_toggled_hanlder), self);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->filter_invert, FALSE, TRUE, 0);
  
  
  /* The view */
  scroll = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                         "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
                         "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                         NULL);
  gtk_box_pack_start (GTK_BOX (self), scroll, TRUE, TRUE, 0);
  
  self->priv->view = g_object_new (GTK_TYPE_TREE_VIEW,
                                   /*"model", self->priv->filter,*/
                                   "headers-visible", FALSE,
                                   NULL);
  gtk_container_add (GTK_CONTAINER (scroll), self->priv->view);
  g_signal_connect (self->priv->view, "row-activated",
                    G_CALLBACK (gvg_ui_view_row_activated), self);
  /* label column */
  cell = gtk_cell_renderer_text_new ();
  col = gtk_tree_view_column_new_with_attributes ("Error", cell,
                                                  "text", GVG_MEMCHECK_STORE_COLUMN_LABEL,
                                                  NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->view), col);
  /* frame IP column */
  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, /*"font", "monospace",*/ NULL);
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", "IP", NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (col), cell, FALSE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (col), cell,
                                      gvg_ui_view_ip_column_set_data,
                                      self, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->view), col);
  
  gtk_widget_show_all (hbox);
  gtk_widget_show_all (scroll);
}


GtkWidget *
gvg_ui_new (GvgMemcheckStore *model)
{
  g_return_val_if_fail (model == NULL || GVG_IS_MEMCHECK_STORE (model), NULL);
  
  if (! model) {
    model = gvg_memcheck_store_new ();
  }
  
  return g_object_new (GVG_TYPE_UI, "model", model, NULL);
}

GvgMemcheckStore *
gvg_ui_get_model (GvgUI *self)
{
  g_return_val_if_fail (GVG_IS_UI (self), NULL);
  
  return self->priv->store;
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

void
gvg_ui_set_model (GvgUI            *self,
                  GvgMemcheckStore *model)
{
  g_return_if_fail (GVG_IS_UI (self));
  g_return_if_fail (GVG_IS_MEMCHECK_STORE (model));
  
  self->priv->store = model;
  g_signal_connect (self->priv->store, "row-inserted",
                    G_CALLBACK (force_refresh_entry), self);
  g_signal_connect (self->priv->store, "row-changed",
                    G_CALLBACK (force_refresh_entry), self);
  self->priv->filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (self->priv->store),
                                                  NULL);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (self->priv->filter),
                                          gvg_ui_filter_func, self, NULL);
  gtk_tree_view_set_model (GTK_TREE_VIEW (self->priv->view),
                           self->priv->filter);
  
  g_object_notify (G_OBJECT (self), "model");
}
