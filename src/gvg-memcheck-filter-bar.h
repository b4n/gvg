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


#ifndef H_GVG_MEMCHECK_FILTER_BAR
#define H_GVG_MEMCHECK_FILTER_BAR

#include <gtk/gtk.h>

#include "gvg-memcheck-parser.h"

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK_FILTER_BAR             (gvg_memcheck_filter_bar_get_type ())
#define GVG_MEMCHECK_FILTER_BAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK_FILTER_BAR, GvgMemcheckFilterBar))
#define GVG_MEMCHECK_FILTER_BAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK_FILTER_BAR, GvgMemcheckFilterBarClass))
#define GVG_IS_MEMCHECK_FILTER_BAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK_FILTER_BAR))
#define GVG_IS_MEMCHECK_FILTER_BAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK_FILTER_BAR))
#define GVG_MEMCHECK_FILTER_BAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK_FILTER_BAR, GvgMemcheckFilterBarClass))


typedef struct _GvgMemcheckFilterBar         GvgMemcheckFilterBar;
typedef struct _GvgMemcheckFilterBarClass    GvgMemcheckFilterBarClass;
typedef struct _GvgMemcheckFilterBarPrivate  GvgMemcheckFilterBarPrivate;

struct _GvgMemcheckFilterBar
{
  GtkHBox                       parent;
  GvgMemcheckFilterBarPrivate  *priv;
};

struct _GvgMemcheckFilterBarClass
{
  GtkHBoxClass parent_class;
};


GType                 gvg_memcheck_filter_bar_get_type    (void) G_GNUC_CONST;
GtkWidget            *gvg_memcheck_filter_bar_new         (void);
GvgMemcheckErrorKind  gvg_memcheck_filter_bar_get_kind    (GvgMemcheckFilterBar  *self);
void                  gvg_memcheck_filter_bar_set_kind    (GvgMemcheckFilterBar  *self,
                                                           GvgMemcheckErrorKind   kind);
const gchar          *gvg_memcheck_filter_bar_get_text    (GvgMemcheckFilterBar  *self);
void                  gvg_memcheck_filter_bar_set_text    (GvgMemcheckFilterBar  *self,
                                                           const gchar           *text);
gboolean              gvg_memcheck_filter_bar_get_invert  (GvgMemcheckFilterBar  *self);
void                  gvg_memcheck_filter_bar_set_invert  (GvgMemcheckFilterBar  *self,
                                                           gboolean               invert);


G_END_DECLS

#endif /* guard */
