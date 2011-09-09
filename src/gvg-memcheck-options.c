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

#include "gvg-memcheck-options.h"

#include <glib.h>
#include <glib-object.h>

#include "gvg-options.h"
#include "gvg-enum-types.h"


struct _GvgMemcheckOptionsPrivate
{
  /*guint                         show_possibly_lost : 1;*/
  guint                         show_reachable : 1;
  guint                         undef_value_erros : 1;
  guint                         track_origins : 1;
  guint                         partial_loads_ok : 1;
  guint                         workaround_gcc296_bugs : 1;
  GvgMemcheckLeakCheckMode      leak_check;
  GvgMemcheckLeakResolutionMode leak_resolution;
  guint64                       freelist_vol;
  gint8                         malloc_fill;
  gint8                         free_fill;
};


G_DEFINE_TYPE (GvgMemcheckOptions,
               gvg_memcheck_options,
               GVG_TYPE_OPTIONS)


static void   gvg_memcheck_options_get_property   (GObject    *object,
                                                   guint       prop_id,
                                                   GValue     *value,
                                                   GParamSpec *pspec);
static void   gvg_memcheck_options_set_property   (GObject      *object,
                                                   guint         prop_id,
                                                   const GValue *value,
                                                   GParamSpec   *pspec);


enum
{
  PROP_0,
  PROP_LEAK_CHECK,
  PROP_LEAK_RESOLUTION,
  PROP_FREELIST_VOL,
  PROP_MALLOC_FILL,
  PROP_FREE_FILL,
  /*PROP_SHOW_POSSIBLY_LOST,*/
  PROP_SHOW_REACHABLE,
  PROP_UNDEF_VALUE_ERRORS,
  PROP_TRACK_ORIGINS,
  PROP_PARTIAL_LOADS_OK,
  PROP_WORKAROUND_GCC296_BUGS
};


