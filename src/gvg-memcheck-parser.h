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

#ifndef H_GVG_MEMCHECK_PARSER
#define H_GVG_MEMCHECK_PARSER

#include <glib.h>
#include <glib-object.h>

#include "gvg-xml-parser.h"
#include "gvg-memcheck-store.h"

G_BEGIN_DECLS


#define GVG_TYPE_MEMCHECK_PARSER             (gvg_memcheck_parser_get_type ())
#define GVG_MEMCHECK_PARSER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_MEMCHECK_PARSER, GvgMemcheckParser))
#define GVG_MEMCHECK_PARSER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_MEMCHECK_PARSER, GvgMemcheckParserClass))
#define GVG_IS_MEMCHECK_PARSER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_MEMCHECK_PARSER))
#define GVG_IS_MEMCHECK_PARSER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_MEMCHECK_PARSER))
#define GVG_MEMCHECK_PARSER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_MEMCHECK_PARSER, GvgMemcheckParserClass))


typedef enum {
  GVG_MEMCHECK_ERROR_KIND_ANY,
  GVG_MEMCHECK_ERROR_KIND_INVALID_FREE,
  GVG_MEMCHECK_ERROR_KIND_MISMATCHED_FREE,
  GVG_MEMCHECK_ERROR_KIND_INVALID_READ,
  GVG_MEMCHECK_ERROR_KIND_INVALID_WRITE,
  GVG_MEMCHECK_ERROR_KIND_INVALID_JUMP,
  GVG_MEMCHECK_ERROR_KIND_OVERLAP,
  GVG_MEMCHECK_ERROR_KIND_INVALID_MEM_POOL,
  GVG_MEMCHECK_ERROR_KIND_UNINIT_CONDITION,
  GVG_MEMCHECK_ERROR_KIND_UNINIT_VALUE,
  GVG_MEMCHECK_ERROR_KIND_SYSCALL_PARAM,
  GVG_MEMCHECK_ERROR_KIND_CLIENT_CHECK,
  GVG_MEMCHECK_ERROR_KIND_LEAK_DEFINITELY_LOST,
  GVG_MEMCHECK_ERROR_KIND_LEAK_INDIRECTLY_LOST,
  GVG_MEMCHECK_ERROR_KIND_LEAK_POSSIBLY_LOST,
  GVG_MEMCHECK_ERROR_KIND_LEAK_STILL_REACHABLE
} GvgMemcheckErrorKind;

typedef struct _GvgMemcheckParser         GvgMemcheckParser;
typedef struct _GvgMemcheckParserClass    GvgMemcheckParserClass;
typedef struct _GvgMemcheckParserPrivate  GvgMemcheckParserPrivate;

struct _GvgMemcheckParser
{
  GvgXmlParser              parent;
  GvgMemcheckParserPrivate *priv;
};

struct _GvgMemcheckParserClass
{
  GvgXmlParserClass parent_class;
};


GType             gvg_memcheck_parser_get_type    (void) G_GNUC_CONST;
GvgXmlParser     *gvg_memcheck_parser_new         (GvgMemcheckStore *store);


G_END_DECLS

#endif /* guard */
