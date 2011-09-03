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

/*
 * A naive helper for progressive XML parsing using LibXML2.
 * 
 * It notifies the caller about open and close elements, with full the
 * element's full path (e.g. /root/node/leaf).
 * 
 * It can also gives a part of an element's content upon element close.
 * It only provides the data between the previous closed tag and this one,
 * but maybe it could be improved to contain the whole element's content.
 */

#include "gvg-xml-parser.h"

#include <glib.h>
#include <libxml/parser.h>


struct _GvgXmlParser {
  xmlSAXHandler           saxh;
  xmlParserCtxtPtr        ctxt;
  
  GString                *content;
  GString                *path;
  guint                   depth;
  
  GvgXmlStartElementFunc  user_start_element_func;
  GvgXmlEndElementFunc    user_end_element_func;
  gpointer                user_data;
  GDestroyNotify          user_data_notify;
};




static void
parser_path_push (GvgXmlParser *parser,
                  const gchar  *element)
{
  g_string_append_c (parser->path, '/');
  g_string_append (parser->path, element);
  parser->depth ++;
}

static void
parser_path_pop (GvgXmlParser *parser)
{
  gsize i;
  
  if (parser->path->len == 0) {
    return;
  }
  
  parser->depth--;
  for (i = parser->path->len; i-- > 0; ) {
    if (parser->path->str[i] == '/') {
      g_string_truncate (parser->path, i);
      break;
    }
  }
}


static void
gvg_xml_parser_start_element_hanlder (void            *data,
                                      const xmlChar   *name,
                                      const xmlChar  **atts)
{
  GvgXmlParser *parser = data;
  
  parser_path_push (parser, (const gchar *) name);
  //~ g_debug ("start element %s", parser->path->str);
  
  if (parser->user_start_element_func) {
    parser->user_start_element_func ((const gchar *) name,
                                     (const gchar **) atts,
                                     parser->path->str,
                                     parser->user_data);
  }
  g_string_truncate (parser->content, 0);
}

static void
gvg_xml_parser_end_element_handler (void          *data,
                                    const xmlChar *name)
{
  GvgXmlParser *parser = data;
  
  //~ g_debug ("end element %s", parser->path->str);
  if (parser->user_end_element_func) {
    parser->user_end_element_func ((const gchar *) name, parser->content->str,
                                   parser->path->str, parser->user_data);
  }
  parser_path_pop (parser);
  
  g_string_truncate (parser->content, 0);
}

static void
gvg_xml_parser_characters_handler (void           *data,
                                   const xmlChar  *chs,
                                   int             len)
{
  GvgXmlParser *parser = data;
  
  g_string_append_len (parser->content, (const gchar *) chs, len);
}

GvgXmlParser *
gvg_xml_parser_new (GvgXmlStartElementFunc  start_element_func,
                    GvgXmlEndElementFunc    end_element_func,
                    gpointer                data,
                    GDestroyNotify          data_notify)
{
  GvgXmlParser *parser;
  
  parser = g_malloc0 (sizeof *parser);
  parser->ctxt                    = NULL;
  parser->saxh.startElement       = gvg_xml_parser_start_element_hanlder;
  parser->saxh.endElement         = gvg_xml_parser_end_element_handler;
  parser->saxh.characters         = gvg_xml_parser_characters_handler;
  parser->path                    = g_string_new (NULL);
  parser->content                 = g_string_new (NULL);
  parser->depth                   = 0;
  /* user funcs */
  parser->user_start_element_func = start_element_func;
  parser->user_end_element_func   = end_element_func;
  parser->user_data               = data;
  parser->user_data_notify        = data_notify;
  
  return parser;
}

void
gvg_xml_parser_free (GvgXmlParser *parser)
{
  g_return_if_fail (parser != NULL);
  
  if (parser->user_data_notify) {
    parser->user_data_notify (parser->user_data);
  }
  
  if (parser->ctxt) {
    xmlFreeDoc (parser->ctxt->myDoc);
    xmlFreeParserCtxt (parser->ctxt);
  }
  g_string_free (parser->content, TRUE);
  g_string_free (parser->path, TRUE);
  g_free (parser);
}

gboolean
gvg_xml_parser_push (GvgXmlParser  *parser,
                     const gchar   *data,
                     gsize          len,
                     gboolean       end)
{
  g_return_val_if_fail (parser != NULL, FALSE);
  g_return_val_if_fail (len <= G_MAXINT, FALSE);
  
  //~ g_debug ("got data");
  if (! parser->ctxt) {
    parser->ctxt = xmlCreatePushParserCtxt (&parser->saxh, parser,
                                            data, (gint) len, NULL);
    if (end) {
      xmlParseChunk (parser->ctxt, NULL, 0, end);
    }
  } else {
    xmlParseChunk (parser->ctxt, data, (gint) len, end);
  }
  
  if (! parser->ctxt->wellFormed) {
    g_warning ("malformed XML");
  }
  
  return parser->ctxt->wellFormed;
}
