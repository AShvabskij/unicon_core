var backendPort = 7000;
var host = "127.0.0.1"; // location.hostname

var dataSocketUrl = "ws://" + host + ":" + backendPort;  
var dataSocket = new WebSocket(dataSocketUrl);

function output(message) {
    var item = document.createElement('li');
    item.textContent = message;
    var messages = document.getElementById('messages');
    messages.appendChild(item);
    window.scrollTo(0, document.body.scrollHeight);
}

document.getElementById("api_params_request").addEventListener('click', function(e) {
    e.preventDefault();

    let deviceId = document.getElementById('api_params_deviceId').value;
    let moduleId = document.getElementById('api_params_moduleId').value;
    let paramId = undefined;

    getParamsInfo(deviceId, moduleId, paramId);
});

document.getElementById("api_param_request").addEventListener('click', function(e) {
    e.preventDefault();

    let elem = document.getElementById('api_params_deviceId');
    let deviceId = document.getElementById('api_params_deviceId').value;
    let moduleId = undefined;
    let paramId = document.getElementById('api_param_id').value;

    getParamsInfo(deviceId, moduleId, paramId);
});

async function getParamsInfo(deviceId, moduleId, paramId) {

    let reqUri = "/api/params";
    if (paramId) {
        reqUri += "/" + paramId;
    }

    if (deviceId) {
        reqUri += "?device_id=" + deviceId
    }

    if (moduleId) {
        reqUri += "&module_id=" + moduleId;
    }

    console.log(reqUri);

    let response = await fetch(reqUri, {
        method: "GET",
        headers: {
        "Content-Type": "application/json; charset=utf-8"
        }
    });

    let params = await response.json();
    console.log(params);

    params.forEach(function(item, i, arr){
        var message = `param_id = ${item.param_id}, name = ${item.name}`;
        output(message);    
    })
}

document.getElementById("api_param_data_request").addEventListener('click', function(e) {
    e.preventDefault();

    var deviceId = document.getElementById('api_param_data_deviceId').value;
    var paramId = document.getElementById('api_param_data_id').value;
    var stream = document.getElementById('api_param_data_stream').value;

    getParamValue(deviceId, paramId, stream);
});

async function getParamValue(deviceId, paramId, stream) {
    if (!deviceId || !paramId) return;

    let reqUri = "/api/params/data/" + paramId  + "?device_id=" + deviceId;
    if (stream) {
        reqUri += "&stream=" + stream;
    }
    
    console.log(reqUri);

    let response = await fetch(reqUri, {
        method: "GET",
        headers: {
        "Content-Type": "application/json; charset=utf-8"
        }
    });

    if (!response.ok) {
        return;
    }

    let param = await response.json();
    console.log(param);

    if (!param) {
        return;
    }

    if ("error" in param) {
        return;
    }

    if (!stream) {
        var message = `device_id = ${param.device_id}, param_id = ${param.param_id}, value = ${param.value.value}`;
        output(message);    
    }
}

document.getElementById("api_param_data_stop").addEventListener('click', function(e) {
    e.preventDefault();

    var deviceId = document.getElementById('api_param_data_deviceId').value;
    var paramId = document.getElementById('api_param_data_id').value;

    stopParamValue(deviceId, paramId);
});

async function stopParamValue(deviceId, paramId) {
    if (!deviceId || !paramId) return;

    let reqUri = "/api/params/data/" + paramId  + "?device_id=" + deviceId + "&stream=" + "off";
    console.log(reqUri);

    let response = await fetch(reqUri, {
        method: "GET",
        headers: {
        "Content-Type": "application/json; charset=utf-8"
        }
    });

    let param = await response.json();
    console.log(param);
}

document.getElementById("api_device_request").addEventListener('click', function(e) {
    e.preventDefault();

    var deviceId = document.getElementById('api_device_id').value;

    getDeviceInfo(deviceId);
});

document.getElementById("api_devices_request").addEventListener('click', function(e) {
    e.preventDefault();

    getDeviceInfo();
});

document.getElementById("api_module_request").addEventListener('click', function(e) {
    e.preventDefault();

    var deviceId = document.getElementById('api_module_deviceId').value;
    var moduleId = document.getElementById('api_module_id').value;

    getDeviceInfo(deviceId, moduleId);
});

async function getDeviceInfo(deviceId, moduleId) {
    let reqUri = "/api/devices";

    if (deviceId != null && deviceId != undefined) {
        reqUri += "/"+ deviceId;
    }
     
    let isModule = (moduleId != null && moduleId != undefined)
    if (isModule) {
        reqUri += "?module_id=" + moduleId;
    }
    
    console.log(reqUri);

    let response = await fetch(reqUri, {
        method: "GET",
        headers: {
        "Content-Type": "application/json; charset=utf-8"
        }
    });

    let res = await response.json();
    console.log(res);
    let device = !isModule ? res : null
    let module = isModule ? res : null

    if (device) {
        device.forEach(function(item, i, arr) {
            let modules = item.modules;

            modules.forEach(function(md, i, arr){
                var message = `device name = ${item.name}, module_id = ${md}`;
                output(message);    
            })
        })
    }

    if (module) {
        module.params.forEach(function(param, i, arr){
            var message = `module name = ${module.name}, param_id = ${param}`;
            output(message);    
        })
    }
}

//------------------------------------------------------------------------------------
dataSocket.onclose = function() {
    console.error("web channel closed");
};

dataSocket.onerror = function(error) {
    console.error("web channel error: " + error);
};

dataSocket.onopen = function() {
    output("WebSocket connected");
    dataSocket.onmessage = function(message) {
        
        if (message.data === null) {
            return;
        }

      var obj = JSON.parse(message.data);
      var message = `device_id = ${obj.device_id}, param_id = ${obj.param_id}, value = ${obj.value.value}`;

      output(message);
      console.log("data received: " + obj);
    };
}