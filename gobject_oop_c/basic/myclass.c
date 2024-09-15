#include <glib.h>
#include "myclass.h"
G_DEFINE_TYPE_WITH_PRIVATE(MyClass, myclass, G_TYPE_OBJECT);

static void
myclass_init (MyClass *self) {

}

static void
myclass_class_init (MyClassClass *klass) {
    //GObjectClass *object_class = G_OBJECT_CLASS (klass);

}

MyClass *
myclass_new () {
    MyClass *obj;
    g_print ("Creating object of MyClass\n");
    obj = g_object_new (MYCLASS_TYPE, NULL);
    
    return obj;
}

void
myclass_set_a_b (MyClass *myclass, int a, int b) {
    g_print ("Setting a and b\n");
}

