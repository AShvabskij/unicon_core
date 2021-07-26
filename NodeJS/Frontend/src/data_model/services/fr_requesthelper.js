
let dataServerPort = 1235;
let host =  'localhost';
let socketUrl = "ws://" + host + ":" + dataServerPort; 

class _RequestHelper {
    static m_socket = new WebSocket(socketUrl);

    constructor()
    {
        this.m_socket = new WebSocket(socketUrl);

        this.m_socket.onopen = function(event) {
            console.log("Connected to data server");
        
        };
        
        this.m_socket.onerror = function(error) {
            console.log('Connection error: ' + error.message);
        };
        
        this.m_socket.onclose = function() {
            console.log('Connection closed.');
        };         
    }

    request(cmd) {
        let cmdStr = JSON.stringify(cmd);
        this.m_socket.send(cmdStr);
        console.log('sended cmd = ' + cmdStr);
        
        let promise = new Promise((resolve, reject) => {
    
            setTimeout(() => reject({status:500, msg:"Request time out"}), 1000)

            this.m_socket.onmessage = function(message) {
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