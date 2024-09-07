#!/usr/bin/python3

import os
import sys
import gi
gi.require_version('Gst','1.0')

from gi.repository import Gst, GLib

if __name__ == "__main__":
    Gst.init(sys.argv)
    pipeline = Gst.parse_launch("videotestsrc ! autovideosink")
    pipeline.set_state (Gst.State.PLAYING)
    loop = GLib.MainLoop();
    loop.run()