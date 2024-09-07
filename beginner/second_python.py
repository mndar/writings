#!/bin/python3

import os
import sys
import gi
gi.require_version('Gst','1.0')

from gi.repository import Gst, GLib

def bus_callback(bus,msg,data):
    loop = data
    t = msg.type
    if t == Gst.MessageType.EOS:
        print("Received EOS on pipeline\n")
        loop.quit()
    elif t == Gst.MessageType.ERROR:
        print("Receievd Error on pipeline\n");
        loop.quit()
    
if __name__ == "__main__":
    Gst.init(sys.argv)
    pipeline = Gst.parse_launch("videotestsrc ! autovideosink")
    loop = GLib.MainLoop()

    bus = pipeline.get_bus()
    bus.add_watch(GLib.PRIORITY_DEFAULT, bus_callback, loop)
    pipeline.set_state (Gst.State.PLAYING)
    
    loop.run()