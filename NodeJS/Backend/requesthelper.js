'use strict';

const WebSocket = require('faye-websocket');
const Events = require("events");

let dataServerPort = 1235;
let host =  'localhost';
let socketUrl = "ws://" + host + ":" + dataServerPort; 

let socket = new WebSocket.Client(socketUrl);

//static let socket = new WebSocket.Client(socketUrl);

class _RequestHelper {
    static m_events = new Events();

    constructor()
    {
        this.m_events = new Events();

        socket.on('open', function(event) {
            console.log("Connected to data server");
        
        });
        
        socket.on('message', function(message) {
            var messageData = JSON.parse(message.data);
            
            RequestHelper.m_events.emit(messageData.request_id, messageData);
        });

        socket.on('error', function(error) {
            console.log('Connection error: ' + error.message);
            process.exit(1);
        });
        
        socket.on('close', function() {
            console.log('Connection closed.');
            process.exit(1);
        });         
    }

    request(cmd) {
        let cmdStr = JSON.stringify(cmd);
        socket.send(cmdStr);
        console.log('sended cmd = ' + cmdStr);
        
        let promise = new Promise((resolve, reject) => {
    
            setTimeout(() => reject({status:500, msg:"Request time out"}), 1000)
    
            this.m_events.once(cmd.request_id, function(response) {
                let res = response.body
//              console.log("Received data: " + JSON.stringify(res));

                res.status = 200; // ok
                resolve(res);
            });
        });
    
        return promise;
    };
}

const RequestHelper = new _RequestHelper();
module.exports = RequestHelper;