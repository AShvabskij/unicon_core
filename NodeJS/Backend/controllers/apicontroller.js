const WebSocket = require('faye-websocket');
const {ParamProvider}  = require("../services/paramprovider.js");
const {DeviceProvider}  = require("../services/deviceprovider.js");

let streamServerPort = 1237;
let host =  'localhost';
let streamSocketUrl = "ws://" + host + ":" + streamServerPort; 
let streamSocket = new WebSocket.Client('ws://localhost:9999');

let destWebSocket = new WebSocket.Client('ws://');

let paramProvider = new ParamProvider();
let deviceProvider = new DeviceProvider();

exports.setDestWebSocket = function (frontSocket) {
    destWebSocket = frontSocket;
}

exports.getParam = function(req, res) {
    if (isNaN(req.params.id)) {
        next('route');
    }
    let paramId = parseInt(req.params.id);
    let deviceId = parseInt(req.query.device_id);
    let moduleId = parseInt(req.query.module_id);

    paramProvider.reqParam(deviceId, moduleId, paramId)
        .then(result => {
        console.log(result);
        res.send(result);
    },
    error => {
        console.log(error);
        res.status(error.status).send({error: error.msg})
    });
};

exports.getParams = function(req, res) {

    let deviceId = parseInt(req.query.device_id);
    let moduleId = parseInt(req.query.module_id);

    paramProvider.reqParam(deviceId, moduleId).then(result => {
        console.log(result);
        res.send(result);
    },
    error => {
        console.log(error);
        res.status(error.status).send({error: error.msg})
    });
};

exports.getParamData = function(req, res) {

    let paramId = parseInt(req.params.id);
    let deviceId = parseInt(req.query.device_id);
    let moduleId = parseInt(req.query.module_id);

    let stream = undefined;
    if ("stream" in req.query) {
        streamSocket = new WebSocket.Client(streamSocketUrl);
        stream = req.query.stream;
    }

    let promise = paramProvider.reqParamValue(deviceId, moduleId, paramId, stream);

    promise.then(result => {
        console.log(result);
        res.send(result);
    }, 
    error => {
        console.log(error);
        res.status(error.status).send({error: error.msg})
    });
};

exports.getDevice = function(req, res, next) {
    if (isNaN(req.params.id)) {
        next('route');
    }

    let deviceId = parseInt(req.params.id);
    let moduleId = parseInt(req.query.module_id);

    deviceProvider.requestDevice(deviceId, moduleId)
    .then(result => {
        res.send(result);
    }, error => {
        res.status(error.status).send({error: error.msg})
    });
};

//-------------------------------------------------------------------------
// Working with data stream sockets
streamSocket.on('open', function(event) {
    console.log("Connected to stream data server");

    streamSocket.on('message', function(message) {
        var messageData = JSON.parse(message.data);
        
        var strObj = JSON.stringify(messageData.body);
        console.log("Recevied on stream socket: " + strObj);

        if (destWebSocket) {
            destWebSocket.send(strObj);
        }
    });
});

streamSocket.on('error', function(error) {
    console.log('Stream error: ' + error.message);
//  process.exit(1);
});

streamSocket.on('close', function() {
    console.log('Stream closed.');
//  process.exit(1);
});
