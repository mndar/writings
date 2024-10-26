import os
import json
import socket
import uuid
import mmap
from io import BytesIO

from threading import Thread

from plugins import yolo as Yolo
from plugins import mobilenet as Mobilenet

class PyInferServer:
    def __init__ (self, socket_path, plugin_dir):
        self.plugin_dir = plugin_dir
        self.sessions = {}
        self.extra_data = {}
        self.mmap = {}
        if os.path.exists(socket_path):
            os.unlink(socket_path)

        #init Models
        self.yolo = Yolo.YoloInference()
        self.mobilenet  = Mobilenet.MobilenetInference()

        #start Unix socket server at /tmp/pyinfer.sock
        s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        s.bind(socket_path)
        s.listen(5)
        while True:
            c, addr = s.accept()     # Establish connection with client.
            thread = Thread(target=self.on_new_client, args=(c,addr,))
            thread.start()
        s.close()
        
    def on_new_client(self, client_socket, addr):
        #init client
        self.sessions[str(client_socket)] = str(uuid.uuid4())
        while True:
            data = client_socket.recv(1024).decode('utf-8')
            if not data:
                break
            #print(f"{addr} >> {data}")
            self.handle_json (client_socket, json.loads(data))
        print (f"Client Disconnect: {str(client_socket)}")
        identifier = str(client_socket)
        del self.sessions[identifier]
        del self.extra_data[identifier]
        del self.mmap[identifier]
        client_socket.close()

    def handle_json (self, client_socket, json_msg):
        if json_msg["type"] == "login":
            reply = { 'type':'welcome', 'sessionId': self.sessions[str(client_socket)] }
            self.send(client_socket, reply)
        elif json_msg["type"] == "infer":

            (stream_type, model, width, height) = self.extra_data[str(client_socket)]
            #read from mmap
            if stream_type == 1: #Only Video Supported
                buf_bytearray = self.read_mmap (client_socket, height*width*3)
                (stream_type, model, width, height) = self.extra_data[str(client_socket)]
                if model == "yolo":
                    result = self.yolo.infer (buf_bytearray, self.extra_data[str(client_socket)])
                elif model == "mobilenet":
                    result = self.mobilenet.infer (buf_bytearray, self.extra_data[str(client_socket)])
                else:
                    result = {}
            else:
                result = {}
                
            
            reply= { 'type':'inference', 'sessionId': json_msg['sessionId'], 'result':result}
            self.send(client_socket, reply)

        elif json_msg["type"] == "extraData":
            #read extra data for session.
            #in case of video contains, width and height
            #TODO: Command extra_Data for all streams. set tuple fields to empty default values where not applicable
            stream_type = json_msg["extraData"]["streamType"]
            if stream_type == 1: #Video Stream
                width = json_msg["extraData"]["width"]
                height = json_msg["extraData"]["height"]
                model = json_msg["extraData"]["model"]
                self.extra_data[str(client_socket)] = (stream_type, model, width, height)
            else:
                print ("ERROR: Unsupported Stream")
                client_socket.close()

    def read_mmap(self, client_socket, len):

        #init client. TODO: move this to after receiving memory map confirmation from client
        mmap_path = '/tmp/' + self.sessions[str(client_socket)]
        
        #print (f"Reading {len} from memory map {mmap_path}")
        f = open (mmap_path, "r+b")
        mm = mmap.mmap(f.fileno(), 0)
        self.mmap[str(client_socket)] = mm

        # read mmap
        mm = self.mmap[str(client_socket)]
        buf_bytearray = BytesIO()

        buf_bytearray.write(mm.read(len))
        
        buf_bytearray.seek(0)

        f.close()
        return buf_bytearray

    def send (self, client_socket, msg):
        utf8 = (json.dumps(msg)+'\n').encode('utf-8')
        client_socket.send(utf8)

if __name__ == "__main__":
    PyInferServer (socket_path='/tmp/pyinfer.sock', plugin_dir="plugins/")