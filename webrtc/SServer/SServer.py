#!/bin/python
import asyncio
import websockets
import websockets.server
import json
import logging
import sys

import Client
import Backend
from threading import Timer
        
class SServer:
    def __init__(self):
        self.backend = Backend.Backend()
        self.client_list = []
        self.sessions = {}
        self.ping_timer = None

    def getClientList(self):
        client_username_list = []
        for client in self.client_list:
            client_username_list.append(client.getUsername())
        return client_username_list

    def ping_timeout(self, client):
        logging.info(f"Clearing Client: {client.getUsername()}")
        self.client_cleanup(client.websocket)

    async def handleJson(self, message, websocket):
        logging.info(f"Received: {message}")
        json_msg = None
        try:
            json_msg = json.loads(message)
            msg_type = json_msg["type"]
            client = self.find_client_using_websocket (websocket)
            if msg_type == "login":
                username = json_msg["username"]
                password = json_msg["password"]
                if self.backend.verify_login(username, password):
                    logging.info("Login Successful")
                    
                    #create new client and add to list
                    client  = Client.Client(username)
                    client.websocket = websocket
                    self.client_list.append(client)

                    #start ping timer
                    client.ping_timer = Timer (5, self.ping_timeout, args=[client,])
                    client.ping_timer.start()

                    reply_msg = {"type":"loginSuccess", "peerId": client.getPeerId()}
                    logging.info(json.dumps(reply_msg))
                    await websocket.send(json.dumps(reply_msg))
            elif msg_type == "ping":
                client = self.find_client_using_websocket (websocket)
                #restart ping timer
                client.ping_timer.cancel()
                client.ping_timer = Timer (5, self.ping_timeout, args=[client,])
                client.ping_timer.start()
            elif msg_type == "list":
                reply_msg = {"type":"userList", "producers": self.getClientList()}
                logging.info(json.dumps(reply_msg))
                await websocket.send(json.dumps(reply_msg))
            elif msg_type == "startSession":
                other_client_id = json_msg["peerId"]
                client = self.find_client_using_websocket (websocket)
                other_client = self.find_client_using_peer_id(other_client_id)
                if self.backend.verify_access(client, other_client):
                    logging.info(f"Starting Session Between {client.getUsername()} and {other_client.getUsername()}")
                else:
                    return
                
                sessionId = json_msg["sessionId"]
                client.sessionids.append(sessionId)
                other_client.sessionids.append(sessionId)
                
                reply_msg = {"type":"startSession", "peerId": client.getPeerId(), "sessionId": sessionId}
                self.sessions[sessionId] = [client, other_client]
                await other_client.send(reply_msg)
            elif msg_type == "peer":
                for key, value in json_msg.items() :
                    print (key, value)
                if "sdp" in json_msg:
                    if json_msg["sdp"]["type"] == "offer":
                        session_id = json_msg["sessionId"]
                        other_client = self.get_session_peer(session_id, websocket)
                        logging.info(f"Sending SDP Offer to Session Peer {other_client.getUsername()}")
                        await other_client.send(json_msg)
                    elif json_msg["sdp"]["type"] == "answer":
                        session_id = json_msg["sessionId"]
                        other_client = self.get_session_peer(session_id, websocket)
                        logging.info(f"Sending SDP Answer to Session Peer {other_client.getUsername()}")
                        await other_client.send(json_msg)
                elif "ice" in json_msg:
                    session_id = json_msg["sessionId"]
                    other_client = self.get_session_peer(session_id, websocket)
                    await other_client.send(json_msg)

                
            else:
                logging.info("Unknown message from peer")
        except json.decoder.JSONDecodeError:
            logging.info("Error decoding Json")
        finally:
            pass

    def get_session_peer (self, session_id, websocket):
        clients = self.sessions[session_id]
        for client in clients:
            if client.websocket != websocket:
                return client

    def find_client_using_websocket(self, websocket):
        print(websocket)
        for x in self.client_list:
            print(x.websocket)
            if x.websocket == websocket:
                return x
        return None
    
    def find_client_using_peer_id(self, peer_id):
        for x in self.client_list:
            if x.peer_id == peer_id:
                return x
        return None
    
    async def handleMessage(self, websocket):
        try:
            async for message in websocket:
                await self.handleJson(message, websocket)
        except Exception as e:
             print(f"Websockets Exception: {e.__class__.__name__, e}")
             self.client_cleanup(websocket)

    def client_cleanup(self, websocket):
        client = self.find_client_using_websocket (websocket)
        if client:
            print(f"Removing Client: {client.getUsername()}")
            self.client_list.remove(client)

        #clear sesssions
        print(f"Clearing Sessions: {client.sessionids}")
        for id in client.sessionids:
            del self.sessions[id]

    async def serve(self, port = 8443):
        async with websockets.server.serve(self.handleMessage, None, port):
            await asyncio.get_running_loop().create_future()  # run forever

async def main():
        sserver = SServer()
        await sserver.serve()
        
if __name__ == "__main__":
    logging.basicConfig(stream=sys.stdout, level=logging.INFO)
    asyncio.run(main())
