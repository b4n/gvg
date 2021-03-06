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

#include "gvg-memcheck.h"

#include <glib.h>
#include <glib-object.h>

#include "gvg.h"
#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-options.h"


G_DEFINE_TYPE (GvgMemcheck,
               gvg_memcheck,
               GVG_TYPE_GVG)


static void
gvg_memcheck_class_init (GvgMemcheckClass *klass)
{
}

static void
gvg_memcheck_init (GvgMemcheck *self)
{
}

GvgMemcheck *
gvg_memcheck_new (GvgMemcheckOptions *options,
                  GvgMemcheckParser  *parser)
{
  return g_object_new (GVG_TYPE_MEMCHECK,
                       "options", options,
                       "parser", parser,
                       NULL);
}
