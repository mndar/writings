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
 * SECTION:element-gstpyinfer
 *
 * The pyinfer element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! pyinfer ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstpyinfer.h"

GST_DEBUG_CATEGORY_STATIC (gst_pyinfer_debug_category);
#define GST_CAT_DEFAULT gst_pyinfer_debug_category

/* prototypes */


static void gst_pyinfer_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_pyinfer_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_pyinfer_dispose (GObject * object);
static void gst_pyinfer_finalize (GObject * object);

static gboolean gst_pyinfer_set_caps (GstBaseTransform * trans,
    GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_pyinfer_start (GstBaseTransform * trans);
static gboolean gst_pyinfer_stop (GstBaseTransform * trans);
static GstFlowReturn gst_pyinfer_transform_ip (GstBaseTransform * trans,
    GstBuffer * buf);

void output_stream_write_line (GstPyinfer *pyinfer, gchar *msg);
void input_stream_read_line (GstPyinfer *pyinfer, gchar *msg, int max_len);
enum
{
  PROP_0
};

/* pad templates */

static GstStaticPadTemplate gst_pyinfer_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    );

static GstStaticPadTemplate gst_pyinfer_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
  );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstPyinfer, gst_pyinfer, GST_TYPE_BASE_TRANSFORM,
  GST_DEBUG_CATEGORY_INIT (gst_pyinfer_debug_category, "pyinfer", 0,
  "debug category for pyinfer element"));

static void
gst_pyinfer_class_init (GstPyinferClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_pyinfer_src_template);
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_pyinfer_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "GStreamer PyInfer", "Generic", "GStreamer Element from Python Inference Server",
      "Mandar Joshi <emailmandar@gmail.com>");

  gobject_class->set_property = gst_pyinfer_set_property;
  gobject_class->get_property = gst_pyinfer_get_property;
  gobject_class->dispose = gst_pyinfer_dispose;
  gobject_class->finalize = gst_pyinfer_finalize;
  base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_pyinfer_set_caps);
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_pyinfer_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_pyinfer_stop);
  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_pyinfer_transform_ip);

  pyinfer_properties[PYINFER_PROP_MODEL] = g_param_spec_string ("model", "model", "Name of the model to use for inference", "None", G_PARAM_WRITABLE);
  g_object_class_install_properties (gobject_class, PYINFER_N_PROPERTIES, pyinfer_properties);

}

static void
gst_pyinfer_init (GstPyinfer *pyinfer)
{
  pyinfer->socket_path = g_strdup ("/tmp/pyinfer.sock");
  pyinfer->session_id = NULL;
  pyinfer->infer_command = NULL;
  pyinfer->model_name = g_strdup ("None");

  #ifdef ANALYTICS_OVERLAY_ENABLE
  pyinfer->add_meta = FALSE;
  #endif
}

