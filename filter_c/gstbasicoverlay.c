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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstbasicoverlay
 *
 * The basicoverlay element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! basicoverlay ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/video/video.h>
#include "gstbasicoverlay.h"

GST_DEBUG_CATEGORY_STATIC (gst_basic_overlay_debug_category);
#define GST_CAT_DEFAULT gst_basic_overlay_debug_category

/* prototypes */


static void gst_basic_overlay_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_basic_overlay_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_basic_overlay_dispose (GObject * object);
static gboolean gst_basic_overlay_set_caps (GstBaseTransform * trans, GstCaps * in_caps,
    GstCaps * out_caps);
static GstFlowReturn gst_basic_overlay_transform_ip (GstBaseTransform * trans,
    GstBuffer * buf);

enum
{
  PROP_0
};

enum 
{
  SIGNAL_ADD_BOX,
  SIGNAL_REMOVE_BOX,
  N_SIGNALS
};

static guint gst_basic_overlay_signals[N_SIGNALS];
/* pad templates */

static GstStaticPadTemplate gst_basic_overlay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE("{ BGRx, BGRA, RGB16 }"))
    );

static GstStaticPadTemplate gst_basic_overlay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE("{ BGRx, BGRA, RGB16 }"))
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstBasicOverlay, gst_basic_overlay, GST_TYPE_BASE_TRANSFORM,
  GST_DEBUG_CATEGORY_INIT (gst_basic_overlay_debug_category, "basicoverlay", 0,
  "debug category for basicoverlay element"));

static void
gst_basic_overlay_class_init (GstBasicOverlayClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_basic_overlay_src_template);
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_basic_overlay_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "Basic Overlay", "Generic", "Basic Overlay",
      "Mandar Joshi <emailmandar@gmail.com>");

  gobject_class->set_property = gst_basic_overlay_set_property;
  gobject_class->get_property = gst_basic_overlay_get_property;
  gobject_class->dispose = gst_basic_overlay_dispose;
  base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_basic_overlay_set_caps);
  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_basic_overlay_transform_ip);

  gst_basic_overlay_signals[SIGNAL_ADD_BOX] =
      g_signal_new ("addbox",
      G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
      G_TYPE_NONE, 6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);  
  gst_basic_overlay_signals[SIGNAL_REMOVE_BOX] =
      g_signal_new ("removebox",
      G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
      G_TYPE_NONE, 1, G_TYPE_STRING);  
}

static void 
gst_basic_overlay_add_box (GstBasicOverlay *basicoverlay, gchar *boxname, gchar *boxlabel, guint x, guint y, 
                           guint width, guint height, gpointer data) {
  g_print("adding box %s\n", boxname);
  BoxInfo *box = g_new0 (BoxInfo, 1);
  box->boxname = g_strdup (boxname);
  box->boxlabel = g_strdup (boxlabel);
  box->x = x;
  box->y = y;
  box->width = width;
  box->height = height;

  basicoverlay->boxlist = g_list_append (basicoverlay->boxlist, box);
}

void
free_boxinfo (gpointer data) {
  BoxInfo *box = (BoxInfo *) data;
  g_free (box->boxname);
  g_free (box->boxlabel);
}

gint
compare_names (gconstpointer a, gconstpointer b) {
  BoxInfo *box_iter;
  gchar *boxname;

  boxname = (gchar *) b;
  box_iter = (BoxInfo *) a;

  g_print ("Comparing %s %s\n", boxname, box_iter->boxname);
  
  return g_strcmp0 (boxname, box_iter->boxname);
}

static void 
gst_basic_overlay_remove_box (GstBasicOverlay *basicoverlay, gchar *boxname) {
  g_print("Removing box %s\n", boxname);
  
  GList *box_item;

  box_item = g_list_find_custom (basicoverlay->boxlist, boxname, compare_names);
  if (box_item) {
    free_boxinfo (box_item->data);
    basicoverlay->boxlist = g_list_remove (basicoverlay->boxlist, box_item->data);
    g_print ("Box List Length: %d\n", g_list_length (basicoverlay->boxlist));
  }
  else {
    g_print ("Box Item %s Not Found\n", boxname);
  }
}

static void
gst_basic_overlay_init (GstBasicOverlay *basicoverlay)
{
  basicoverlay->boxlist = NULL;
  g_signal_connect (G_OBJECT (basicoverlay), "addbox", G_CALLBACK(gst_basic_overlay_add_box), NULL);
  g_signal_connect (G_OBJECT (basicoverlay), "removebox", G_CALLBACK(gst_basic_overlay_remove_box), NULL);
}

