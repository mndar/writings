/* GStreamer
 * Copyright (C) 2024 Mandar Joshi <emailmandar@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_BASIC_OVERLAY_H_
#define _GST_BASIC_OVERLAY_H_

#include <gst/base/gstbasetransform.h>
#include <cairo.h>

G_BEGIN_DECLS

#define GST_TYPE_BASIC_OVERLAY   (gst_basic_overlay_get_type())
#define GST_BASIC_OVERLAY(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BASIC_OVERLAY,GstBasicOverlay))
#define GST_BASIC_OVERLAY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BASIC_OVERLAY,GstBasicOverlayClass))
#define GST_IS_BASIC_OVERLAY(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BASIC_OVERLAY))
#define GST_IS_BASIC_OVERLAY_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BASIC_OVERLAY))

typedef struct _GstBasicOverlay GstBasicOverlay;
typedef struct _GstBasicOverlayClass GstBasicOverlayClass;

typedef struct {
  gchar *boxname, *boxlabel;
  guint x, y, width, height;
}BoxInfo;

struct _GstBasicOverlay
{
  GstBaseTransform base_basicoverlay;

  GstVideoInfo info;
  GList *boxlist;
  cairo_t *cr;

  guint video_width, video_height;
};

struct _GstBasicOverlayClass
{
  GstBaseTransformClass base_basicoverlay_class;
};

GType gst_basic_overlay_get_type (void);

G_END_DECLS

#endif
