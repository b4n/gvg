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
#include "gvg-ui.h"

int
main (int     argc,
      char  **argv)
{
  GtkWidget          *window;
  GvgMemcheckStore   *store;
  GtkWidget          *ui;
  
  gtk_init (&argc, &argv);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", gtk_main_quit, NULL);
  
  store = gvg_memcheck_store_new ();
  
  ui = gvg_ui_new (store);
  gtk_container_add (GTK_CONTAINER (window), ui);
  
  if (argc > 1) {
    GvgMemcheck        *memcheck;
    GvgMemcheckOptions *options;
    GvgMemcheckParser  *parser;
    GError *err = NULL;
    
    options = gvg_memcheck_options_new ();
    parser = GVG_MEMCHECK_PARSER (gvg_memcheck_parser_new (store));
    memcheck = gvg_memcheck_new (options, parser);
    g_object_unref (parser);
    if (! gvg_run (GVG (memcheck), (const gchar **) &argv[1], &err)) {
      g_warning ("failed to run memcheck: %s", err->message);
      g_error_free (err);
      return 1;
    }
  }
  
  gtk_widget_show_all (window);
  gtk_main ();
  
  return 0;
}
