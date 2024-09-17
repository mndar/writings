gcc -Werror -o usr/bin/testclass testclass.c myclass.c `pkg-config --cflags --libs glib-2.0 gobject-2.0`
