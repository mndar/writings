#!/bin/python3

import os
import sys
import gi
import json
gi.require_version('Gst','1.0')

from gi.repository import Gst, GLib

def bus_callback(bus,msg,data):
    loop = data
    t = msg.type
    if t == Gst.MessageType.EOS:
        print("Received EOS on pipeline\n")
        loop.quit()
    elif t == Gst.MessageType.ERROR:
        print("Received Error on pipeline\n")
        loop.quit()
    elif t == Gst.MessageType.APPLICATION:
        structure = msg.get_structure()
        inference_str = structure.get_string ("inference");
        inference = json.loads(inference_str)
        print(inference["result"]["boxes"])

    return True

if __name__ == "__main__":
    Gst.init(sys.argv)
    type = sys.argv[1]

    if type == "file":
        filepath = "archive/3.jpg"
        pipeline_string = f"filesrc location={filepath} ! jpegdec ! videoconvert ! video/x-raw,format=BGR ! pyinfer model=yolo ! \
                           objectdetectionoverlay ! videoconvert ! pngenc ! filesink location=out.png"
    elif type == "webcam":
        pipeline_string = "v4l2src device=/dev/video0 ! image/jpeg,width=640,height=480 ! jpegdec !  videoconvert ! video/x-raw,format=BGR ! \
                           videorate ! video/x-raw,framerate=3/1 ! pyinfer model=yolo ! objectdetectionoverlay ! videoconvert !  ximagesink sync=false"
    pipeline = Gst.parse_launch(pipeline_string)
    loop = GLib.MainLoop()

    bus = pipeline.get_bus()
    bus.add_watch(GLib.PRIORITY_DEFAULT, bus_callback, loop)
    pipeline.set_state (Gst.State.PLAYING)
    
    loop.run()