void
gst_pyinfer_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstPyinfer *pyinfer = GST_PYINFER (object);

  GST_DEBUG_OBJECT (pyinfer, "set_property");

  switch (property_id) {
    case PYINFER_PROP_MODEL:
      pyinfer->model_name = g_strdup(g_value_get_string (value));
      if (g_strcmp0 (pyinfer->model_name, "yolo") == 0) {
        #ifdef ANALYTICS_OVERLAY_ENABLE
        pyinfer->add_meta = TRUE;
        #endif
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_pyinfer_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstPyinfer *pyinfer = GST_PYINFER (object);

  GST_DEBUG_OBJECT (pyinfer, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_pyinfer_dispose (GObject * object)
{
  GstPyinfer *pyinfer = GST_PYINFER (object);

  GST_DEBUG_OBJECT (pyinfer, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_pyinfer_parent_class)->dispose (object);
}

void
gst_pyinfer_finalize (GObject * object)
{
  GstPyinfer *pyinfer = GST_PYINFER (object);

  GST_DEBUG_OBJECT (pyinfer, "finalize");

  /* clean up object here */
  g_free (pyinfer->socket_path);

  if (pyinfer->session_id) {
    g_remove (pyinfer->mmap_path);
    g_free (pyinfer->session_id);
    g_free (pyinfer->mmap_path);
  }

  if (pyinfer->model_name) {
    g_free (pyinfer->model_name);
  }
  G_OBJECT_CLASS (gst_pyinfer_parent_class)->finalize (object);
}

static gboolean
gst_pyinfer_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstPyinfer *pyinfer = GST_PYINFER (trans);
  GstStructure *caps_structure;
  gint width, height;
  gboolean error = FALSE;
  gchar *caps_string, *extra_data, *extra_data_command;

  GST_DEBUG_OBJECT (pyinfer, "set_caps");
  //get type of stream
  caps_string = gst_caps_to_string (incaps);
  if (g_str_has_prefix (caps_string, "video"))
    pyinfer->stream_type = VIDEO_STREAM;
  else
    pyinfer->stream_type = NONE_STREAM;
  g_free (caps_string);

  switch (pyinfer->stream_type) {
    case VIDEO_STREAM:
    caps_structure = gst_caps_get_structure (incaps, 0);
      if (!gst_structure_get_int (caps_structure, "width", &width)) {
        error = TRUE;
        break;
      }
      if (!gst_structure_get_int (caps_structure, "height", &height)) {
        error = TRUE;
        break;
      }
      extra_data = g_strdup_printf ("{\"streamType\":%d, \"model\":\"%s\", \"width\":%d, \"height\":%d}", pyinfer->stream_type, pyinfer->model_name, width, height);
      extra_data_command = g_strdup_printf ("{\"type\":\"extraData\", \"sessionId\":\"%s\", \"extraData\":%s}\n", pyinfer->session_id, extra_data);
      output_stream_write_line (pyinfer, extra_data_command);
      g_free (extra_data_command);
      g_free (extra_data);
      break;
    default:
      GST_ERROR_OBJECT (pyinfer, "Only Video Streams are supported");
      error = TRUE;
  }

  if (error)
    return FALSE;
  else
    return TRUE;
}

void
input_stream_read_line (GstPyinfer *pyinfer, gchar *buffer, int max_len) {
    GError *error = NULL;
    gssize n;
    gssize bytes_read;
    bytes_read = 0;
    char c;

    memset (buffer, 0, max_len + 1);
    while (TRUE) {
        n = g_input_stream_read (pyinfer->istream, &c, 1, NULL, &error);
        if (error)
            g_error ("%s", error->message);
        
        if (c == '\n' || n == 0 || n == -1)
            break;

        buffer[bytes_read] = c;
        bytes_read++;

        if (bytes_read == MAX_MSG_LEN)
          break;
    }
}

void output_stream_write_line (GstPyinfer *pyinfer, gchar *msg) {
  GError *error = NULL;
  g_output_stream_write  (pyinfer->ostream,
                          msg,
                          strlen(msg),
                          NULL,
                          &error);
  if (error != NULL)
    GST_ERROR_OBJECT (pyinfer, "%s", error->message);
}

gchar * get_json_string (GstPyinfer *pyinfer, gchar *buffer, gchar *var) {
  JsonParser *parser;
  JsonNode *root;
  JsonObject *object;
  GError *error;
  gchar *var_string;

  parser = json_parser_new ();

  error = NULL;
  json_parser_load_from_data (parser, buffer, strlen(buffer), &error);
  if (error)
    {
        GST_ERROR_OBJECT (pyinfer, "Unable to parse json messge: %s", error->message);
        g_error_free (error);
        g_object_unref (parser);
        return NULL;
    }
  root = json_parser_get_root (parser);
  //check errors here
  object = json_node_get_object (root);
  var_string = g_strdup (json_object_get_string_member (object, var));
  g_object_unref (parser);

  return var_string;
}

gboolean init_mmap (GstPyinfer *pyinfer, const gchar *session_id) {

  pyinfer->mmap_path = g_strdup_printf ("/tmp/%s", session_id);
  g_remove (pyinfer->mmap_path);

  GST_DEBUG_OBJECT (pyinfer, "Creating Memory Mapped File: %s", pyinfer->mmap_path);
  pyinfer->fdimage = open(pyinfer->mmap_path, O_RDWR|O_CREAT, 0666);

  if (pyinfer->fdimage < 0) {
    GST_ERROR_OBJECT (pyinfer,"Could Not Open mmap path");
    return FALSE;
  }
  
  ftruncate (pyinfer->fdimage, MAX_MMAP_SIZE);

  pyinfer->image_memory = mmap (NULL, MAX_MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, 
                                pyinfer->fdimage, 0);

  if (pyinfer->image_memory == MAP_FAILED) {
    GST_ERROR_OBJECT (pyinfer, "Could Not Map Memory");
    return FALSE;
  }

  return TRUE;
}

/* states */
static gboolean
gst_pyinfer_start (GstBaseTransform * trans)
{
  GstPyinfer *pyinfer = GST_PYINFER (trans);
  gchar *login_msg = "{\"type\":\"login\"}\n";
  gchar buffer[MAX_MSG_LEN + 1];
  gboolean ret;
  GError *error = NULL;

  GST_DEBUG_OBJECT (pyinfer, "start");

  //create new socket
  pyinfer->socket = g_socket_new (G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM, 
                                G_SOCKET_PROTOCOL_DEFAULT, &error);
  if (error != NULL) {
    GST_ERROR_OBJECT (trans, "%s", error->message);
    return FALSE;
  }
  
  //socket address poiting to unix socket path
  pyinfer->address = g_unix_socket_address_new_with_type (pyinfer->socket_path, -1, 
                                                        G_UNIX_SOCKET_ADDRESS_PATH);

  //connect
  g_socket_connect (pyinfer->socket, pyinfer->address, NULL, &error);
  if (error != NULL) {
    GST_ERROR_OBJECT (trans, "Unable to connect to Unix Socket (/tmp/pyinfer.sock): %s", error->message);
    return FALSE;
  }

  //Get IO Stream
  pyinfer->connection = g_socket_connection_factory_create_connection (pyinfer->socket);
  /* use the connection */
  pyinfer->istream = g_io_stream_get_input_stream (G_IO_STREAM (pyinfer->connection));
  pyinfer->ostream = g_io_stream_get_output_stream (G_IO_STREAM (pyinfer->connection));
  
  //Login
  output_stream_write_line (pyinfer, login_msg);
  //read reply containing session id
  input_stream_read_line (pyinfer, buffer, MAX_MSG_LEN);

  //init mmap to /tmp/sessionid
  pyinfer->session_id = get_json_string (pyinfer, buffer, "sessionId");
  ret = init_mmap (pyinfer, pyinfer->session_id);
  
  //start if mmap succeded
  return ret;
}

static gboolean
gst_pyinfer_stop (GstBaseTransform * trans)
{
  GstPyinfer *pyinfer = GST_PYINFER (trans);
  gchar *logout_msg = "{\"type\":\"logout\"}\n";
  GST_DEBUG_OBJECT (pyinfer, "stop");

  output_stream_write_line (pyinfer, logout_msg);  
  g_socket_close (pyinfer->socket, NULL);

  return TRUE;
}

#ifdef ANALYTICS_OVERLAY_ENABLE
void add_od_meta (JsonArray *array,
                  guint index_,
                  JsonNode *element_node,
                  gpointer user_data) {
    
  GstAnalyticsRelationMeta *rmeta;
  GstAnalyticsODMtd od_mtd;
  JsonObject *object;
  gboolean ret;

  rmeta = user_data;
  object = json_node_get_object (element_node);

  JsonObject *xywh = json_object_get_object_member (object, "xywh");

  GQuark type = g_quark_from_string (json_object_get_string_member (object, "name"));
  gfloat conf = json_object_get_double_member (object, "conf");

  gint x = json_object_get_int_member (xywh, "x");
  gint y = json_object_get_int_member (xywh, "y");
  gint w = json_object_get_int_member (xywh, "w");
  gint h = json_object_get_int_member (xywh, "h");

  ret = gst_analytics_relation_meta_add_od_mtd (rmeta, type, x, y,
      w, h, conf, &od_mtd);

  if (ret != TRUE)
    g_print ("Adding Meta Failed");
}

void parse_od_meta (GstPyinfer *pyinfer, GstBuffer *buf, gchar *inference) {
  GstAnalyticsRelationMetaInitParams init_params = { 5, 150 };
  GstAnalyticsRelationMeta *rmeta;
  
  

  JsonParser *parser;
  JsonNode *root;
  JsonObject *object;
  GError *error;

  //g_print ("Inference String: %s\n", inference);
  // {"type": "inference", "sessionId": "2e891ad7-57d6-401c-a197-ad9ba0ca10b4", "result": {"detections": 1, "boxes": [{"name": "person", "conf": 0.8934722542762756, "xywh": {"x": 291, "y": 262, "w": 580, "h": 433}}]}}
  rmeta = gst_buffer_add_analytics_relation_meta_full (buf, &init_params);
  
  //Parse JSON infernce string
  parser = json_parser_new ();

  error = NULL;
  json_parser_load_from_data (parser, inference, strlen(inference), &error);
  if (error)
  {
      GST_ERROR_OBJECT (pyinfer, "Unable to parse inference json messge: %s", error->message);
      g_error_free (error);
      g_object_unref (parser);
      return;
  }
  root = json_parser_get_root (parser);
  //check errors here
  object = json_node_get_object (root);
  
  JsonObject *result = json_object_get_object_member (object, "result");
  JsonArray *boxes = json_object_get_array_member (result, "boxes");

  json_array_foreach_element (boxes, add_od_meta, rmeta);

  //get result object member here
  g_object_unref (parser);
}

#endif

void post_inference_on_bus (GstPyinfer *pyinfer, gchar *inference) {

  GstStructure *structure;
  GstMessage *msg;
  GstBus *bus;
  gboolean ret;

  structure = gst_structure_new("inference_string",
                            "inference", G_TYPE_STRING, inference,
                            NULL);
	msg = gst_message_new_application (GST_OBJECT (pyinfer), structure);
  bus = gst_element_get_bus (GST_ELEMENT (pyinfer));

  ret = gst_bus_post (bus, msg);
	if (!ret)
	  GST_ERROR_OBJECT (pyinfer, "Error posting Inference on bus. Application won't receive inference");
}

static GstFlowReturn
gst_pyinfer_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstPyinfer *pyinfer = GST_PYINFER (trans);
  GstMapInfo mapinfo;
  gboolean ret;
  gchar inference[MAX_MSG_LEN];
  //GST_DEBUG_OBJECT (pyinfer, "transform_ip");

  ret = gst_buffer_map (buf, &mapinfo, GST_MAP_READ);
  if (!ret) {
    GST_ERROR_OBJECT (pyinfer, "Failed to map buffer");
    goto end;
  }

  GST_DEBUG_OBJECT (trans, "Buf Size: %ld", mapinfo.size);

  //Copy Buffer Data to memory mapped region
  memcpy ((void *)pyinfer->image_memory, mapinfo.data, mapinfo.size);
  //memcpy ((void *)pyinfer->image_memory, test_data, 4);


  pyinfer->infer_command = g_strdup_printf ("{\"type\":\"infer\", \"sessionId\":\"%s\", \"mapsize\": %ld}\n", 
                                            pyinfer->session_id, mapinfo.size);
  output_stream_write_line (pyinfer, pyinfer->infer_command);
  g_free (pyinfer->infer_command);

  input_stream_read_line (pyinfer, inference, MAX_MSG_LEN);

  //g_print ("Inference Result: %s\n", buffer);

  #ifdef ANALYTICS_OVERLAY_ENABLE
  if (pyinfer->add_meta)
    parse_od_meta (pyinfer, buf, inference);
  #endif

  post_inference_on_bus (pyinfer, inference);
  gst_buffer_unmap (buf, &mapinfo);  

end:
  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "pyinfer", GST_RANK_NONE,
      GST_TYPE_PYINFER);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "PyInfer Package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "GStreamer Element for Python Inference"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://mndar.github.io"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    pyinfer,
    "Gstremer PyInfer",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

