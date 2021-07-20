'use strict';

import React, { useState, useCallback, useMemo, useRef } from 'react';
import useWebSocket, { ReadyState } from 'react-use-websocket';

// const WebSocket = require('faye-websocket');
const Events = require("events");

//static let socket = new WebSocket.Client(socketUrl);

var sendingMessage = "";

const WebSocketDemo = () => {
    let dataServerPort = 1235;
    let host =  '192.168.7.111';
    let socketUrlCustom = "ws://" + host + ":" + dataServerPort;

    const [socketUrl, setSocketUrl] = useState(socketUrlCustom);
    const messageHistory = useRef([]);

    const {
        sendMessage,
        lastMessage,
        readyState,
    } = useWebSocket(socketUrl);

    messageHistory.current = useMemo(() => {
        messageHistory.current.concat(lastMessage),[lastMessage];
        // var messageData = JSON.parse(lastMessage);
        // RequestHelper.m_events.emit(messageData.request_id, messageData);
    });

    // const handleClickChangeSocketUrl = useCallback(() =>
    //     setSocketUrl('wss://demos.kaazing.com/echo'), []);

    const handleSendMessage = useCallback(() =>
        sendMessage(sendingMessage), []);

    const connectionStatus = {
        [ReadyState.CONNECTING]: 'Connecting',
        [ReadyState.OPEN]: 'Open',
        [ReadyState.CLOSING]: 'Closing',
        [ReadyState.CLOSED]: 'Closed',
        [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
    }[readyState];
};


class _RequestHelper {
    static m_events = new Events();

    constructor()
    {
        this.m_events = new Events();
        this.m_socket = new WebSocketDemo();
        

        /* socket.on('open', function(event) {
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
        });          */
    }

    request(cmd) {
        let cmdStr = JSON.stringify(cmd);
        sendingMessage = cmdStr;
        this.m_socket.handleSendMessage();
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


