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

#ifndef H_GVG_MEMCHECK_STORE
#define H_GVG_MEMCHECK_STORE

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK_STORE             (gvg_memcheck_store_get_type ())
#define GVG_MEMCHECK_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK_STORE, GvgMemcheckStore))
#define GVG_MEMCHECK_STORE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK_STORE, GvgMemcheckStoreClass))
#define GVG_IS_MEMCHECK_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK_STORE))
#define GVG_IS_MEMCHECK_STORE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK_STORE))
#define GVG_MEMCHECK_STORE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK_STORE, GvgMemcheckStoreClass))


enum
{
  GVG_MEMCHECK_STORE_COLUMN_TYPE,
  GVG_MEMCHECK_STORE_COLUMN_LABEL,
  GVG_MEMCHECK_STORE_COLUMN_IP,
  GVG_MEMCHECK_STORE_COLUMN_DIR,
  GVG_MEMCHECK_STORE_COLUMN_FILE,
  GVG_MEMCHECK_STORE_COLUMN_LINE,
  GVG_MEMCHECK_STORE_COLUMN_KIND,
  
  GVG_MEMCHECK_STORE_N_COLUMNS
};

typedef struct _GvgMemcheckStore      GvgMemcheckStore;
typedef struct _GvgMemcheckStoreClass GvgMemcheckStoreClass;

struct _GvgMemcheckStore
{
  GtkTreeStore parent_instance;
};

struct _GvgMemcheckStoreClass
{
  GtkTreeStoreClass parent_class;
};


GType             gvg_memcheck_store_get_type         (void) G_GNUC_CONST;
GvgMemcheckStore *gvg_memcheck_store_new              (void);


G_END_DECLS

#endif /* guard */
