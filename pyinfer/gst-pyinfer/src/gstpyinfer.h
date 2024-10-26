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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_PYINFER_H_
#define _GST_PYINFER_H_

#include <gst/base/gstbasetransform.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <glib/gstdio.h>

#include <json-glib/json-glib.h>

#if GST_VERSION_MAJOR == 1 && GST_VERSION_MINOR >= 24
#define ANALYTICS_OVERLAY_ENABLE
#include <gst/analytics/analytics.h>
#endif

#define FOO_TYPE_OBJECT (foo_object_get_type ())

G_BEGIN_DECLS

#define GST_TYPE_PYINFER   (gst_pyinfer_get_type())
#define GST_PYINFER(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PYINFER,GstPyinfer))
#define GST_PYINFER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PYINFER,GstPyinferClass))
#define GST_IS_PYINFER(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PYINFER))
#define GST_IS_PYINFER_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PYINFER))

#define MAX_MSG_LEN 2048
#define MAX_MMAP_SIZE 1920*1080*4

typedef struct _GstPyinfer GstPyinfer;
typedef struct _GstPyinferClass GstPyinferClass;

typedef enum {
  NONE_STREAM,
  VIDEO_STREAM
}StreamType;

enum {
  PYINFER_PROP_MODEL=1,
  PYINFER_N_PROPERTIES
};

static GParamSpec *pyinfer_properties[PYINFER_N_PROPERTIES] = { NULL, };


struct _GstPyinfer
{
  GstBaseTransform base_pyinfer;
  GSocketConnection *connection;
  GSocket *socket;
  GSocketAddress *address;
  GInputStream *istream;
  GOutputStream *ostream;

  gchar *socket_path, *mmap_path;
  gchar *session_id;
  gchar *infer_command;
  
  int fdimage;
  void *image_memory;

  StreamType stream_type;
  gchar *model_name;

  #ifdef ANALYTICS_OVERLAY_ENABLE
  gboolean add_meta;
  #endif
};

struct _GstPyinferClass
{
  GstBaseTransformClass base_pyinfer_class;
};

GType gst_pyinfer_get_type (void);

G_END_DECLS

#endif
