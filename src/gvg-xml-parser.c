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
#include <glib-object.h>
#include <libxml/parser.h>
#include <string.h>


struct _GvgXmlParserPrivate {
  xmlSAXHandler           saxh;
  xmlParserCtxtPtr        ctxt;
  
  GString                *content;
  GString                *path;
  guint                   depth;
};


static void     gvg_xml_parser_finalize               (GObject *object);
static void     gvg_xml_parser_characters_handler     (void           *data,
                                                       const xmlChar  *chs,
                                                       int             len);
static void     gvg_xml_parser_end_element_handler    (void          *data,
                                                       const xmlChar *name);
static void     gvg_xml_parser_start_element_hanlder  (void            *data,
                                                       const xmlChar   *name,
                                                       const xmlChar  **atts);


G_DEFINE_ABSTRACT_TYPE (GvgXmlParser,
                        gvg_xml_parser,
                        G_TYPE_OBJECT)


static void
gvg_xml_parser_class_init (GvgXmlParserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gvg_xml_parser_finalize;

  g_type_class_add_private (klass, sizeof (GvgXmlParserPrivate));
}

static void
gvg_xml_parser_init (GvgXmlParser *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_XML_PARSER,
                                            GvgXmlParserPrivate);
  
  self->priv->ctxt              = NULL;
  memset (&self->priv->saxh, 0, sizeof self->priv->saxh);
  self->priv->saxh.startElement = gvg_xml_parser_start_element_hanlder;
  self->priv->saxh.endElement   = gvg_xml_parser_end_element_handler;
  self->priv->saxh.characters   = gvg_xml_parser_characters_handler;
  self->priv->path              = g_string_new (NULL);
  self->priv->content           = g_string_new (NULL);
  self->priv->depth             = 0;
}

static void
gvg_xml_parser_finalize (GObject *object)
{
  GvgXmlParser *self = GVG_XML_PARSER (object);
  
  if (self->priv->ctxt) {
    xmlFreeDoc (self->priv->ctxt->myDoc);
    xmlFreeParserCtxt (self->priv->ctxt);
  }
  g_string_free (self->priv->content, TRUE);
  g_string_free (self->priv->path, TRUE);
  
  G_OBJECT_CLASS (gvg_xml_parser_parent_class)->finalize (object);
}

static void
parser_path_push (GvgXmlParser *self,
                  const gchar  *element)
{
  g_string_append_c (self->priv->path, '/');
  g_string_append (self->priv->path, element);
  self->priv->depth ++;
}

static void
parser_path_pop (GvgXmlParser *self)
{
  gsize i;
  
  g_return_if_fail (self->priv->path->len > 0);
  
  self->priv->depth--;
  for (i = self->priv->path->len; i-- > 0; ) {
    if (self->priv->path->str[i] == '/') {
      g_string_truncate (self->priv->path, i);
      break;
    }
  }
}

static void
gvg_xml_parser_start_element_hanlder (void            *data,
                                      const xmlChar   *name,
                                      const xmlChar  **atts)
{
  GvgXmlParser       *self  = data;
  GvgXmlParserClass  *klass = GVG_XML_PARSER_GET_CLASS (self);
  
  parser_path_push (self, (const gchar *) name);
  //~ g_debug ("start element %s", self->priv->path->str);
  
  if (klass->element_start) {
    klass->element_start (self,
                          (const gchar *) name,
                          (const gchar **) atts,
                          self->priv->path->str);
  }
  g_string_truncate (self->priv->content, 0);
}

static void
gvg_xml_parser_end_element_handler (void          *data,
                                    const xmlChar *name)
{
  GvgXmlParser       *self  = data;
  GvgXmlParserClass  *klass = GVG_XML_PARSER_GET_CLASS (self);
  
  //~ g_debug ("end element %s", self->priv->path->str);
  if (klass->element_end) {
    klass->element_end (self,
                        (const gchar *) name,
                        self->priv->content->str,
                        self->priv->path->str);
  }
  parser_path_pop (self);
  
  g_string_truncate (self->priv->content, 0);
}

static void
gvg_xml_parser_characters_handler (void           *data,
                                   const xmlChar  *chs,
                                   int             len)
{
  GvgXmlParser *self = data;
  
  g_string_append_len (self->priv->content, (const gchar *) chs, len);
}

gboolean
gvg_xml_parser_push (GvgXmlParser  *self,
                     const gchar   *data,
                     gsize          len,
                     gboolean       end)
{
  g_return_val_if_fail (GVG_IS_XML_PARSER (self), FALSE);
  g_return_val_if_fail (len <= G_MAXINT, FALSE);
  
  //~ g_debug ("got data");
  /*fwrite (data, 1, len, stderr);*/
  if (! self->priv->ctxt) {
    self->priv->ctxt = xmlCreatePushParserCtxt (&self->priv->saxh, self,
                                                data, (gint) len, NULL);
    if (end) {
      xmlParseChunk (self->priv->ctxt, NULL, 0, end);
    }
  } else {
    xmlParseChunk (self->priv->ctxt, data, (gint) len, end);
  }
  
  if (! self->priv->ctxt->wellFormed) {
    g_warning ("malformed XML");
  }
  
  return self->priv->ctxt->wellFormed;
}