static void
gvg_memcheck_options_class_init (GvgMemcheckOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->set_property  = gvg_memcheck_options_set_property;
  object_class->get_property  = gvg_memcheck_options_get_property;
  
  g_object_class_install_property (object_class,
                                   PROP_LEAK_CHECK,
                                   g_param_spec_enum ("leak-check",
                                                      "Leak check",
                                                      "Leak check mode",
                                                      GVG_TYPE_MEMCHECK_LEAK_CHECK_MODE,
                                                      GVG_MEMCHECK_LEAK_CHECK_SUMMARY,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_LEAK_RESOLUTION,
                                   g_param_spec_enum ("leak-resolution",
                                                      "Leak resolution",
                                                      "Leak resolution mode",
                                                      GVG_TYPE_MEMCHECK_LEAK_RESOLUTION_MODE,
                                                      GVG_MEMCHECK_LEAK_RESOLUTION_HIGH,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_FREELIST_VOL,
                                   g_param_spec_uint64 ("freelist-vol",
                                                        "Free list vol",
                                                        "About of memory not to reuse when released",
                                                        0, G_MAXUINT64, 20000000,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_MALLOC_FILL,
                                   g_param_spec_char ("malloc-fill",
                                                      "Malloc fill",
                                                      "Byte with which fill allocations (-1 means none)",
                                                      -1, G_MAXINT8, -1,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_FREE_FILL,
                                   g_param_spec_char ("free-fill",
                                                      "free fill",
                                                      "Byte with which fill freed blocks (-1 means none)",
                                                      -1, G_MAXINT8, -1,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  /* this option is not supported by Valgrind < 3.6.1 */
  /*g_object_class_install_property (object_class,
                                   PROP_SHOW_POSSIBLY_LOST,
                                   g_param_spec_boolean ("show-possibly-lost",
                                                         "Show possibly lost",
                                                         "Whether to show possibly lost blocks",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));*/
  g_object_class_install_property (object_class,
                                   PROP_SHOW_REACHABLE,
                                   g_param_spec_boolean ("show-reachable",
                                                         "Show reachable",
                                                         "Whether to show reachable blocks",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_UNDEF_VALUE_ERRORS,
                                   g_param_spec_boolean ("undef-value-errors",
                                                         "Udef values error",
                                                         "Whether to show use of undefined values",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_TRACK_ORIGINS,
                                   g_param_spec_boolean ("track-origins",
                                                         "Track origins",
                                                         "Whether to track origins of undefined values",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_PARTIAL_LOADS_OK,
                                   g_param_spec_boolean ("partial-loads-ok",
                                                         "Partial loads OK",
                                                         "Whether partial loads are not reported as an error",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_WORKAROUND_GCC296_BUGS,
                                   g_param_spec_boolean ("workaround-gcc296-bugs",
                                                         "Workaround GCC 2.96 bugs",
                                                         "Whether to workaround GCC 2.96 bugs",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GvgMemcheckOptionsPrivate));
}

static void
gvg_memcheck_options_init (GvgMemcheckOptions *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_MEMCHECK_OPTIONS,
                                            GvgMemcheckOptionsPrivate);
  
  /*self->priv->show_possibly_lost      = TRUE;*/
  self->priv->show_reachable          = FALSE;
  self->priv->undef_value_erros       = TRUE;
  self->priv->track_origins           = FALSE;
  self->priv->partial_loads_ok        = FALSE;
  self->priv->workaround_gcc296_bugs  = FALSE;
  self->priv->leak_check              = GVG_MEMCHECK_LEAK_CHECK_SUMMARY;
  self->priv->leak_resolution         = GVG_MEMCHECK_LEAK_RESOLUTION_HIGH;
  self->priv->freelist_vol            = 20000000;
  self->priv->malloc_fill             = -1;
  self->priv->free_fill               = -1;
}

static void
gvg_memcheck_options_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  GvgMemcheckOptions *self = GVG_MEMCHECK_OPTIONS (object);
  
  switch (prop_id) {
    /*case PROP_SHOW_POSSIBLY_LOST:
      g_value_set_boolean (value, self->priv->show_possibly_lost != 0);
      break;*/
    
    case PROP_SHOW_REACHABLE:
      g_value_set_boolean (value, self->priv->show_reachable != 0);
      break;
    
    case PROP_UNDEF_VALUE_ERRORS:
      g_value_set_boolean (value, self->priv->undef_value_erros != 0);
      break;
    
    case PROP_TRACK_ORIGINS:
      g_value_set_boolean (value, self->priv->track_origins != 0);
      break;
    
    case PROP_PARTIAL_LOADS_OK:
      g_value_set_boolean (value, self->priv->partial_loads_ok != 0);
      break;
    
    case PROP_WORKAROUND_GCC296_BUGS:
      g_value_set_boolean (value, self->priv->workaround_gcc296_bugs != 0);
      break;
    
    case PROP_LEAK_CHECK:
      g_value_set_enum (value, self->priv->leak_check);
      break;
    
    case PROP_LEAK_RESOLUTION:
      g_value_set_enum (value, self->priv->leak_resolution);
      break;
    
    case PROP_FREELIST_VOL:
      g_value_set_uint64 (value, self->priv->freelist_vol);
      break;
    
    case PROP_MALLOC_FILL:
      g_value_set_char (value, self->priv->malloc_fill);
      break;
    
    case PROP_FREE_FILL:
      g_value_set_char (value, self->priv->free_fill);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

void
gvg_memcheck_options_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  GvgMemcheckOptions *self = GVG_MEMCHECK_OPTIONS (object);
  
  switch (prop_id) {
    /*case PROP_SHOW_POSSIBLY_LOST:
      self->priv->show_possibly_lost = g_value_get_boolean (value);
      break;*/
    
    case PROP_SHOW_REACHABLE:
      self->priv->show_reachable = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_UNDEF_VALUE_ERRORS:
      self->priv->undef_value_erros = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_TRACK_ORIGINS:
      self->priv->track_origins = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_PARTIAL_LOADS_OK:
      self->priv->partial_loads_ok = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_WORKAROUND_GCC296_BUGS:
      self->priv->workaround_gcc296_bugs = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_LEAK_CHECK:
      self->priv->leak_check = g_value_get_enum (value);
      break;
    
    case PROP_LEAK_RESOLUTION:
      self->priv->leak_resolution = g_value_get_enum (value);
      break;
    
    case PROP_FREELIST_VOL:
      self->priv->freelist_vol = g_value_get_uint64 (value);
      break;
    
    case PROP_MALLOC_FILL:
      self->priv->malloc_fill = g_value_get_char (value);
      break;
    
    case PROP_FREE_FILL:
      self->priv->free_fill = g_value_get_char (value);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
  g_object_notify_by_pspec (object, pspec);
}


GvgMemcheckOptions *
gvg_memcheck_options_new (void)
{
  return g_object_new (GVG_TYPE_MEMCHECK_OPTIONS, NULL);
}
