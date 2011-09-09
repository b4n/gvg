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

#include "gvg-memcheck-store.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "gvg-enum-types.h"


G_DEFINE_TYPE (GvgMemcheckStore,
               gvg_memcheck_store,
               GTK_TYPE_TREE_STORE)


static void
gvg_memcheck_store_class_init (GvgMemcheckStoreClass *klass)
{
  
}

static void
gvg_memcheck_store_init (GvgMemcheckStore *self)
{
  GType column_types[GVG_MEMCHECK_STORE_N_COLUMNS] = { 0 };
  
  column_types[GVG_MEMCHECK_STORE_COLUMN_LABEL] = G_TYPE_STRING;
  column_types[GVG_MEMCHECK_STORE_COLUMN_IP]    = G_TYPE_UINT64;
  column_types[GVG_MEMCHECK_STORE_COLUMN_DIR]   = G_TYPE_STRING;
  column_types[GVG_MEMCHECK_STORE_COLUMN_FILE]  = G_TYPE_STRING;
  column_types[GVG_MEMCHECK_STORE_COLUMN_LINE]  = G_TYPE_UINT;
  column_types[GVG_MEMCHECK_STORE_COLUMN_KIND]  = GVG_TYPE_MEMCHECK_ERROR_KIND;
  
  gtk_tree_store_set_column_types (GTK_TREE_STORE (self),
                                   G_N_ELEMENTS (column_types), column_types);
}

GvgMemcheckStore *
gvg_memcheck_store_new (void)
{
  return g_object_new (GVG_TYPE_MEMCHECK_STORE, NULL);
}
