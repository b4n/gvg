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

#ifndef H_GVG_OPTIONS
#define H_GVG_OPTIONS

#include <glib.h>
#include <glib-object.h>

#include "gvg-args-builder.h"

G_BEGIN_DECLS


#define GVG_TYPE_OPTIONS            (gvg_options_get_type ())
#define GVG_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_OPTIONS, GvgOptions))
#define GVG_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_OPTIONS, GvgOptionsClass))
#define GVG_IS_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_OPTIONS))
#define GVG_IS_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_OPTIONS))
#define GVG_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_OPTIONS, GvgOptionsClass))


typedef struct _GvgOptions              GvgOptions;
typedef struct _GvgOptionsClass         GvgOptionsClass;
typedef struct _GvgOptionsPrivate       GvgOptionsPrivate;

struct _GvgOptions
{
  GObject             parent;
  GvgOptionsPrivate  *priv;
};

struct _GvgOptionsClass
{
  GObjectClass parent_class;
};


GType         gvg_options_get_type      (void) G_GNUC_CONST;
GvgOptions   *gvg_options_new           (void);
void          gvg_options_to_args       (GvgOptions     *self,
                                         GvgArgsBuilder *builder);


G_END_DECLS

#endif /* guard */
