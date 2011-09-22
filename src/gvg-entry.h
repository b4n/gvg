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


#ifndef H_GVG_ENTRY
#define H_GVG_ENTRY

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GVG_TYPE_ENTRY            (gvg_entry_get_type ())
#define GVG_ENTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GVG_TYPE_ENTRY, GvgEntry))
#define GVG_ENTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GVG_TYPE_ENTRY, GvgEntryClass))
#define GVG_IS_ENTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GVG_TYPE_ENTRY))
#define GVG_IS_ENTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GVG_TYPE_ENTRY))
#define GVG_ENTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GVG_TYPE_ENTRY, GvgEntryClass))


typedef struct _GvgEntry        GvgEntry;
typedef struct _GvgEntryClass   GvgEntryClass;
typedef struct _GvgEntryPrivate GvgEntryPrivate;

struct _GvgEntry
{
  GtkEntry          parent;
  GvgEntryPrivate  *priv;
};

struct _GvgEntryClass
{
  GtkEntryClass parent_class;
};


GType             gvg_entry_get_type          (void) G_GNUC_CONST;
GtkWidget        *gvg_entry_new               (const gchar *description);
void              gvg_entry_set_description   (GvgEntry    *self,
                                               const gchar *description);
const gchar      *gvg_entry_get_description   (GvgEntry *self);


G_END_DECLS

#endif /* guard */
