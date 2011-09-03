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

#ifndef H_GVG_ARGS_BUILDER
#define H_GVG_ARGS_BUILDER

#include <glib.h>

G_BEGIN_DECLS


typedef GPtrArray GvgArgsBuilder;

#define gvg_args_builder_new \
  g_ptr_array_new
#define gvg_args_builder_free(bld, free_segment) \
  ((gchar **) g_ptr_array_free ((bld), (free_segment)))

#define gvg_args_builder_take \
  g_ptr_array_add
#define gvg_args_builder_add(bld, arg) \
  (gvg_args_builder_take ((bld), g_strdup (arg)))

#define gvg_args_builder_add_arg(bld, arg, fmt, val) \
  (gvg_args_builder_take ((bld), g_strdup_printf ("--%s=" fmt, (arg), (val))))
#define gvg_args_builder_add_string(bld, arg, val) \
  (gvg_args_builder_add_arg ((bld), (arg), "%s", (val)))
#define gvg_args_builder_add_bool(bld, arg, val) \
  (gvg_args_builder_add_string ((bld), (arg), (val) ? "yes" : "no"))
#define gvg_args_builder_add_uint(bld, arg, val) \
  (gvg_args_builder_add_arg ((bld), (arg), "%u", (val)))
#define gvg_args_builder_add_ssize(bld, arg, val) \
  (gvg_args_builder_add_arg ((bld), (arg), "%" G_GSSIZE_FORMAT, (val)))

static inline void
gvg_args_builder_add_ssize_checked (GvgArgsBuilder *bld,
                                    const gchar    *arg,
                                    gssize          val)
{
  if (val != -1) {
    gvg_args_builder_add_ssize (bld, arg, val);
  }
}

static inline void
gvg_args_builder_add_byte_checked (GvgArgsBuilder *bld,
                                   const gchar    *arg,
                                   gint            val)
{
  if (val >= 0) {
    gvg_args_builder_add_arg (bld, arg, "%x", (guint) val);
  }
}

static inline void
gvg_args_builder_add_args (GvgArgsBuilder *bld,
                           const gchar   **args)
{
  for (; *args; args ++) {
    gvg_args_builder_add (bld, *args);
  }
}


G_END_DECLS

#endif /* guard */
