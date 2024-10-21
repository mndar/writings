import websockets
import sys
import json
import asyncio
import logging
import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstSdp', '1.0')
gi.require_version('GstWebRTC', '1.0')
from gi.repository import GLib, Gst, GstSdp, GstWebRTC

class RepeatTimer:
    def __init__(self, timeout, callback):
        self._timeout = timeout
        self._callback = callback
        self._task = asyncio.ensure_future(self._job())

    async def _job(self):
        while True:
            await asyncio.sleep(self._timeout)
            await self._callback()

    def cancel(self):
        self._task.cancel()

class WebRTCClient:
    def __init__ (self, loop, sigserver, username, password):


        self.event_loop = loop
        self.sigserver = sigserver
        self.username = username
        self.password = password
        self.pipeline = Gst.parse_launch('videotestsrc ! sink. audiotestsrc ! sink. webrtcsink name=sink')
        self.webrtcsink = self.pipeline.get_by_name ('sink')
        self.signaller = self.webrtcsink.get_property('signaller')
        self.signaller.connect('start', self.signaller_on_start)
        self.signaller.connect('stop', self.signaller_on_stop)
        self.signaller.connect('send-session-description', self.signaller_on_send_session_description)
        self.signaller.connect('send-ice', self.signaller_on_send_ice)
        self.pipeline.set_state(Gst.State.PLAYING)

    async def send (self, msg):
        await self.conn.send (json.dumps(msg))

    def send_async(self, msg):
        asyncio.run_coroutine_threadsafe(self.send(msg), self.event_loop)

    async def send_ping(self):
        await self.conn.send('{"type":"ping"}')

    async def login(self):
        await self.send({"type":"login", "username":self.username, "password":self.password})
        self.ping_timer = RepeatTimer (1, self.send_ping)
    
    async def handle_json(self, message):
        try:
            msg = json.loads(message)
        except json.decoder.JSONDecoderError:
            print('Parsing Json message failed')
            raise
        print (msg)

        if msg['type'] == 'loginSuccess':
            logging.info ("Login Successful")
        elif msg['type'] == 'startSession':
            offer = None
            self.signaller.emit('session-requested', msg['sessionId'], msg['peerId'], offer)
        elif msg['type'] == 'peer':
            if 'sdp' in msg:
                sdp = msg['sdp']['sdp']
                session_id = msg['sessionId']
                res, sdp = GstSdp.SDPMessage.new_from_text(sdp)
                answer = GstWebRTC.WebRTCSessionDescription.new(GstWebRTC.WebRTCSDPType.ANSWER, sdp)
                self.signaller.emit('session-description', session_id, answer)
            elif 'ice' in msg:
                ice = msg['ice']
                sdp_m_line_index = ice['sdpMLineIndex']
                sdp_mid = None
                candidate = ice['candidate']
                session_id = msg['sessionId']
                self.signaller.emit('handle-ice', session_id, sdp_m_line_index, sdp_mid, candidate)
            else:
                logging.info('Unknown Peer Message')
            

    async def connect(self):
        print(f'Establishing connection to {self.sigserver}')
        self.conn = await websockets.connect(self.sigserver)
        await self.login()
        async for message in self.conn:
            await self.handle_json(message)
        self.close_pipeline()

    def signaller_on_start(self, _):
        logging.info ("WebRTCClient Start")
        asyncio.run_coroutine_threadsafe(self.connect(), self.event_loop)
        return True

    def signaller_on_stop(self, _):
         logging.info ("WebRTCClient Stop")
         return True

    #send SDP to signalling server
    def signaller_on_send_session_description(self, _, session_id, offer):
        logging.info ("WebRTCClient SDP Generated")
        sdp = offer.sdp.as_text()
        self.send_async({'type': 'peer', 'sessionId': session_id, 'sdp': { 'type': 'offer', 'sdp': sdp }})
        return True

    #Send ICE candidate to signalling server
    def signaller_on_send_ice(self, _, session_id, candidate, sdp_m_line_index, sdp_mid):
        self.send_async({'type': 'peer', 'sessionId': session_id, 'ice': {'candidate': candidate, 'sdpMLineIndex': sdp_m_line_index}})
        return True
        
if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO)
    
    Gst.init(None)
    if len(sys.argv) != 4:
        print ('Usage: ./pythonclient <signalling-server> <username> <password>')
        exit (1)

    sigserver = sys.argv[1]
    username = sys.argv[2]
    password = sys.argv[3]

    loop = asyncio.new_event_loop()
    client = WebRTCClient(loop, sigserver, username, password)
    res = loop.run_forever()