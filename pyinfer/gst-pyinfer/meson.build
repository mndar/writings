#### Project ####
project('pyinfer', 'c',
        version: '1.0',
        meson_version: '>= 0.58.0')

## Directories ##
lib_install_dir = '/usr/lib/x86_64-linux-gnu/gstreamer-1.0/'


gst_dep = dependency('gstreamer-1.0')

deps = [ gst_dep, dependency('gstreamer-base-1.0'), dependency('gstreamer-app-1.0'), dependency('glib-2.0'), dependency('gio-2.0'), dependency('json-glib-1.0')]

if gst_dep.version() > '1.24'
message ('Enabling Support for GStreamer Analytics Overlay')
deps += dependency('gstreamer-analytics-1.0')
endif
   
subdir('src')

#### Report ####
if meson.version().version_compare('>=0.53')
   summary({'install': lib_install_dir,
           }, section: 'Directories')
endif