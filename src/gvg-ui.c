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

#include "gvg-ui.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include "gvg-memcheck-store.h"
#include "gvg-memcheck-store-filter.h"
#include "gvg-memcheck-filter-bar.h"
#include "gvg-memcheck-view.h"
#include "gvg-enum-types.h"


enum
{
  KIND_COL_KIND,
  KIND_COL_LABEL,
  KIND_N_COLUMNS
};

struct _GvgUIPrivate
{
  GvgMemcheckStore *store;
  GtkTreeModel     *filter;
  GtkWidget        *view;
};


G_DEFINE_TYPE (GvgUI,
               gvg_ui,
               GTK_TYPE_VBOX)


enum
{
  PROP_0,
  PROP_MODEL
};


/*static void     gvg_ui_finalize       (GObject *object);*/


static void
gvg_ui_finalize (GObject *object)
{
  GvgUI *self = GVG_UI (object);
  
  G_OBJECT_CLASS (gvg_ui_parent_class)->finalize (object);
}

static void
gvg_ui_get_property (GObject    *object,
                     guint       prop_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
  GvgUI *self = GVG_UI (object);
  
  switch (prop_id) {
    case PROP_MODEL:
      g_value_set_object (value, self->priv->store);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_ui_set_property (GObject      *object,
                     guint         prop_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  GvgUI *self = GVG_UI (object);
  
  switch (prop_id) {
    case PROP_MODEL:
      gvg_ui_set_model (self, g_value_get_object (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static void
gvg_ui_class_init (GvgUIClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize      = gvg_ui_finalize;
  object_class->get_property  = gvg_ui_get_property;
  object_class->set_property  = gvg_ui_set_property;
  
  g_object_class_install_property (object_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model",
                                                        "Model",
                                                        "The model to display",
                                                        GVG_TYPE_MEMCHECK_STORE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private ((gpointer) klass, sizeof (GvgUIPrivate));
}

static void
filter_bar_mirror_property (GObject     *object,
                            GParamSpec  *pspec,
                            GvgUI       *self)
{
  if (self->priv->filter) {
    GValue value = {0};
    
    g_value_init (&value, pspec->value_type);
    g_object_get_property (object, pspec->name, &value);
    g_object_set_property (G_OBJECT (self->priv->filter), pspec->name, &value);
    g_value_unset (&value);
  }
}

static void
gvg_ui_view_file_activated (GvgMemcheckView  *view,
                            const gchar      *dir,
                            const gchar      *file,
                            guint             line,
                            GvgUI            *self)
{
  g_debug ("open file %s/%s:%u", dir, file, line);
}

static void
gvg_ui_view_object_activated (GvgMemcheckView  *view,
                              const gchar      *obj,
                              GvgUI            *self)
{
  g_debug ("open object %s", obj);
}

static void
gvg_ui_init (GvgUI *self)
{
  GtkWidget *hbox;
  GtkWidget *scroll;
  GtkWidget *filter_bar;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_UI, GvgUIPrivate);
  
  self->priv->store = NULL;
  self->priv->filter = NULL;
  
  /* Top bar */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (self), hbox, FALSE, TRUE, 0);
  
  filter_bar = gvg_memcheck_filter_bar_new ();
  g_signal_connect (filter_bar, "notify::kind",
                    G_CALLBACK (filter_bar_mirror_property), self);
  g_signal_connect (filter_bar, "notify::text",
                    G_CALLBACK (filter_bar_mirror_property), self);
  g_signal_connect (filter_bar, "notify::invert",
                    G_CALLBACK (filter_bar_mirror_property), self);
  gtk_box_pack_start (GTK_BOX (hbox), filter_bar, TRUE, TRUE, 0);
  
  /* The view */
  scroll = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                         "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
                         "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                         NULL);
  gtk_box_pack_start (GTK_BOX (self), scroll, TRUE, TRUE, 0);
  
  self->priv->view = g_object_new (GVG_TYPE_MEMCHECK_VIEW,
                                   "headers-visible", FALSE,
                                   NULL);
  g_signal_connect (self->priv->view, "file-activated",
                    G_CALLBACK (gvg_ui_view_file_activated), self);
  g_signal_connect (self->priv->view, "object-activated",
                    G_CALLBACK (gvg_ui_view_object_activated), self);
  gtk_container_add (GTK_CONTAINER (scroll), self->priv->view);
  
  gtk_widget_show_all (hbox);
  gtk_widget_show_all (scroll);
}


GtkWidget *
gvg_ui_new (GvgMemcheckStore *model)
{
  g_return_val_if_fail (model == NULL || GVG_IS_MEMCHECK_STORE (model), NULL);
  
  if (! model) {
    model = gvg_memcheck_store_new ();
  }
  
  return g_object_new (GVG_TYPE_UI, "model", model, NULL);
}

GvgMemcheckStore *
gvg_ui_get_model (GvgUI *self)
{
  g_return_val_if_fail (GVG_IS_UI (self), NULL);
  
  return self->priv->store;
}

void
gvg_ui_set_model (GvgUI            *self,
                  GvgMemcheckStore *model)
{
  g_return_if_fail (GVG_IS_UI (self));
  g_return_if_fail (GVG_IS_MEMCHECK_STORE (model));
  
  self->priv->store = model;
  self->priv->filter = gvg_memcheck_store_filter_new (self->priv->store, NULL);
  gtk_tree_view_set_model (GTK_TREE_VIEW (self->priv->view),
                           self->priv->filter);
  
  g_object_notify (G_OBJECT (self), "model");
}
