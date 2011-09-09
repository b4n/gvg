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

#ifndef H_GVG_MEMCHECK_OPTIONS
#define H_GVG_MEMCHECK_OPTIONS

#include <glib.h>
#include <glib-object.h>

#include "gvg-options.h"

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK_OPTIONS            (gvg_memcheck_options_get_type ())
#define GVG_MEMCHECK_OPTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK_OPTIONS, GvgMemcheckOptions))
#define GVG_MEMCHECK_OPTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK_OPTIONS, GvgMemcheckOptionsClass))
#define GVG_IS_MEMCHECK_OPTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK_OPTIONS))
#define GVG_IS_MEMCHECK_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK_OPTIONS))
#define GVG_MEMCHECK_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK_OPTIONS, GvgMemcheckOptionsClass))


typedef struct _GvgMemcheckOptions              GvgMemcheckOptions;
typedef struct _GvgMemcheckOptionsClass         GvgMemcheckOptionsClass;
typedef struct _GvgMemcheckOptionsPrivate       GvgMemcheckOptionsPrivate;

struct _GvgMemcheckOptions
{
  GvgOptions                  parent;
  GvgMemcheckOptionsPrivate  *priv;
};

struct _GvgMemcheckOptionsClass
{
  GvgOptionsClass parent_class;
};


GType               gvg_memcheck_options_get_type     (void) G_GNUC_CONST;
GvgMemcheckOptions *gvg_memcheck_options_new          (void);


G_END_DECLS

#endif /* guard */
