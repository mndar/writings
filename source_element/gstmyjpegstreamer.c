/* GStreamer
 * Copyright (C) 2024 FIXME <fixme@example.com>
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
 * SECTION:element-gstmyjpegstreamer
 *
 * The myjpegstreamer element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! myjpegstreamer ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
#include "gstmyjpegstreamer.h"


GST_DEBUG_CATEGORY_STATIC (gst_my_jpeg_streamer_debug_category);
#define GST_CAT_DEFAULT gst_my_jpeg_streamer_debug_category

/* prototypes */


static void gst_my_jpeg_streamer_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_my_jpeg_streamer_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);

static gboolean gst_my_jpeg_streamer_start(GstBaseSrc *src);
static GstFlowReturn gst_my_jpeg_streamer_fill (GstPushSrc * src, GstBuffer * buf);
static void gst_my_jpeg_streamer_dispose (GObject * gobject);

enum
{
  PROP_0,
  PROP_DIRECTORY
};

/* pad templates */

static GstStaticPadTemplate gst_my_jpeg_streamer_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jpeg")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstMyJpegStreamer, gst_my_jpeg_streamer, GST_TYPE_PUSH_SRC,
  GST_DEBUG_CATEGORY_INIT (gst_my_jpeg_streamer_debug_category, "myjpegstreamer", 0,
  "debug category for myjpegstreamer element"));

static void
gst_my_jpeg_streamer_class_init (GstMyJpegStreamerClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstPushSrcClass *push_src_class = GST_PUSH_SRC_CLASS (klass);
  GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_my_jpeg_streamer_src_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_my_jpeg_streamer_set_property;
  gobject_class->get_property = gst_my_jpeg_streamer_get_property;

  base_src_class->start = GST_DEBUG_FUNCPTR (gst_my_jpeg_streamer_start);
  push_src_class->fill = GST_DEBUG_FUNCPTR (gst_my_jpeg_streamer_fill);
  gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_my_jpeg_streamer_dispose);

   g_object_class_install_property
      (gobject_class, PROP_DIRECTORY,
      g_param_spec_string ("directory", "Directory",
          "Value of the Directory containing JPEG files",
          "myjpegstreamer", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

}

static void
gst_my_jpeg_streamer_init (GstMyJpegStreamer *myjpegstreamer)
{
  myjpegstreamer->directory = NULL;
  myjpegstreamer->jpeg_files = NULL;
  myjpegstreamer->curr_file = NULL;
  myjpegstreamer->file_index = 0;
}

void
gst_my_jpeg_streamer_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMyJpegStreamer *myjpegstreamer = GST_MY_JPEG_STREAMER (object);

  GST_DEBUG_OBJECT (myjpegstreamer, "set_property");

  switch (property_id) {
    case PROP_DIRECTORY:
      const gchar *directory;
      directory = g_value_get_string (value);
      myjpegstreamer->directory = g_strdup (directory);
      g_print ("Setting JPEG Directory to %s\n", myjpegstreamer->directory);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_my_jpeg_streamer_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstMyJpegStreamer *myjpegstreamer = GST_MY_JPEG_STREAMER (object);

  GST_DEBUG_OBJECT (myjpegstreamer, "get_property");

  switch (property_id) {
    case PROP_DIRECTORY:
      g_value_set_string (value, myjpegstreamer->directory);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

/* start and stop processing, ideal for opening/closing the resource */
static gboolean
gst_my_jpeg_streamer_start (GstBaseSrc * src)
{
  GstMyJpegStreamer *myjpegstreamer = GST_MY_JPEG_STREAMER (src);

  GST_DEBUG_OBJECT (myjpegstreamer, "start");

  GDir *dir;
  GError *error = NULL;
  const gchar *file;

  dir = g_dir_open(myjpegstreamer->directory, 0, &error);
  while ((file = g_dir_read_name(dir))) {
      if (g_str_has_suffix (file, ".jpeg") || g_str_has_suffix (file, "jpg")) {
        g_print("%s\n", file);
        myjpegstreamer->jpeg_files = 
          g_list_append (myjpegstreamer->jpeg_files, g_strdup (file));
      }
  }

  g_dir_close (dir);

  myjpegstreamer->num_files = g_list_length (myjpegstreamer->jpeg_files);

  return TRUE;
}

gboolean
stream_list_file (GstMyJpegStreamer *src, gchar *directory, gchar *filename, GstBuffer *buf) {
  g_print("Copying File %s To Buffer\n", filename);
  
  long count;
  gsize buf_size;
  gchar *file_path;
  
  GstMapInfo info;

  if (!src->curr_file) {
    file_path = g_strdup_printf ("%s/%s", directory, filename);
    src->curr_file = fopen (file_path, "rb");
    g_free (file_path);
  }

  gst_buffer_map (buf, &info, GST_MAP_WRITE);
  buf_size = gst_buffer_get_size (buf);
  
  count = fread (info.data, 1, buf_size, src->curr_file);
  
  g_print ("File Index: %d Count: %ld, Buf Size: %ld\n", src->file_index, count, buf_size);

  if (count < buf_size) {
    fclose (src->curr_file);
    src->curr_file = NULL;
    gst_buffer_resize (buf, 0, count);
    gst_buffer_unmap (buf, &info);
    return FALSE;
  }

  gst_buffer_unmap (buf, &info);
  return TRUE;
}
 
/* ask the subclass to fill the buffer with data from offset and size */
static GstFlowReturn
gst_my_jpeg_streamer_fill (GstPushSrc * src, GstBuffer * buf)
{
  GstMyJpegStreamer *myjpegstreamer = GST_MY_JPEG_STREAMER (src);
  GList *file;
  gchar *filename;
  gboolean ret;

  GST_DEBUG_OBJECT (myjpegstreamer, "fill");
  
  if (myjpegstreamer->num_files == 0)
    return GST_FLOW_EOS;

  if (myjpegstreamer->file_index >  myjpegstreamer->num_files - 1) {
    return GST_FLOW_EOS;
  }

  file = g_list_nth (myjpegstreamer->jpeg_files, myjpegstreamer->file_index);
  filename = (gchar *) file->data;
  g_print ("Streaming File %s\n", filename);

  ret = stream_list_file (myjpegstreamer, myjpegstreamer->directory, filename, buf);
  if (!ret)
    myjpegstreamer->file_index++;
  
  return GST_FLOW_OK;
}

void free_list_string (gpointer data) {
  g_print("Freeing List Data\n");
  g_free (data);
}

static void
gst_my_jpeg_streamer_dispose (GObject *gobject) {
  GstPushSrc *src = GST_PUSH_SRC (gobject);
  GstMyJpegStreamer *myjpegstreamer = GST_MY_JPEG_STREAMER (src);

  g_free (myjpegstreamer->directory);
  g_list_free_full (myjpegstreamer->jpeg_files, free_list_string);
  G_OBJECT_CLASS (gst_my_jpeg_streamer_parent_class)->dispose (gobject);
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "myjpegstreamer", GST_RANK_NONE,
      GST_TYPE_MY_JPEG_STREAMER);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    myjpegstreamer,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

