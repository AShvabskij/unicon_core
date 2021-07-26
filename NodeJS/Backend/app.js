'use strict';

const Express = require('express');
const path = require('path');
const { createServer } = require('http');

const expressSrv = Express();
const jsonParser = Express.json();

const apiController = require("./controllers/apicontroller.js");

let listenPort = process.argv[2] || 7000;
const WebSocket = require('faye-websocket');

const { stringify } = require('querystring');

expressSrv.use(Express.static(path.join(__dirname, '/public')));
expressSrv.use(Express.static(path.join(__dirname, '/node_modules')));

expressSrv.get("/api", function (request, response) {

    response.sendFile(__dirname + "/public/demo_api.html");
});

expressSrv.get("/model", function (request, response) {

  response.sendFile(__dirname + "/public/demo_model.html");
});


const server = createServer(expressSrv);

server.on('upgrade', function(request, socket, body) {
  initFrontWebSocket(request, socket, body);
  apiController.setDestWebSocket(frontWebSocket);
});

server.listen(listenPort);
//----------------------------------------------------------

const paramRouter = Express.Router();
const deviceRouter = Express.Router();

expressSrv.use("/api/params", paramRouter)  
expressSrv.use("/api/devices", deviceRouter)  

// api/params/id?device=id[&module=id]
paramRouter.get("/:id", apiController.getParam);

// api/params?device=id[&module=id]
paramRouter.get("", apiController.getParams);

// api/params/data/id?device=id[&stream=on|off]
paramRouter.get("/data/:id", apiController.getParamData);

// api/devices/[id]?[module=id]
deviceRouter.get("/:id", apiController.getDevice);

let frontWebSocket = new WebSocket.Client("ws://");

function initFrontWebSocket (request, socket, body) {
  if (WebSocket.isWebSocket(request)) {
      frontWebSocket = new WebSocket(request, socket, body);
      console.log('Front Websocket is inited and connected. ');
    
    frontWebSocket.on('close', function(event) {
      if (event.wasClean) {
          console.log(`[close] The connection is closed, code=${event.code} reason=${event.reason}`);
        } else {
          // например, сервер убил процесс или сеть недоступна
          // обычно в этом случае event.code 1006
          console.log(`[close] The connecton is broken, code=${event.code} reason=${event.reason}`);
        }

      console.log(`Front Websocket is closed, code = ${event.code} reason = ${event.reason}`);
      frontWebSocket = null;
    });

    frontWebSocket.on('error', function(error) {
      console.log(`[error] ${error.message}`);
    });
  }
}

/*
Model.events.on('stream', function(value) {
  if (value.value == -1) {
    return;
  }
  
  if (frontWebSocket) {
    frontWebSocket.send(JSON.stringify(value));
  }
});
*/