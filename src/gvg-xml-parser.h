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

#ifndef H_GVG_XML_PARSER
#define H_GVG_XML_PARSER

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define GVG_TYPE_XML_PARSER             (gvg_xml_parser_get_type ())
#define GVG_XML_PARSER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_XML_PARSER, GvgXmlParser))
#define GVG_XML_PARSER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GVG_TYPE_XML_PARSER, GvgXmlParserClass))
#define GVG_IS_XML_PARSER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_XML_PARSER))
#define GVG_IS_XML_PARSER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GVG_TYPE_XML_PARSER))
#define GVG_XML_PARSER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GVG_TYPE_XML_PARSER, GvgXmlParserClass))


typedef struct _GvgXmlParser        GvgXmlParser;
typedef struct _GvgXmlParserClass   GvgXmlParserClass;
typedef struct _GvgXmlParserPrivate GvgXmlParserPrivate;

struct _GvgXmlParser
{
  GObject parent;
  GvgXmlParserPrivate *priv;
};

struct _GvgXmlParserClass
{
  GObjectClass  parent_class;
  
  void        (*element_start)    (GvgXmlParser  *self,
                                   const gchar   *name,
                                   const gchar  **attrs,
                                   const gchar   *path);
  void        (*element_end)      (GvgXmlParser  *self,
                                   const gchar   *name,
                                   const gchar   *content,
                                   const gchar   *path);
};


GType           gvg_xml_parser_get_type     (void) G_GNUC_CONST;
gboolean        gvg_xml_parser_push         (GvgXmlParser  *parser,
                                             const gchar   *data,
                                             gsize          len,
                                             gboolean       end);


G_END_DECLS

#endif /* guard */
