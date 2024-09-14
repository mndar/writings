#!/usr/bin/python3

import os
import sys
import gi
gi.require_version('Gst','1.0')

from gi.repository import Gst, GLib
from threading import Timer

def add_boxes(pipeline):
    print('Adding Boxes\n')
    basicoverlaypy = pipeline.get_by_name("boverlay")
    basicoverlaypy.emit("addbox", "firstbox", "First Box", 100, 10, 300, 300)
    basicoverlaypy.emit("addbox", "secondbox", "Second Box", 1024, 250, 300, 400);

def remove_boxes(pipeline):
    print("Removing Boxes\n")
    basicoverlaypy = pipeline.get_by_name("boverlay")
    basicoverlaypy.emit("removebox", "firstbox")

if __name__ == "__main__":
    Gst.init(sys.argv)
    pipeline = Gst.parse_launch("videotestsrc pattern=4 ! video/x-raw,width=1920,height=1080 ! videoconvert ! basicoverlaypy name=boverlay ! videoconvert ! autovideosink")
    pipeline.set_state (Gst.State.PLAYING)

    timer1 =  Timer(2.0, add_boxes, args=[pipeline,])
    timer1.start()

    timer2 = Timer(6.0, remove_boxes, args=[pipeline,])
    timer2.start()

    loop = GLib.MainLoop();
    loop.run()