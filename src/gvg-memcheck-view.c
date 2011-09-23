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

#include "gvg-memcheck-view.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gvg.h"
#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-store.h"


G_DEFINE_TYPE (GvgMemcheckView,
               gvg_memcheck_view,
               GTK_TYPE_TREE_VIEW)


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
gvg_memcheck_view_row_activated (GtkTreeView       *view,
                                 GtkTreePath       *path,
                                 GtkTreeViewColumn *column)
{
  GvgMemcheckView  *self = GVG_MEMCHECK_VIEW (view);
  GtkTreeModel     *model = gtk_tree_view_get_model (view);
  GtkTreeIter       iter;
  
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
gvg_memcheck_view_class_init (GvgMemcheckViewClass *klass)
{
  GtkTreeViewClass *tree_view_class = GTK_TREE_VIEW_CLASS (klass);
  
  tree_view_class->row_activated = gvg_memcheck_view_row_activated;
}

static void
gvg_memcheck_view_label_column_set_data (GtkCellLayout   *cell_layout,
                                         GtkCellRenderer *cell,
                                         GtkTreeModel    *model,
                                         GtkTreeIter     *iter,
                                         gpointer         data)
{
  GvgMemcheckView  *self = data;
  GvgRowType        type;
  gchar            *label;
  PangoStyle        style;
  
  gtk_tree_model_get (model, iter,
                      GVG_MEMCHECK_STORE_COLUMN_TYPE, &type,
                      GVG_MEMCHECK_STORE_COLUMN_LABEL, &label,
                      -1);
  switch (type) {
    case GVG_ROW_TYPE_OTHER:
    case GVG_ROW_TYPE_STATUS: style = PANGO_STYLE_ITALIC; break;
    default:                  style = PANGO_STYLE_NORMAL; break;
  }
  
  g_object_set (cell, "text", label, "style", style, NULL);
  g_free (label);
}

static void
gvg_memcheck_view_ip_column_set_data (GtkCellLayout   *cell_layout,
                                      GtkCellRenderer *cell,
                                      GtkTreeModel    *model,
                                      GtkTreeIter     *iter,
                                      gpointer         data)
{
  GvgMemcheckView  *self = data;
  guint64           ip;
  gchar            *text;
  
  gtk_tree_model_get (model, iter, GVG_MEMCHECK_STORE_COLUMN_IP, &ip, -1);
  text = ip > 0 ? g_strdup_printf ("%#" G_GINT64_MODIFIER "x", ip) : NULL;
  g_object_set (cell, "text", text, "visible", text != NULL, NULL);
  g_free (text);
}

static void
gvg_memcheck_view_init (GvgMemcheckView *self)
{
  GtkTreeViewColumn  *col;
  GtkCellRenderer    *cell;
  
  /* label column */
  cell = gtk_cell_renderer_text_new ();
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", _("Error"), NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (col), cell, TRUE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (col), cell,
                                      gvg_memcheck_view_label_column_set_data,
                                      self, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), col);
  /* frame IP column */
  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, /*"font", "monospace",*/ NULL);
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", "IP", NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (col), cell, FALSE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (col), cell,
                                      gvg_memcheck_view_ip_column_set_data,
                                      self, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), col);
}


GtkWidget *
gvg_memcheck_view_new (GtkTreeModel *model)
{
  return g_object_new (GVG_TYPE_MEMCHECK_VIEW, "model", model, NULL);
}
