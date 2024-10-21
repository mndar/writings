#!/bin/python
import uuid
import asyncio
import json

class Client:
    def __init__(self, username):
        self.username = username
        self.peer_id = username
        self.websocket = None
        self.sessionids = []
        self.ping_timer = None

    def set_username(self, username):
        self.username = username
    
    def get_uuid(self):
        return self.peer_id
    
    def getUsername(self):
        return self.username

    def getPeerId(self):
        return self.peer_id
    
    async def send(self, message):
        await self.websocket.send(json.dumps(message))
