#include <stdio.h>
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gchar *pipeline_string = "videotestsrc ! autovideosink";
    GstElement *pipeline;
    GMainLoop *loop;

    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    pipeline = gst_parse_launch (pipeline_string, NULL);
    if (pipeline == NULL) {
        printf("Could Not Create Pipeline");
        exit(1);
    }

    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    g_main_loop_run (loop);
    return 0;
}