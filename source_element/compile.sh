gcc -Wall -Werror -fPIC $CPPFLAGS `pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0` -c -o gstmyjpegstreamer.o gstmyjpegstreamer.c
gcc -shared -o gstmyjpegstreamer.so gstmyjpegstreamer.o `pkg-config --libs gstreamer-1.0 gstreamer-base-1.0`
