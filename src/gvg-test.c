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

#include <glib.h>
#include <gtk/gtk.h>

#include "gvg-memcheck.h"
#include "gvg-memcheck-store.h"


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
view_row_activated (GtkTreeView        *view,
                    GtkTreePath        *path,
                    GtkTreeViewColumn  *column,
                    gpointer            data)
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
set_ip_data (GtkCellLayout   *cell_layout,
             GtkCellRenderer *cell,
             GtkTreeModel    *tree_model,
             GtkTreeIter     *iter,
             gpointer         data)
{
  guint64   ip;
  gchar    *text;
  
  gtk_tree_model_get (tree_model, iter, GVG_MEMCHECK_STORE_COLUMN_IP, &ip, -1);
  text = ip > 0 ? g_strdup_printf ("%#lx", (gulong) ip) : NULL;
  g_object_set (cell, "text", text, "visible", text != NULL, NULL);
  g_free (text);
}

int
main (int     argc,
      char  **argv)
{
  GtkWidget          *window;
  GtkWidget          *scroll;
  GtkWidget          *view;
  GtkTreeViewColumn  *col;
  GtkCellRenderer    *cell;
  GvgMemcheckStore   *store;
  
  gtk_init (&argc, &argv);
  
  store = gvg_memcheck_store_new ();
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", gtk_main_quit, NULL);
  
  scroll = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                         "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
                         "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                         NULL);
  gtk_container_add (GTK_CONTAINER (window), scroll);
  
  view = g_object_new (GTK_TYPE_TREE_VIEW,
                       "model", store,
                       //~ "headers-visible", FALSE,
                       NULL);
  gtk_container_add (GTK_CONTAINER (scroll), view);
  g_signal_connect (view, "row-activated",
                    G_CALLBACK (view_row_activated), NULL);
  /* label column */
  cell = gtk_cell_renderer_text_new ();
  col = gtk_tree_view_column_new_with_attributes ("Error", cell,
                                                  "text", GVG_MEMCHECK_STORE_COLUMN_LABEL,
                                                  NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
  /* frame IP column */
  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, /*"font", "monospace",*/ NULL);
  col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN, "title", "IP", NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (col), cell, FALSE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (col), cell,
                                      set_ip_data, store, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
  
  if (argc > 1) {
    GvgMemcheck        *memcheck;
    GvgMemcheckOptions *options;
    GvgMemcheckParser  *parser;
    GError *err = NULL;
    
    options = gvg_memcheck_options_new ();
    parser = gvg_memcheck_parser_new (store);
    memcheck = gvg_memcheck_new (options, parser);
    g_object_unref (parser);
    if (! gvg_run (GVG (memcheck), (const gchar **) &argv[1], &err)) {
      g_warning ("failed to run memcheck: %s", err->message);
      g_error_free (err);
      return 1;
    }
    /*g_object_unref (memcheck);*/
  }
  
  gtk_widget_show_all (window);
  gtk_main ();
  
  return 0;
}
