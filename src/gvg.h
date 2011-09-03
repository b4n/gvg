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

#ifndef H_GVG
#define H_GVG

#include <glib.h>

#include "gvg-xml-parser.h"
#include "gvg-args-builder.h"

G_BEGIN_DECLS


typedef struct _GvgOptions  GvgOptions;

struct _GvgOptions {
  guint   demangle        : 1;
  guint   error_limit     : 1;
  guint   show_below_main : 1;
  guint   track_fds       : 1;
  guint   time_stamp      : 1;
  
  guint   num_callers;
  gssize  max_stackframe; /* -1 means default */
  gssize  main_stacksize; /* -1 means default */
};

/** default initializer for GvgOptions */
#define GVG_OPTIONS_INIT {        \
    TRUE,   /* demangle */        \
    TRUE,   /* error limit */     \
    FALSE,  /* show below main */ \
    FALSE,  /* track fds */       \
    FALSE,  /* time stamp */      \
    12,     /* num callers */     \
    -1,     /* max stackframe */  \
    -1,     /* main-stacksize */  \
  }


typedef void    (*GvgGetArgsFunc)       (GvgArgsBuilder  *bld,
                                         gpointer         data);

void          gvg_add_options       (GvgArgsBuilder        *args,
                                     const GvgOptions      *opts);
gboolean      gvg                   (const gchar          **program_argv,
                                     GvgGetArgsFunc         get_args,
                                     gpointer               get_args_data,
                                     GvgXmlStartElementFunc xml_start_tag_func,
                                     GvgXmlEndElementFunc   xml_end_tag_func,
                                     gpointer               xml_data,
                                     GDestroyNotify         xml_data_destroy,
                                     GError               **error);


G_END_DECLS

#endif /* guard */
