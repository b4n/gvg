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


#ifndef H_GVG_MEMCHECK_STORE_FILTER
#define H_GVG_MEMCHECK_STORE_FILTER

#include <gtk/gtk.h>

#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-store.h"

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK_STORE_FILTER            (gvg_memcheck_store_filter_get_type ())
#define GVG_MEMCHECK_STORE_FILTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK_STORE_FILTER, GvgMemcheckStoreFilter))
#define GVG_MEMCHECK_STORE_FILTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK_STORE_FILTER, GvgMemcheckStoreFilterClass))
#define GVG_IS_MEMCHECK_STORE_FILTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK_STORE_FILTER))
#define GVG_IS_MEMCHECK_STORE_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK_STORE_FILTER))
#define GVG_MEMCHECK_STORE_FILTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK_STORE_FILTER, GvgMemcheckStoreFilterClass))


typedef struct _GvgMemcheckStoreFilter        GvgMemcheckStoreFilter;
typedef struct _GvgMemcheckStoreFilterClass   GvgMemcheckStoreFilterClass;
typedef struct _GvgMemcheckStoreFilterPrivate GvgMemcheckStoreFilterPrivate;

struct _GvgMemcheckStoreFilter
{
  GtkTreeModelFilter              parent;
  GvgMemcheckStoreFilterPrivate  *priv;
};

struct _GvgMemcheckStoreFilterClass
{
  GtkTreeModelFilterClass parent_class;
};


GType                 gvg_memcheck_store_filter_get_type    (void) G_GNUC_CONST;
GtkTreeModel         *gvg_memcheck_store_filter_new         (GvgMemcheckStore  *model,
                                                             GtkTreePath       *root);
GvgMemcheckErrorKind  gvg_memcheck_store_filter_get_kind    (GvgMemcheckStoreFilter *self);
void                  gvg_memcheck_store_filter_set_kind    (GvgMemcheckStoreFilter *self,
                                                             GvgMemcheckErrorKind    kind);
const gchar          *gvg_memcheck_store_filter_get_text    (GvgMemcheckStoreFilter *self);
void                  gvg_memcheck_store_filter_set_text    (GvgMemcheckStoreFilter *self,
                                                             const gchar            *text);
gboolean              gvg_memcheck_store_filter_get_invert  (GvgMemcheckStoreFilter *self);
void                  gvg_memcheck_store_filter_set_invert  (GvgMemcheckStoreFilter *self,
                                                             gboolean                invert);


G_END_DECLS

#endif /* guard */
