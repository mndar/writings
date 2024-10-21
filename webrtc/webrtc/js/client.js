var connection = new WebSocket('ws://<EDIT-Signalling-Server-IP>:8443');  
var loginName = "";

var loginInput = document.querySelector('#loginInput'); 
var loginBtn = document.querySelector('#loginBtn'); 
var connectedUser, myConnection;
var peer_connection;
var session_id;
var rtc_configuration = {iceServers: [{urls: "stun:stun.services.mozilla.com"},
                                      {urls: "stun:stun.l.google.com:19302"},
                                      {
                                          'urls': 'turn:<EDIT-TURN-Server-IP>:3478',
                                          'username': 'turn',
                                          'credential': 'turn456',
                                      }]};

//when a user clicks the login button 
loginBtn.addEventListener("click", function(event){ 

   loginName = loginInput.value; 
	
   if(loginName.length > 0){ 
      send({type: "login", username: loginName, password: loginName}); 
   }
});

function setError(text) {
   console.error(text);
}
  
// Local description set, send it to peer
function onLocalDescription(desc) {
   console.log("Got local description: " + JSON.stringify(desc));
   peer_connection.setLocalDescription(desc).then(function() {
       console.log("Sending SDP " + desc.type);
       reply = {type: 'peer', sessionId: session_id, sdp: peer_connection.localDescription};
       console.log (JSON.stringify(reply))
       send(reply);
   });
}

// SDP offer received from peer, set remote description and create an answer
function onIncomingSDP(sdp) {
   peer_connection.setRemoteDescription(sdp).then(() => {
       console.log("Remote SDP set");
       if (sdp.type != "offer")
           return;
       console.log("Got SDP offer");
       peer_connection.createAnswer()
           .then(onLocalDescription).catch(setError);
   }).catch(setError);
}

// ICE candidate received from peer, add it to the peer connection
function onIncomingICE(ice) {
   var candidate = new RTCIceCandidate(ice);
   peer_connection.addIceCandidate(candidate).catch(setError);
}

function getVideoElement() {
   return document.getElementById("stream");
}

function onRemoteTrack(event) {
   if (getVideoElement().srcObject !== event.streams[0]) {
       console.log('Incoming stream');
       getVideoElement().srcObject = event.streams[0];
   }
}

function sendPing() {
   send({type:"ping"})
}

//handle messages from the server 
connection.onmessage = function (message) { 
   console.log("Got message", message.data);
   var data = JSON.parse(message.data); 
   switch(data.type) {
      case "loginSuccess":
         console.log("Login Successful");
         setInterval (sendPing, 1000);
         send({type:"list"})
         break;
      case "userList":
         console.log("Received Client List");
         data.producers.forEach(element => {
            addToClientList(element)
         }); 
         break;
      case "peer":
         console.log("Got Peer message for session: " + data.sessionId);
         if (data.sdp) {
            if (data.sdp.type === "offer")
               session_id = data.sessionId;
               onIncomingSDP(data.sdp);
         } 
         else if (data.ice) {
            onIncomingICE(data.ice);
         }
         break;
      default:
         console.log("Unknown Message");
   }
}

connection.onopen = function () { 
   console.log("Connected"); 
};
   
 connection.onerror = function (err) { 
   console.log("Got error", err); 
};
   
 // Alias for sending messages in JSON format 
 function send(message) { 
   connection.send(JSON.stringify(message)); 
 };

 function addToClientList(name) {
   var list = document.getElementById('clientList');
   var entry = document.createElement('li');
   var button = document.createElement('button');
   button.innerHTML = "Stream Client";
   
   button.addEventListener("click", function(event) {
      startSesssion(name);
   });

   entry.appendChild(document.createTextNode(name));
   entry.appendChild(document.createTextNode("    "));
   entry.appendChild(button)

   list.appendChild(entry);
 }

 function startSesssion(name) {
   peer_connection = new RTCPeerConnection(rtc_configuration);
   peer_connection.onicecandidate = (event) => {
      if (event.candidate == null) {
               console.log("ICE Candidate was null, done");
               return;
      }
      send({type:"peer", sessionId: session_id, ice: event.candidate});
   }
   peer_connection.ontrack = onRemoteTrack;

   session_id = crypto.randomUUID();
   send({type:"startSession", peerId:name, sessionId:session_id });
 }