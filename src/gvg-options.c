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

#include "gvg-options.h"

#include <glib.h>
#include <glib-object.h>

#include "gvg-args-builder.h"


struct _GvgOptionsPrivate
{
  guint       demangle : 1;
  guint       error_limit : 1;
  guint       show_below_main : 1;
  guint       track_fds : 1;
  guint       time_stamp : 1;
  
  guint       num_callers;
  guint64     max_stackframe;
  gint64      main_stacksize;
};


G_DEFINE_TYPE (GvgOptions,
               gvg_options,
               G_TYPE_OBJECT)


static void   gvg_options_get_property    (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec);
static void   gvg_options_set_property    (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec);


enum
{
  PROP_0,
  PROP_DEMANGLE,
  PROP_ERROR_LIMIT,
  PROP_SHOW_BELOW_MAIN,
  PROP_TRACK_FDS,
  PROP_TIME_STAMP,
  PROP_NUM_CALLERS,
  PROP_MAX_STACKFRAME,
  PROP_MAIN_STACKSIZE
};


static void
gvg_options_class_init (GvgOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->set_property  = gvg_options_set_property;
  object_class->get_property  = gvg_options_get_property;
  
  g_object_class_install_property (object_class,
                                   PROP_DEMANGLE,
                                   g_param_spec_boolean ("demangle",
                                                         "Demangle",
                                                         "Whether to demangle C++ names",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_ERROR_LIMIT,
                                   g_param_spec_boolean ("error-limit",
                                                         "Error limit",
                                                         "Whether to stop reporting errors "
                                                         "when too many was already reported",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_SHOW_BELOW_MAIN,
                                   g_param_spec_boolean ("show-below-main",
                                                         "Show below main",
                                                         "Whether to show stack trace for "
                                                         "functions below main",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_TRACK_FDS,
                                   g_param_spec_boolean ("track-fds",
                                                         "Track FDs",
                                                         "Whether to track file descriptors",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
 /* FIXME: this option should not be a configurable option here but rather after parsing... */
  g_object_class_install_property (object_class,
                                   PROP_TIME_STAMP,
                                   g_param_spec_boolean ("time-stamp",
                                                         "Time stamp",
                                                         "Whether to add time stamp information",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_NUM_CALLERS,
                                   g_param_spec_uint ("num-callers",
                                                      "Num callers",
                                                      "The maximum entries to show in stack traces",
                                                      0, 50, 12,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_MAX_STACKFRAME,
                                   g_param_spec_uint64 ("max-stackframe",
                                                        "Max stack frame",
                                                        "The maximum size of a stack frame",
                                                        0, G_MAXUINT64, 2000000,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  /* -1 means default */
  g_object_class_install_property (object_class,
                                   PROP_MAIN_STACKSIZE,
                                   g_param_spec_int64 ("main-stacksize",
                                                       "Main stack size",
                                                       "The size of the main thread's stack (-1 uses ulimit value)",
                                                       -1, G_MAXINT64, -1,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GvgOptionsPrivate));
}

static void
gvg_options_init (GvgOptions *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GVG_TYPE_OPTIONS,
                                            GvgOptionsPrivate);
  
  self->priv->demangle        = TRUE;
  self->priv->error_limit     = TRUE;
  self->priv->show_below_main = FALSE;
  self->priv->track_fds       = FALSE;
  self->priv->time_stamp      = FALSE;
  self->priv->num_callers     = 12;
  self->priv->max_stackframe  = 2000000;
  self->priv->main_stacksize  = -1;
}

static void
gvg_options_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GvgOptions *self = GVG_OPTIONS (object);
  
  switch (prop_id) {
    case PROP_DEMANGLE:
      g_value_set_boolean (value, self->priv->demangle != 0);
      break;
    
    case PROP_ERROR_LIMIT:
      g_value_set_boolean (value, self->priv->error_limit != 0);
      break;
    
    case PROP_SHOW_BELOW_MAIN:
      g_value_set_boolean (value, self->priv->show_below_main != 0);
      break;
    
    case PROP_TRACK_FDS:
      g_value_set_boolean (value, self->priv->track_fds != 0);
      break;
    
    case PROP_TIME_STAMP:
      g_value_set_boolean (value, self->priv->time_stamp != 0);
      break;
    
    case PROP_NUM_CALLERS:
      g_value_set_uint (value, self->priv->num_callers);
      break;
    
    case PROP_MAX_STACKFRAME:
      g_value_set_uint64 (value, self->priv->max_stackframe);
      break;
    
    case PROP_MAIN_STACKSIZE:
      g_value_set_int64 (value, self->priv->main_stacksize);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
gvg_options_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GvgOptions *self = GVG_OPTIONS (object);
  
  switch (prop_id) {
    case PROP_DEMANGLE:
      self->priv->demangle = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_ERROR_LIMIT:
      self->priv->error_limit = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_SHOW_BELOW_MAIN:
      self->priv->show_below_main = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_TRACK_FDS:
      self->priv->track_fds = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_TIME_STAMP:
      self->priv->time_stamp = g_value_get_boolean (value) != FALSE;
      break;
    
    case PROP_NUM_CALLERS:
      self->priv->num_callers = g_value_get_uint (value);
      break;
    
    case PROP_MAX_STACKFRAME:
      self->priv->max_stackframe = g_value_get_uint64 (value);
      break;
    
    case PROP_MAIN_STACKSIZE:
      self->priv->main_stacksize = g_value_get_int64 (value);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
  g_object_notify_by_pspec (object, pspec);
}


GvgOptions *
gvg_options_new (void)
{
  return g_object_new (GVG_TYPE_OPTIONS, NULL);
}

static void
add_arg (GvgArgsBuilder  *builder,
         const gchar     *arg,
         const GValue    *value)
{
  switch (G_TYPE_FUNDAMENTAL (G_VALUE_TYPE (value))) {
    case G_TYPE_BOOLEAN:
      gvg_args_builder_add_bool (builder, arg, g_value_get_boolean (value));
      break;
    
    case G_TYPE_ENUM: {
      GEnumClass *enum_class;
      GEnumValue *enum_value;
      gint        val = g_value_get_enum (value);
      
      enum_class = g_type_class_ref (G_VALUE_TYPE (value));
      enum_value = g_enum_get_value (enum_class, val);
      if (! enum_value) {
        g_critical ("value \"%d\" is not valid for setting \"%s\"", val, arg);
      } else {
        gvg_args_builder_add_string (builder, arg, enum_value->value_nick);
      }
      g_type_class_unref (enum_class);
      break;
    }
    
    /* FIXME: we assume < 0 mean don't pass the arg, is it always true?
     * FIXME: we output 0x... instead of the character... */
    case G_TYPE_CHAR: {
      gint8 v = g_value_get_char (value);
      
      if (v >= 0) {
        gvg_args_builder_add_arg (builder, arg, "%#x", (guint8) v);
      }
      break;
    }
    
    /* FIXME: we assume < 0 mean don't pass the arg, is it always true? */
    case G_TYPE_INT: {
      gint v = g_value_get_int (value);
      
      if (v >= 0) {
        gvg_args_builder_add_arg (builder, arg, "%d", v);
      }
      break;
    }
    
    case G_TYPE_UINT:
      gvg_args_builder_add_arg (builder, arg, "%u", g_value_get_uint (value));
      break;
    
    /* FIXME: we assume < 0 mean don't pass the arg, is it always true? */
    case G_TYPE_INT64: {
      gint64 v = g_value_get_int64 (value);
      
      if (v >= 0) {
        gvg_args_builder_add_arg (builder, arg, "%"G_GINT64_FORMAT, v);
      }
      break;
    }
    
    case G_TYPE_UINT64:
      gvg_args_builder_add_arg (builder, arg, "%"G_GUINT64_FORMAT,
                                g_value_get_uint64 (value));
      break;
    
    case G_TYPE_STRING:
      gvg_args_builder_add_string (builder, arg, g_value_get_string (value));
      break;
    
    default:
      g_critical ("unsupported setting type \"%s\" for setting \"%s\"",
                   G_VALUE_TYPE_NAME (value), arg);
  }
}

void
gvg_options_to_args (GvgOptions     *self,
                     GvgArgsBuilder *builder)
{
  guint         i;
  guint         n_props;
  GParamSpec  **props;
  GObjectClass *object_class = G_OBJECT_GET_CLASS (self);
  
  g_return_if_fail (G_IS_OBJECT (self));
  g_return_if_fail (builder != NULL);
  
  props = g_object_class_list_properties (object_class, &n_props);
  for (i = 0; i < n_props; i++) {
    GValue value = {0};
    
    g_value_init (&value, props[i]->value_type);
    g_object_get_property (G_OBJECT (self), props[i]->name, &value);
    add_arg (builder, props[i]->name, &value);
    g_value_unset (&value);
  }
  g_free (props);
}
