#include <glib.h>
#include "myclass.h"
G_DEFINE_TYPE_WITH_PRIVATE(MyClass, myclass, G_TYPE_OBJECT);

static void
myclass_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
	MyClass *self = MYCLASS (object);
    MyClassPrivate *priv = myclass_get_instance_private (self);

	switch (property_id) {
        case PROP_OPERATION:
            priv->operation = g_value_get_int (value);
            break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
}

static void
myclass_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {
	MyClass *self = MYCLASS (object);
	MyClassPrivate *priv = myclass_get_instance_private (self);
	
    switch (property_id) {
		case PROP_OPERATION:
			g_value_set_int (value, priv->operation);
			break;
	}
}

static void
myclass_class_init (MyClassClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->set_property = myclass_set_property;
    object_class->get_property = myclass_get_property;

    myclass_properties[PROP_OPERATION] = g_param_spec_int ("operation", "Operation", 
                                            "Operation to be performed on the values.", -1, 10,
                                            -1, G_PARAM_READWRITE);

	g_object_class_install_properties (object_class, N_PROPERTIES, myclass_properties);


    //signals
    myclass_signals[SIGNAL_OPERATION_COMPLETE] = g_signal_new ("operation-complete",
                                                            G_TYPE_FROM_CLASS (klass),
                                                            G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
                                                            G_TYPE_NONE, 1, G_TYPE_INT);  
}

static void
myclass_init (MyClass *self) {
    g_print ("Myclass Constructor\n");   
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
    MyClassPrivate *priv = myclass_get_instance_private (myclass);
    priv->a = a;
    priv->b = b;
}

void
myclass_perform_op (MyClass *myclass) {
    MyClassPrivate *priv = myclass_get_instance_private (myclass);
    switch (priv->operation) {
        case 0: //add
            int result_add = priv->a + priv->b;
            g_signal_emit_by_name (myclass, "operation-complete", result_add);
            break;
        case 1:
            int result_mul = priv->a * priv->b;
            g_signal_emit_by_name (myclass, "operation-complete", result_mul);
            break;
    }
}

