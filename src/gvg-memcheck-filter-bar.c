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

#include "gvg-memcheck-filter-bar.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-store.h"
#include "gvg-entry.h"
#include "gvg-enum-types.h"


enum
{
  KIND_COL_KIND,
  KIND_COL_LABEL,
  KIND_N_COLUMNS
};

struct _GvgMemcheckFilterBarPrivate
{
  GtkListStore     *kind_store;
  GtkWidget        *kind_combo;
  GtkWidget        *filter_entry;
  GtkWidget        *filter_invert;
};


G_DEFINE_TYPE (GvgMemcheckFilterBar,
               gvg_memcheck_filter_bar,
               GTK_TYPE_HBOX)


enum
{
  PROP_0,
  PROP_KIND,
  PROP_TEXT,
  PROP_INVERT
};


/*static void     gvg_memcheck_filter_bar_finalize       (GObject *object);*/


static void
gvg_memcheck_filter_bar_finalize (GObject *object)
{
  GvgMemcheckFilterBar *self = GVG_MEMCHECK_FILTER_BAR (object);
  
  
  G_OBJECT_CLASS (gvg_memcheck_filter_bar_parent_class)->finalize (object);
}

static void
gvg_memcheck_filter_bar_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  GvgMemcheckFilterBar *self = GVG_MEMCHECK_FILTER_BAR (object);
  
  switch (prop_id) {
    case PROP_KIND:
      g_value_set_enum (value, gvg_memcheck_filter_bar_get_kind (self));
      break;
    
    case PROP_TEXT:
      g_value_set_string (value, gvg_memcheck_filter_bar_get_text (self));
      break;
    
    case PROP_INVERT:
      g_value_set_boolean (value, gvg_memcheck_filter_bar_get_invert (self));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_memcheck_filter_bar_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  GvgMemcheckFilterBar *self = GVG_MEMCHECK_FILTER_BAR (object);
  
  switch (prop_id) {
    case PROP_KIND:
      gvg_memcheck_filter_bar_set_kind (self, g_value_get_enum (value));
      break;
    
    case PROP_TEXT:
      gvg_memcheck_filter_bar_set_text (self, g_value_get_string (value));
      break;
    
    case PROP_INVERT:
      gvg_memcheck_filter_bar_set_invert (self, g_value_get_boolean (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static void
gvg_memcheck_filter_bar_class_init (GvgMemcheckFilterBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize      = gvg_memcheck_filter_bar_finalize;
  object_class->get_property  = gvg_memcheck_filter_bar_get_property;
  object_class->set_property  = gvg_memcheck_filter_bar_set_property;
  
  g_object_class_install_property (object_class,
                                   PROP_KIND,
                                   g_param_spec_enum ("kind",
                                                      "Kind",
                                                      "The error kind to show",
                                                      GVG_TYPE_MEMCHECK_ERROR_KIND,
                                                      GVG_MEMCHECK_ERROR_KIND_ANY,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "Text",
                                                        "A text the error should contain",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_INVERT,
                                   g_param_spec_boolean ("invert",
                                                         "Invert",
                                                         "Whether to invert the match",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private ((gpointer) klass,
                            sizeof (GvgMemcheckFilterBarPrivate));
}

static GtkListStore *
gvg_memcheck_filter_bar_create_kind_store (void)
{
  GtkListStore *store;
  GEnumClass   *enum_class;
  guint         i;
  
  store = gtk_list_store_new (KIND_N_COLUMNS,
                              GVG_TYPE_MEMCHECK_ERROR_KIND,
                              G_TYPE_STRING);
  enum_class = g_type_class_ref (GVG_TYPE_MEMCHECK_ERROR_KIND);
  for (i = 0; i < enum_class->n_values; i++) {
    GtkTreeIter iter;
    
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        0, enum_class->values[i].value,
                        1, enum_class->values[i].value_nick,
                        -1);
  }
  g_type_class_unref (enum_class);
  
  return store;
}

static GtkWidget *
gvg_memcheck_filter_bar_create_kind_combo (GtkListStore *store)
{
  GtkWidget        *combo;
  GtkCellRenderer  *cell;
  
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), cell,
                                 "text", KIND_COL_LABEL);
  
  /* select first row by default */
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  
  return combo;
}

static void
gvg_memcheck_filter_bar_kind_combo_changed_hanlder (GtkComboBox          *combo,
                                                    GvgMemcheckFilterBar *self)
{
  g_object_notify (G_OBJECT (self), "kind");
}

static void
gvg_memcheck_filter_bar_filter_entry_notify_text_hanlder (GObject              *object,
                                                          GParamSpec           *pspec,
                                                          GvgMemcheckFilterBar *self)
{
  g_object_notify (G_OBJECT (self), "text");
}

static void
gvg_memcheck_filter_bar_filter_invert_toggled_hanlder (GtkToggleButton       *button,
                                                       GvgMemcheckFilterBar  *self)
{
  g_object_notify (G_OBJECT (self), "invert");
}

static void
gvg_memcheck_filter_bar_init (GvgMemcheckFilterBar *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GVG_TYPE_MEMCHECK_FILTER_BAR,
                                            GvgMemcheckFilterBarPrivate);
  
  /* kind filter */
  /* TODO: make unsensitive types that don't exist actually in the filtered
   *       view -- this gonna be tricky */
  self->priv->kind_store = gvg_memcheck_filter_bar_create_kind_store ();
  self->priv->kind_combo = gvg_memcheck_filter_bar_create_kind_combo (self->priv->kind_store);
  gtk_widget_set_tooltip_text (self->priv->kind_combo,
                               _("The error kind to show"));
  g_signal_connect (self->priv->kind_combo, "changed",
                    G_CALLBACK (gvg_memcheck_filter_bar_kind_combo_changed_hanlder),
                    self);
  gtk_box_pack_start (GTK_BOX (self), self->priv->kind_combo, FALSE, TRUE, 0);
  
  /* filter entry */
  self->priv->filter_entry = gvg_entry_new (_("Filter"));
  gtk_widget_set_tooltip_text (self->priv->filter_entry,
                               _("A text the error should contain"));
  g_signal_connect (self->priv->filter_entry, "notify::text",
                    G_CALLBACK (gvg_memcheck_filter_bar_filter_entry_notify_text_hanlder),
                    self);
  gtk_box_pack_start (GTK_BOX (self), self->priv->filter_entry, TRUE, TRUE, 0);
  /* reverse match */
  self->priv->filter_invert = gtk_check_button_new_with_label (_("Reverse match"));
  g_signal_connect (self->priv->filter_invert, "toggled",
                    G_CALLBACK (gvg_memcheck_filter_bar_filter_invert_toggled_hanlder),
                    self);
  gtk_box_pack_start (GTK_BOX (self), self->priv->filter_invert,
                      FALSE, TRUE, 0);
  
  gtk_widget_show (self->priv->kind_combo);
  gtk_widget_show (self->priv->filter_entry);
  gtk_widget_show (self->priv->filter_invert);
}


GtkWidget *
gvg_memcheck_filter_bar_new (void)
{
  return g_object_new (GVG_TYPE_MEMCHECK_FILTER_BAR, NULL);
}

GvgMemcheckErrorKind
gvg_memcheck_filter_bar_get_kind (GvgMemcheckFilterBar *self)
{
  GvgMemcheckErrorKind  kind = GVG_MEMCHECK_ERROR_KIND_ANY;
  GtkTreeIter           iter;
  
  g_return_val_if_fail (GVG_IS_MEMCHECK_FILTER_BAR (self), kind);
  
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (self->priv->kind_combo),
                                     &iter)) {
    gtk_tree_model_get (GTK_TREE_MODEL (self->priv->kind_store), &iter,
                        KIND_COL_KIND, &kind, -1);
  }
  
  return kind;
}

void
gvg_memcheck_filter_bar_set_kind (GvgMemcheckFilterBar *self,
                                  GvgMemcheckErrorKind  kind)
{
  g_return_if_fail (GVG_IS_MEMCHECK_FILTER_BAR (self));
  g_return_if_fail (kind >= 0 &&
                    kind <= GVG_MEMCHECK_ERROR_KIND_LEAK_STILL_REACHABLE);
  
  /* the index should always match the kind it holds, so this should work */
  gtk_combo_box_set_active (GTK_COMBO_BOX (self->priv->kind_combo), kind);
  /* no need to notify since we do so in a changed handler anyway */
}

const gchar *
gvg_memcheck_filter_bar_get_text (GvgMemcheckFilterBar *self)
{
  g_return_val_if_fail (GVG_IS_MEMCHECK_FILTER_BAR (self), NULL);
  
  return gtk_entry_get_text (GTK_ENTRY (self->priv->filter_entry));
}

void
gvg_memcheck_filter_bar_set_text (GvgMemcheckFilterBar *self,
                                  const gchar          *text)
{
  g_return_if_fail (GVG_IS_MEMCHECK_FILTER_BAR (self));
  
  gtk_entry_set_text (GTK_ENTRY (self->priv->filter_entry), text);
  /* no need to notify since we do so in a changed handler anyway */
}

gboolean
gvg_memcheck_filter_bar_get_invert (GvgMemcheckFilterBar *self)
{
  g_return_val_if_fail (GVG_IS_MEMCHECK_FILTER_BAR (self), FALSE);
  
  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self->priv->filter_invert));
}

void
gvg_memcheck_filter_bar_set_invert (GvgMemcheckFilterBar *self,
                                    gboolean              invert)
{
  g_return_if_fail (GVG_IS_MEMCHECK_FILTER_BAR (self));
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->priv->filter_invert),
                                invert);
  /* no need to notify since we do so in a changed handler anyway */
}
