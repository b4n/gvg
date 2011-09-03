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

G_BEGIN_DECLS


typedef struct _GvgXmlParser GvgXmlParser;


typedef void    (*GvgXmlStartElementFunc)   (const gchar   *name,
                                             const gchar  **attrs,
                                             const gchar   *path,
                                             gpointer       data);
typedef void    (*GvgXmlEndElementFunc)     (const gchar   *name,
                                             const gchar   *content,
                                             const gchar   *path,
                                             gpointer       data);


GvgXmlParser   *gvg_xml_parser_new  (GvgXmlStartElementFunc start_element_func,
                                     GvgXmlEndElementFunc   end_element_func,
                                     gpointer               data,
                                     GDestroyNotify         data_notify);
void            gvg_xml_parser_free (GvgXmlParser  *parser);
gboolean        gvg_xml_parser_push (GvgXmlParser  *parser,
                                     const gchar   *data,
                                     gsize          len,
                                     gboolean       end);


G_END_DECLS

#endif /* guard */
