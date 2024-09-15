#ifndef __MYCLASS_H__
#define __MYCLASS_H__

#include <glib.h>
#include <glib-object.h>

#define MYCLASS_TYPE    (myclass_get_type ())
#define MYCLASS(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), MYCLASS_TYPE, MyClass))
#define MYCLASS_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), MYCLASS_TYPE, MyClassClass))
#define MYCLASS_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), MYCLASS_TYPE, MyClassClass))

typedef struct _MyClass MyClass;
typedef struct _MyClassClass MyClassClass;

typedef struct {
    gint c, d;
}MyClassPrivate;

struct _MyClass {
    GObject parent;
    gint a, b;
};

struct _MyClassClass {
    GObjectClass parent;
};

MyClass *myclass_new();
void myclass_set_a_b (MyClass *, int, int);

#endif