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

#ifndef H_GVG_MEMCHECK
#define H_GVG_MEMCHECK

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "gvg.h"
#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-options.h"

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK             (gvg_memcheck_get_type ())
#define GVG_MEMCHECK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK, GvgMemcheck))
#define GVG_MEMCHECK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK, GvgMemcheckClass))
#define GVG_IS_MEMCHECK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK))
#define GVG_IS_MEMCHECK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK))
#define GVG_MEMCHECK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK, GvgMemcheckClass))


typedef enum {
  GVG_MEMCHECK_LEAK_CHECK_NO,
  GVG_MEMCHECK_LEAK_CHECK_SUMMARY,
  GVG_MEMCHECK_LEAK_CHECK_YES,
  GVG_MEMCHECK_LEAK_CHECK_FULL
} GvgMemcheckLeakCheckMode;

typedef enum {
  GVG_MEMCHECK_LEAK_RESOLUTION_LOW,
  GVG_MEMCHECK_LEAK_RESOLUTION_MED,
  GVG_MEMCHECK_LEAK_RESOLUTION_HIGH
} GvgMemcheckLeakResolutionMode;


typedef struct _GvgMemcheck         GvgMemcheck;
typedef struct _GvgMemcheckClass    GvgMemcheckClass;

struct _GvgMemcheck
{
  Gvg parent;
};

struct _GvgMemcheckClass
{
  GvgClass parent_class;
};


GType               gvg_memcheck_get_type       (void) G_GNUC_CONST;
GvgMemcheck        *gvg_memcheck_new            (GvgMemcheckOptions *options,
                                                 GvgMemcheckParser  *parser);


G_END_DECLS

#endif /* guard */
