gcc -Wall -Werror -fPIC $CPPFLAGS `pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 cairo-gobject` -c -o gstbasicoverlay.o gstbasicoverlay.c
gcc -shared -o gstbasicoverlay.so gstbasicoverlay.o `pkg-config --libs gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 cairo-gobject`
