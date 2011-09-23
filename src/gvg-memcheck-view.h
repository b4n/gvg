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


#ifndef H_GVG_MEMCHECK_VIEW
#define H_GVG_MEMCHECK_VIEW

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK_VIEW            (gvg_memcheck_view_get_type ())
#define GVG_MEMCHECK_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK_VIEW, GvgMemcheckView))
#define GVG_MEMCHECK_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK_VIEW, GvgMemcheckViewClass))
#define GVG_IS_MEMCHECK_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK_VIEW))
#define GVG_IS_MEMCHECK_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK_VIEW))
#define GVG_MEMCHECK_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK_VIEW, GvgMemcheckViewClass))


typedef struct _GvgMemcheckView         GvgMemcheckView;
typedef struct _GvgMemcheckViewClass    GvgMemcheckViewClass;

struct _GvgMemcheckView
{
  GtkTreeView parent;
};

struct _GvgMemcheckViewClass
{
  GtkTreeViewClass  parent_class;
  
  void            (*file_activated)     (GvgMemcheckView *view,
                                         const gchar     *dir,
                                         const gchar     *file,
                                         guint            line);
  void            (*object_activated)   (GvgMemcheckView *view,
                                         const gchar     *object);
};


GType             gvg_memcheck_view_get_type    (void) G_GNUC_CONST;
GtkWidget        *gvg_memcheck_view_new         (GtkTreeModel *model);


G_END_DECLS

#endif /* guard */
