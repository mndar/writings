#include "myclass.h"

static void operation_complete (MyClass *object, gint result, gpointer data) {
    g_print("Result: %d\n", result);
}

int main (int argc, char *argv[]) {
    MyClass *object;

    object = myclass_new ();
    g_signal_connect (object, "operation-complete", G_CALLBACK(operation_complete), NULL);
    
    myclass_set_a_b (object, 10, 20);
    g_object_set (object, "operation", 0, NULL);
    myclass_perform_op (object);
    return 0;
}