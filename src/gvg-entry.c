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
 * An entry with a descriptive text shown when there is no user text
 */

#include "gvg-entry.h"

#include <gtk/gtk.h>


struct _GvgEntryPrivate
{
  PangoLayout  *desc_layout;
  gchar        *desc_text;
};


G_DEFINE_TYPE (GvgEntry,
               gvg_entry,
               GTK_TYPE_ENTRY)


enum
{
  PROP_0,
  PROP_DESCRIPTION
};


static void
gvg_entry_finalize (GObject *object)
{
  GvgEntry *self = GVG_ENTRY (object);
  
  g_free (self->priv->desc_text);
  if (self->priv->desc_layout) {
    g_object_unref (self->priv->desc_layout);
  }
  
  G_OBJECT_CLASS (gvg_entry_parent_class)->finalize (object);
}

static void
gvg_entry_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  GvgEntry *self = GVG_ENTRY (object);
  
  switch (prop_id) {
    case PROP_DESCRIPTION:
      g_value_set_string (value, self->priv->desc_text);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_entry_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  GvgEntry *self = GVG_ENTRY (object);
  
  switch (prop_id) {
    case PROP_DESCRIPTION:
      gvg_entry_set_description (self, g_value_get_string (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

/* tries to replicate _gtk_entry_get_borders() in some way */
static void
get_layout_offsets (GvgEntry *self,
                    gint     *x,
                    gint     *y)
{
  gtk_entry_get_layout_offsets (GTK_ENTRY (self), x, y);
  
  if (gtk_entry_get_has_frame (GTK_ENTRY (self))) {
    GtkStyle *style = gtk_widget_get_style (GTK_WIDGET (self));
    
    *x -= style->xthickness;
    *y -= style->ythickness;
  }
}

static gboolean
gvg_entry_expose_event (GtkWidget      *widget,
                        GdkEventExpose *event)
{
  GvgEntry *self = GVG_ENTRY (widget);
  gboolean  ret;
  
  ret = GTK_WIDGET_CLASS (gvg_entry_parent_class)->expose_event (widget, event);
  
  if (self->priv->desc_text && *self->priv->desc_text &&
      gtk_widget_is_drawable (widget) &&
      ! gtk_widget_has_focus (widget) &&
      event->window == gtk_entry_get_text_window (GTK_ENTRY (widget))) {
    const gchar  *text;
    
    text = gtk_entry_get_text (GTK_ENTRY (widget));
    if (! text || ! *text) {
      /* setup layout if necessary */
      if (! self->priv->desc_layout) {
        PangoLayout *layout = gtk_entry_get_layout (GTK_ENTRY (widget));
        
        if (layout) {
          self->priv->desc_layout = pango_layout_copy (layout);
          pango_layout_set_text (self->priv->desc_layout,
                                 self->priv->desc_text, -1);
        }
      }
      
      if (self->priv->desc_layout) {
        GtkStyle *style = gtk_widget_get_style (widget);
        cairo_t  *cr;
        gint      x;
        gint      y;
        
        get_layout_offsets (self, &x, &y);
        cr = gdk_cairo_create (event->window);
        cairo_move_to (cr, x, y);
        gdk_cairo_set_source_color (cr, &style->text[GTK_STATE_INSENSITIVE]);
        pango_cairo_show_layout (cr, self->priv->desc_layout);
        cairo_destroy (cr);
      }
    }
  }
  
  return ret;
}

static void
gvg_entry_class_init (GvgEntryClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->finalize      = gvg_entry_finalize;
  object_class->get_property  = gvg_entry_get_property;
  object_class->set_property  = gvg_entry_set_property;
  
  widget_class->expose_event  = gvg_entry_expose_event;
  
  g_object_class_install_property (object_class,
                                   PROP_DESCRIPTION,
                                   g_param_spec_string ("description",
                                                        "Description",
                                                        "The description to show in background "
                                                        "when no user text is set",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private ((gpointer) klass, sizeof (GvgEntryPrivate));
}

static void
gvg_entry_init (GvgEntry *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GVG_TYPE_ENTRY,
                                            GvgEntryPrivate);
  
  self->priv->desc_layout = NULL;
  self->priv->desc_text   = NULL;
}


GtkWidget *
gvg_entry_new (const gchar *description)
{
  return g_object_new (GVG_TYPE_ENTRY, "description", description, NULL);
}

void
gvg_entry_set_description (GvgEntry    *self,
                           const gchar *description)
{
  g_return_if_fail (GTK_IS_ENTRY (self));
  
  g_free (self->priv->desc_text);
  self->priv->desc_text = g_strdup (description);
  /* update layout if needed */
  if (self->priv->desc_layout) {
    pango_layout_set_text (self->priv->desc_layout, self->priv->desc_text, -1);
  }
}

const gchar *
gvg_entry_get_description (GvgEntry *self)
{
  g_return_val_if_fail (GTK_IS_ENTRY (self), NULL);
  
  return self->priv->desc_text;
}
