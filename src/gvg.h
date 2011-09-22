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

#ifndef H_GVG
#define H_GVG

#include <glib.h>
#include <glib-object.h>

#include "gvg-xml-parser.h"
#include "gvg-options.h"
#include "gvg-args-builder.h"

G_BEGIN_DECLS


#define GVG_TYPE_GVG            (gvg_get_type ())
#define GVG(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_GVG, Gvg))
#define GVG_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_GVG, GvgClass))
#define GVG_IS_GVG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_GVG))
#define GVG_IS_GVG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_GVG))
#define GVG_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_GVG, GvgClass))


typedef enum
{
  GVG_ROW_TYPE_OTHER,
  GVG_ROW_TYPE_ERROR,
  GVG_ROW_TYPE_FRAME,
  GVG_ROW_TYPE_STATUS
} GvgRowType;

typedef struct _Gvg         Gvg;
typedef struct _GvgClass    GvgClass;
typedef struct _GvgPrivate  GvgPrivate;

struct _Gvg
{
  GObject     parent;
  GvgPrivate *priv;
};

struct _GvgClass
{
  GObjectClass parent_class;
};


GType         gvg_get_type          (void) G_GNUC_CONST;
gboolean      gvg_run               (Gvg           *self,
                                     const gchar  **program_argv,
                                     GError       **error);
gboolean      gvg_is_busy           (Gvg *self);


G_END_DECLS

#endif /* guard */
