console.log("I have runned!");

let startTime_ = new Date().getTime();
let msgCounter_ = 0;

// import { Model } from "./data_model/fr_model.mjs";
// import { SysInterfacesEnum } from "./data_model/fr_model.mjs";

const  {Model, SysInterfacesEnum}  = require("./data_model/fr_model.mjs");

let model = new Model('127.0.0.1');
model.init();

model.on('system_status', function(res) {
    console.log(`System status chnaged to ${res}`);
});

function output(message) {
    var item = document.createElement('li');
    item.textContent = message;
    var messages = document.getElementById('messages');
    messages.appendChild(item);
    window.scrollTo(0, document.body.scrollHeight);
}

document.getElementById("model_load").addEventListener('click', function (e) {
    e.preventDefault();
    model.clear();

    model.load().then(result => {
        console.log(result)//        model.enablePeriodicCheck();
    }, error => {
        console.log(error);
    });
});

document.getElementById("get_devices").addEventListener('click', function (e) {
    e.preventDefault();

    let res = model.devices(SysInterfacesEnum.Can)
    console.log(res);

    res.forEach(function (item, i, arr) {
        var message = `device id = ${item.id}, name = ${item.name}, modules = ${item.modules.length}`;
        output(message);
    })

});

document.getElementById("get_device_header").addEventListener('click', function (e) {
    e.preventDefault();

    var deviceId = document.getElementById('get_device_id').value;
    let res = model.device(deviceId)
    console.log(res);

    var message = `device id = ${res.id}, name = ${res.name}, modules = ${res.modules.length}`;
    output(message);

    let modules = res.modules;

    modules.forEach(function (md, i, arr) {
        var message = `module id = ${md.id}, name = ${md.name}`;
        output(message);
    })
});

document.getElementById("get_module_header").addEventListener('click', function (e) {
    e.preventDefault();

    let deviceId = document.getElementById('get_module_deviceId').value;
    let moduleId = document.getElementById('get_module_id').value;

    let res = model.device(deviceId).module(moduleId)
    console.log(res);

    var module = res;
    var message = `module id = ${module.id}, name = ${module.name}, params = ${module.params.length}`;

    output(message);

    module.params.forEach(function (item) {
        var message = `param id = ${item.id}, name = ${item.name}`;
        output(message);
    })
});

document.getElementById("get_params").addEventListener('click', function (e) {
    e.preventDefault();

    let deviceId = document.getElementById('get_params_deviceId').value;
    let moduleId = document.getElementById('get_params_moduleId').value;

    if (deviceId == undefined || moduleId == undefined) {
        return response.sendStatus(400);
    }

    let res = model.device(deviceId).module(moduleId);
    console.log(res);

    let params = res.params;
    params.forEach(function (item, i, arr) {
        var message = `param id = ${item.id}, name = ${item.name}, desc = ${item.desc}`;
        output(message);
    })
});

document.getElementById("get_param_header").addEventListener('click', function (e) {
    e.preventDefault();

    let deviceId = document.getElementById('get_param_deviceId').value;
    let paramId = document.getElementById('get_param_id').value;

    let res = model.device(deviceId).param(paramId);
    console.log(res);

    var param = res;
    var message = `param id = ${param.id}, name = ${param.name}, desc = ${param.desc}, dev id = ${param.deviceId}, mod id = ${param.moduleId}`;

    output(message);
});

document.getElementById("get_param_data").addEventListener('click', function (e) {
    e.preventDefault();

    let deviceId = document.getElementById('get_param_data_deviceId').value;
    let paramId = document.getElementById('get_param_data_id').value;

    if (deviceId == undefined || paramId == undefined) {
        console.error("device id or param id is not valid!");
        return;
    }

    model.device(deviceId).param(paramId).lastValue().then(result => {
        console.log(result);
        var paramValue = result;

        var message = `param id = ${paramValue.paramId}, value = ${paramValue.value}, value format = ${paramValue.valueFormat}`;
        output(message);
    }, error => {
        console.error(error);
    });
});

document.getElementById("stream_param_data").addEventListener('click', async function (e) {
    e.preventDefault();

    startTime_ = new Date().getTime();
    console.time(`The stream elapsed time(${startTime_}):`);

    let deviceId = document.getElementById('get_param_data_deviceId').value;
    let paramId = document.getElementById('get_param_data_id').value;

    if (deviceId == undefined || paramId == undefined) {
        console.error("device id or param id is not valid!");
        return;
    }

    let startTime = new Date().getTime();
    console.time(`The front streaming elapsed time(${startTime}):`);

    let param = model.device(deviceId).param(paramId);
    let resStream = await param.openValueStream();

    resStream.on('data', chunk => {
        let stringifiedRes = chunk.toString();
        console.log(`Received from stream: ${stringifiedRes}`);

        var paramValue = JSON.parse(stringifiedRes);
        var message = `param id = ${paramValue.paramId}, value = ${paramValue.value}, value format = ${paramValue.valueFormat}`;
        output(message);

        msgCounter_++;
    });

    resStream.on('end', () => {
        console.log(`The front streaming is finished. Received ${msgCounter_} objects.`);
        console.timeEnd(`The front streaming elapsed time(${startTime}):`);
    });

    resStream.on('close', () => {
        console.log(`The front streaming is closed. `);
    });

    resStream.on('error', error => {
        console.log(`Error while value streaming: ${error}`);
        console.timeEnd(`The front streaming elapsed time(${startTime}):`);
    });
    
});
