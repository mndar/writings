# sudo apt install gstreamer1.0-python3-plugin-loader
import gi
import cairo

gi.require_version('Gst', '1.0')
gi.require_version('GstVideo', '1.0')
from gi.repository import Gst, GLib, GObject, GstVideo, GstBase

Gst.init(None)

class BoxInfo:
    def __init__(self):
        self.name = ""
        self.label = ""
        self.x = 0
        self.y = 0
        self.width = 0
        self.height = 0

    def createBox(self, name, label, x, y, width, height):
        self.name = name
        self.label = label
        self.x = x
        self.y = y
        self.width = width
        self.height = height

class BasicOverlayPy(GstBase.BaseTransform):
    __gstmetadata__ = ('BasicOverlayPy','Filter', \
                      'Draw Bounding Boxes', 'Mandar Joshi')

    __gsttemplates__ = (Gst.PadTemplate.new("src",
                                            Gst.PadDirection.SRC,
                                            Gst.PadPresence.ALWAYS,
                                            GstVideo.video_make_raw_caps([GstVideo.VideoFormat.BGRX, GstVideo.VideoFormat.BGRA, GstVideo.VideoFormat.RGB16])),
                        Gst.PadTemplate.new("sink",
                                            Gst.PadDirection.SINK,
                                            Gst.PadPresence.ALWAYS,
                                            GstVideo.video_make_raw_caps([GstVideo.VideoFormat.BGRX, GstVideo.VideoFormat.BGRA, GstVideo.VideoFormat.RGB16])))
    __gsignals__ = {
        "addbox": (GObject.SignalFlags.RUN_LAST|GObject.SignalFlags.ACTION, GObject.TYPE_NONE, [GObject.TYPE_STRING, GObject.TYPE_STRING, GObject.TYPE_INT, GObject.TYPE_INT, GObject.TYPE_INT, GObject.TYPE_INT]),
        "removebox": (GObject.SignalFlags.RUN_LAST|GObject.SignalFlags.ACTION, GObject.TYPE_NONE, [GObject.TYPE_STRING,]),
    }

    def add_box_to_list(self, box):
        print(f'List add {box.name}\n')
        self.boxlist.append(box)
        print(f'boxlist len {len(self.boxlist)}\n')
    
    def remove_box_from_list(self, name):
        foundbox = None
        for boxiter in self.boxlist:
            if boxiter.name == name:
                foundbox = boxiter
        if foundbox:
            print(f'Found box {foundbox.name}')
            self.boxlist.remove(foundbox)
            print(f'New Boxlist len {len(self.boxlist)}')

    @staticmethod
    def on_addbox(inst, name, label, x, y, width, height):
        print(f'Adding box to list: {name}\n')
        box = BoxInfo()
        box.createBox(name, label, x, y, width, height)
        inst.add_box_to_list(box)
        
    @staticmethod
    def on_removebox(inst, name):
        print(f'Removing box from list: {name}\n')
        inst.remove_box_from_list(name)

    def __init__(self):
        GstBase.BaseTransform.__init__(self)
        self.boxlist = []
        self.connect("addbox", BasicOverlayPy.on_addbox)
        self.connect("removebox", BasicOverlayPy.on_removebox)

    @staticmethod
    def draw_box(inst, ctx, box):
        # draw
        x = float(box.x/inst.width)
        y = float(box.y/inst.height)
        width = float(box.width/inst.width)
        height = float(box.height/inst.height)

        print(f'{x},{y},{width},{height}')
        ctx.set_source_rgb(0, 0, 0)
        ctx.rectangle(x, y, width, height)
        ctx.set_line_width(0.005)
        ctx.stroke()
        
        ctx.move_to(x, y + height + 0.04)
        ctx.set_font_size(0.03)
        ctx.select_font_face("arial",
                     cairo.FONT_SLANT_NORMAL,
                     cairo.FONT_WEIGHT_NORMAL)
        ctx.show_text(box.label)
    
    def do_transform_ip(self, buf):
        result, mapinfo = buf.map(Gst.MapFlags.READ|Gst.MapFlags.WRITE)
        
        # create cairo surface
        surface = cairo.ImageSurface.create_for_data(mapinfo.data, cairo.Format.RGB24, self.width, self.height, cairo.Format.RGB24.stride_for_width(self.width))
        # create cairo context
        ctx= cairo.Context(surface)
        # cairo scale
        ctx.scale(self.width, self.height)
        for box in self.boxlist:
            BasicOverlayPy.draw_box(self, ctx, box)

        return Gst.FlowReturn.OK
    
    def do_set_caps(self, icaps, ocaps):
        ret, self.info = GstVideo.video_info_from_caps (icaps)
        print(str(self.info.width)+"x"+str(self.info.height))
        self.width = self.info.width
        self.height = self.info.height;
        return True

GObject.type_register(BasicOverlayPy)
__gstelementfactory__ = ("basicoverlaypy", Gst.Rank.NONE, BasicOverlayPy)
