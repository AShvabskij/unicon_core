import Config from './../../.config.js';
const Events = require('events');

let DATA_SERVER_PORT = 1235;

export class _RequestHelper {

    m_connected = false;
    m_socketUrl = "ws://" + Config.ip + ":" + DATA_SERVER_PORT;
    m_events = new Events();

    constructor() {
    }

    initConnection(srvHost) {
        if (this.m_connected) {
            return;
        }

        if (srvHost !== undefined) {
            this.m_socketUrl = "ws://" + srvHost + ":" + DATA_SERVER_PORT;
        }

        this.m_socket = new WebSocket(this.m_socketUrl);

        this.m_socket.onopen = (event) => {
            console.log(`Connected to cmd socket ${this.m_socket.url}`);
            this.m_connected = true;
        };

        this.m_socket.onerror = (error) => {
            console.log('Connection error: ' + error.message);
        };

        this.m_socket.onclose = () => {
            console.log('Connection closed.');
            
            this.m_connected = false;
            this.m_socket = null;

            setTimeout(async () => {
                this.initConnection();
            }, 7000);
        };

        this.m_socket.onmessage = (message) => {
            var messageData = JSON.parse(message.data);
            if (messageData.request_id === undefined) {
                console.log(`Inappropriate message is recevied: = ${messageData}`);
            }

            this.m_events.emit(messageData.request_id, messageData);
        }
    }

    request(cmd, timeout = 1000) {

        let promise = new Promise((resolve, reject) => {

            if (!this.m_connected) {
                reject({ status: 500, msg: "The web socket is not connected now." });
            }

            let cmdStr = JSON.stringify(cmd);
            this.m_socket.send(cmdStr);
            console.log('sended cmd = ' + cmdStr);
        
            setTimeout(() => reject({ status: 500, msg: `Request time out for cmd = ${cmdStr}` }), timeout)

            this.m_events.once(cmd.request_id, (data) => {
                console.log("Received data: " + JSON.stringify(data));

                let res = data.body
                if (res === undefined) {
                    reject({ status: 500, msg: `Inappropriate response is recevied for the cmd = ${cmdStr}` });
                }

                res.status = 200; // ok
                resolve(res);
            });
        });

        return promise;
    };
}

export const RequestHelper = new _RequestHelper();
// module.exports = RequestHelper;