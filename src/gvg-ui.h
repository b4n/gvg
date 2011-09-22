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


#ifndef H_GVG_UI
#define H_GVG_UI

#include <gtk/gtk.h>

#include "gvg-memcheck-store.h"

G_BEGIN_DECLS


#define GVG_TYPE_UI             (gvg_ui_get_type ())
#define GVG_UI(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_UI, GvgUI))
#define GVG_UI_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_UI, GvgUIClass))
#define GVG_IS_UI(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_UI))
#define GVG_IS_UI_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_UI))
#define GVG_UI_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_UI, GvgUIClass))


typedef struct _GvgUI         GvgUI;
typedef struct _GvgUIClass    GvgUIClass;
typedef struct _GvgUIPrivate  GvgUIPrivate;

struct _GvgUI
{
  GtkVBox       parent;
  GvgUIPrivate *priv;
};

struct _GvgUIClass
{
  GtkVBoxClass parent_class;
};


GType             gvg_ui_get_type     (void) G_GNUC_CONST;
GtkWidget        *gvg_ui_new          (GvgMemcheckStore  *model);
GvgMemcheckStore *gvg_ui_get_model    (GvgUI *self);
void              gvg_ui_set_model    (GvgUI             *self,
                                       GvgMemcheckStore  *model);


G_END_DECLS

#endif /* guard */