void
gst_basic_overlay_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstBasicOverlay *basicoverlay = GST_BASIC_OVERLAY (object);

  GST_DEBUG_OBJECT (basicoverlay, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_basic_overlay_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstBasicOverlay *basicoverlay = GST_BASIC_OVERLAY (object);

  GST_DEBUG_OBJECT (basicoverlay, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_basic_overlay_dispose (GObject * object)
{
  GstBasicOverlay *basicoverlay = GST_BASIC_OVERLAY (object);

  GST_DEBUG_OBJECT (basicoverlay, "dispose");

  /* clean up as possible.  may be called multiple times */
  g_list_free_full (basicoverlay->boxlist, free_boxinfo);
  basicoverlay->boxlist = NULL;

  G_OBJECT_CLASS (gst_basic_overlay_parent_class)->dispose (object);
}

static gboolean
gst_basic_overlay_set_caps (GstBaseTransform * trans, GstCaps * in_caps,
    GstCaps * out_caps)
{
  GstBasicOverlay *basicoverlay = GST_BASIC_OVERLAY (trans);

  if (!gst_video_info_from_caps (&basicoverlay->info, in_caps))
    return FALSE;

  g_print ("In set_caps\n");
  return TRUE;
}

void
draw_box (gpointer data, gpointer user_data) {
  BoxInfo *box = (BoxInfo *) data;
  GstBasicOverlay *basicoverlay = GST_BASIC_OVERLAY (user_data);
  cairo_t *cr = basicoverlay->cr;
  double x, y, width, height;

  x = (double) box->x / basicoverlay->video_width;
  y = (double) box->y / basicoverlay->video_height;
  width = (double) box->width / basicoverlay->video_width;
  height = (double) box->height / basicoverlay->video_height;

  g_print ("Drawing Box %s %lf %lf %lf %lf\n", box->boxname, x, y, width, height);

  cairo_set_source_rgb (cr, 0, 0, 0);

  //draw bounding box 
  cairo_rectangle (cr, x, y, width, height);
  cairo_set_line_width (cr, 0.005);
  cairo_stroke (cr);

  //show Text below bounding box
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_select_font_face (cr, "arial",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, 0.03);
  cairo_move_to (cr, x, y + height + 0.04);
  cairo_show_text (cr, box->boxlabel);

}

static GstFlowReturn
gst_basic_overlay_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstBasicOverlay *basicoverlay = GST_BASIC_OVERLAY (trans);
  GstVideoFrame frame;
  cairo_surface_t *surface;

  GST_DEBUG_OBJECT (basicoverlay, "transform_ip");

  if (!gst_video_frame_map (&frame, &basicoverlay->info, buf, GST_MAP_READWRITE)) {
    return GST_FLOW_ERROR;
  }
  
  basicoverlay->video_width = GST_VIDEO_FRAME_WIDTH (&frame);
  basicoverlay->video_height = GST_VIDEO_FRAME_HEIGHT (&frame);

  //code borrowed from gstcairooverlay.c
  surface =
        cairo_image_surface_create_for_data (GST_VIDEO_FRAME_PLANE_DATA (&frame,
            0), CAIRO_FORMAT_RGB24, GST_VIDEO_FRAME_WIDTH (&frame),
        GST_VIDEO_FRAME_HEIGHT (&frame), GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0));
  //surface = cairo_image_surface_create_from_png ("test.png");
  if (G_UNLIKELY (!surface))
    return GST_FLOW_ERROR;

  basicoverlay->cr = cairo_create (surface);
  if (G_UNLIKELY (!basicoverlay->cr)) {
    cairo_surface_destroy (surface);
    return GST_FLOW_ERROR;
  }

  g_print("%dx%d\n", GST_VIDEO_FRAME_WIDTH (&frame), GST_VIDEO_FRAME_HEIGHT (&frame));
  
  cairo_scale (basicoverlay->cr, GST_VIDEO_FRAME_WIDTH (&frame), GST_VIDEO_FRAME_HEIGHT (&frame));

  //iterate over bounding boxes
  g_list_foreach (basicoverlay->boxlist, draw_box, basicoverlay);

  cairo_destroy (basicoverlay->cr);
  cairo_surface_destroy (surface);

  gst_video_frame_unmap (&frame);

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "basicoverlay", GST_RANK_NONE,
      GST_TYPE_BASIC_OVERLAY);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "Basic Overlay"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "Basic Overlay"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://mndar.github.io"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    basicoverlay,
    "Basic Overlay",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

