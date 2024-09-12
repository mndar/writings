#include <stdio.h>
#include <gst/gst.h>


gboolean timeout_add_box (gpointer data) {
    g_print ("Adding Box\n");
    GstElement *pipeline = (GstElement *) data;
    GstElement *element;
    element = gst_bin_get_by_name (GST_BIN (pipeline), "boverlay");
    g_signal_emit_by_name (element, "addbox", "firstbox", "First Box", 100, 10, 300, 300);
    g_signal_emit_by_name (element, "addbox", "secondbox", "Second Box", 1024, 400, 300, 400);

    //g_signal_emit_by_name (element, "addbox", "thirdbox", "Third Box", 1600, 400, 300, 400);
    //g_signal_emit_by_name (element, "addbox", "fourthbox", "Fourth Box", 100, 600, 300, 400);
    return FALSE;
}

gboolean timeout_remove_box (gpointer data) {
    g_print ("Removing Box\n");
    GstElement *pipeline = (GstElement *) data;
    GstElement *element;
    element = gst_bin_get_by_name (GST_BIN (pipeline), "boverlay");
    g_signal_emit_by_name (element, "removebox", "firstbox");
    return FALSE;
}

int main(int argc, char *argv[]) {
    gchar *pipeline_string = "videotestsrc pattern=5 ! video/x-raw,width=1920,height=1080 ! videoconvert ! basicoverlay name=boverlay ! videoconvert ! autovideosink";
    GstElement *pipeline;
    GMainLoop *loop;
    GstElement *element;

    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    pipeline = gst_parse_launch (pipeline_string, NULL);
    if (pipeline == NULL) {
        printf("Could Not Create Pipeline");
        exit(1);
    }

    
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    g_timeout_add_seconds (2, timeout_add_box, pipeline);
    g_timeout_add_seconds (6, timeout_remove_box, pipeline);
    g_main_loop_run (loop);
    return 0;
}


