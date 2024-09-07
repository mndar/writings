#include <stdio.h>
#include <gst/gst.h>

gboolean my_bus_callback (GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print ("Received EOS on pipeline\n");
            g_main_loop_quit (loop);

            break;
        case GST_MESSAGE_ERROR:
            g_print ("Recievd Error on pipeline\n");
            gchar *debug;
            GError *error;
            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);

            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);

            g_main_loop_quit (loop);
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    //gchar *pipeline_string = "v4l2src ! image/jpeg,width=1280,height=720 ! jpegdec ! videoconvert ! autovideosink";
    gchar *pipeline_string = "videotestsrc ! autovideosink";
    GstElement *pipeline;
    GMainLoop *loop;
    GstBus *bus;

    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    pipeline = gst_parse_launch (pipeline_string, NULL);
    if (pipeline == NULL) {
        printf("Could Not Create Pipeline\n");
        exit(1);
    }

    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, my_bus_callback, loop);
    gst_object_unref (bus);

    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    g_main_loop_run (loop);
    return 0;
}