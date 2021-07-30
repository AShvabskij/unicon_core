
let DATA_SERVER_PORT = 1235;

class _RequestHelper {

    m_connected = false;
    m_socketUrl = "ws://" + "127.0.0.1" + ":" + DATA_SERVER_PORT;
    m_socket = new WebSocket(this.m_socketUrl);

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
            }, 5000);
        };

        this.m_connected = true;        
    }

    request(cmd) {

        let promise = new Promise((resolve, reject) => {

            if (!this.m_connected) {
                reject({ status: 500, msg: "The web socket is not connected now." });
            }

            let cmdStr = JSON.stringify(cmd);
            this.m_socket.send(cmdStr);
            console.log('sended cmd = ' + cmdStr);

            setTimeout(() => reject({ status: 500, msg: "Request time out" }), 1000)

            this.m_socket.onmessage = (message) => {
                var messageData = JSON.parse(message.data);
                if (messageData.request_id !== cmd.request_id) {
                    return;
                }

                console.log("Received data: " + JSON.stringify(messageData));
                let res = messageData.body

                res.status = 200; // ok
                resolve(res);
            };
        });

        return promise;
    };
}

const RequestHelper = new _RequestHelper();
module.exports = RequestHelper;