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

#ifndef H_GVG_MEMCHECK
#define H_GVG_MEMCHECK

#include <glib.h>
#include <gtk/gtk.h>

#include "gvg.h"
#include "gvg-args-builder.h"

G_BEGIN_DECLS


typedef enum {
  GVG_MEMCHECK_LEAK_CHECK_NO,
  GVG_MEMCHECK_LEAK_CHECK_SUMMARY,
  GVG_MEMCHECK_LEAK_CHECK_YES,
  GVG_MEMCHECK_LEAK_CHECK_FULL
} GvgMemcheckLeakCheckMode;

typedef enum {
  GVG_MEMCHECK_LEAK_RESOLUTION_LOW,
  GVG_MEMCHECK_LEAK_RESOLUTION_MEDIUM,
  GVG_MEMCHECK_LEAK_RESOLUTION_HIGH
} GvgMemcheckLeakResolutionMode;

typedef enum {
  GVG_ERROR_KIND_UNINIT_VALUE,
  GVG_ERROR_KIND_UNINIT_CONDITION,
  GVG_ERROR_KIND_INVALID_READ,
  GVG_ERROR_KIND_LEAK_POSSIBLY_LOST,
  GVG_ERROR_KIND_LEAK_DEFINITELY_LOST
  /* ... */
} GvgMemcheckErrorKind;

typedef struct _GvgMemcheckOptions  GvgMemcheckOptions;

struct _GvgMemcheckOptions {
  GvgOptions parent;
  
  GvgMemcheckLeakCheckMode      leak_check;
  GvgMemcheckLeakResolutionMode leak_resolution;
  gssize                        freelist_vol; /* -1 means default */
  gint                          malloc_fill; /* -1 means none */
  gint                          free_fill; /* -1 means none */
  guint                         show_possibly_lost      : 1;
  guint                         show_reachable          : 1;
  guint                         undef_value_errors      : 1;
  guint                         track_origins           : 1;
  guint                         partial_loads_ok        : 1;
  guint                         workaround_gcc296_bugs  : 1;
  /* gsize ingore_ranges[] */
};

#define GVG_MEMCHECK_OPTIONS_INIT {                           \
    GVG_OPTIONS_INIT, /* parent GvgOptions */                 \
                                                              \
    GVG_MEMCHECK_LEAK_CHECK_SUMMARY,    /* leak check */      \
    GVG_MEMCHECK_LEAK_RESOLUTION_HIGH,  /* leak resolution */ \
    -1, /* freelist vol */                                    \
    -1, /* malloc fill */                                     \
    -1, /* free fill */                                       \
    TRUE, /* show possibly lost */                            \
    FALSE, /* show reachable */                               \
    TRUE, /* undef value errors */                            \
    FALSE, /* track origins */                                \
    FALSE, /* partial loads ok */                             \
    FALSE, /* workaround gcc296 bugs */                       \
  }


const gchar          *gvg_memcheck_leak_check_mode_to_string      (GvgMemcheckLeakCheckMode mode);
const gchar          *gvg_memcheck_leak_resolution_mode_to_string (GvgMemcheckLeakResolutionMode mode);
const gchar          *gvg_memcheck_error_kind_to_string           (GvgMemcheckErrorKind kind);
GvgMemcheckErrorKind  gvg_memcheck_error_kind_from_string         (const gchar *str);
void                  gvg_memcheck_add_options                    (GvgArgsBuilder            *args,
                                                                   const GvgMemcheckOptions  *opts);
gboolean              gvg_memcheck                                (const gchar              **program_argv,
                                                                   const GvgMemcheckOptions  *options,
                                                                   GtkTreeStore              *store,
                                                                   GError                   **error);


G_END_DECLS

#endif /* guard */
