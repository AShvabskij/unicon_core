'use strict';

const Express = require('express');
const path = require('path');
const { createServer } = require('http');

const expressSrv = Express();
const jsonParser = Express.json();

const {Model, SysInterfacesEnum}  = require("./data_model/model.js");
let model = new Model('127.0.0.1');

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

expressSrv.post("/load", jsonParser, function (request, response) {
  console.log(request.body);

  if (!request.body) {
    return response.sendStatus(400);
  }

  model.clear();

  model.load().then(result => {
    response.json(result); 
  }, error => {
    console.log(error);
    response.json(error);
  });
});

expressSrv.get("/get_devices", jsonParser, function (request, response) {
  console.log(request.body);

  if (!request.body) {
    return response.sendStatus(400);
  }

  let res = model.devices(SysInterfacesEnum.Can)
  response.json(res);
});

expressSrv.post("/get_device_header", jsonParser, function (request, response) {
  console.log(request.body);

  if (!request.body) {
    return response.sendStatus(400);
  }

  let res = model.device(request.body.deviceId)
  response.json(res);
});

expressSrv.post("/get_module_header", jsonParser, function (request, response) {
  console.log(request.body);

  if (!request.body) {
    return response.sendStatus(400);
  }

  let res = model.device(request.body.deviceId).module(request.body.moduleId)
  response.json(res);
});

expressSrv.post("/get_params", jsonParser, function (request, response) {
  console.log(request.body);

  let deviceId = request.body.deviceId;
  let moduleId = request.body.moduleId;

  if (deviceId == undefined || moduleId == undefined) {
    return response.sendStatus(400);
  }

  let module = model.device(deviceId).module(moduleId);
  let res = module.params;
  response.json(res);
});

expressSrv.post("/get_param_header", jsonParser, function (request, response) {
  console.log(request.body);

  let deviceId = request.body.deviceId;
  let paramId = request.body.paramId;

  if (deviceId == undefined || paramId == undefined) {
    return response.sendStatus(400);
  }

  let res = model.device(deviceId).param(paramId);
  response.json(res);
});

expressSrv.post("/get_param_data", jsonParser, function (request, response) {
  console.log(request.body);

  let deviceId = request.body.deviceId;
  let paramId = request.body.paramId;

  if (deviceId == undefined || paramId == undefined) {
    return response.sendStatus(400);
  }

  model.device(deviceId).param(paramId).lastValue().then(result => {
    response.json(result);
  }, error => {
    return response.status(500).send(error);
  });
});

expressSrv.post("/stream_param_data", jsonParser, async function (request, response) {
  console.log(request.body);

  let deviceId = request.body.deviceId;
  let paramId = request.body.paramId;

  if (deviceId == undefined || paramId == undefined) {
    return response.sendStatus(400);
  }

  let param = model.device(deviceId).param(paramId);
  let resStream = await param.openValueStream();

  resStream.on('data', chunk => {
    let stringifiedRes = chunk.toString();
    // console.log(`Received from stream: ${stringifiedRes}`);

    if (frontWebSocket) {
      frontWebSocket.send(stringifiedRes);
    }
  });

// resStream.pipe(response);
  return response.sendStatus(200);  
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