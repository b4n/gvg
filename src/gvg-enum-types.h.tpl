/*** BEGIN file-header ***/
#ifndef H_GVG_ENUM_TYPES
#define H_GVG_ENUM_TYPES

#include <glib-object.h>

/* include all headers that may introduce new enums */
#include "gvg.h"
#include "gvg-memcheck.h"
#include "gvg-memcheck-parser.h"
#include "gvg-memcheck-store.h"
#include "gvg-xml-parser.h"

G_BEGIN_DECLS
/*** END file-header ***/

/*** BEGIN file-production ***/

/* enumerations from "@filename@" */
/*** END file-production ***/

/*** BEGIN value-header ***/
G_GNUC_INTERNAL
GType @enum_name@_get_type (void) G_GNUC_CONST;
#define @ENUMPREFIX@_TYPE_@ENUMSHORT@ (@enum_name@_get_type ())
/*** END value-header ***/

/*** BEGIN file-tail ***/
G_END_DECLS

#endif /* guard */
/*** END file-tail ***/
