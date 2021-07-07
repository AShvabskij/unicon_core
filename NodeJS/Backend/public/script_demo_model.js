console.log("I have runned!");

var backendPort = 7000;
var host = "192.168.1.1"; // location.hostname

var dataSocketUrl = "ws://" + host + ":" + backendPort;
var dataSocket = new WebSocket(dataSocketUrl);

let startTime_ = new Date().getTime();
let msgCounter_ = 0;

function output(message) {
    var item = document.createElement('li');
    item.textContent = message;
    var messages = document.getElementById('messages');
    messages.appendChild(item);
    window.scrollTo(0, document.body.scrollHeight);
}

document.getElementById("model_load").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/load", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let res = JSON.parse(request.response);
        console.log(res);
    });
    request.send();
});

document.getElementById("get_devices").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("GET", "/get_devices", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let res = JSON.parse(request.response);
        console.log(res);

        res.forEach(function (item, i, arr) {
            var message = `device id = ${item.id}, name = ${item.name}, modules = ${item.modules.length}`;
            output(message);
        })

    });
    request.send();
});

document.getElementById("get_device_header").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/get_device_header", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let res = JSON.parse(request.response);
        console.log(res);

        var message = `device id = ${res.id}, name = ${res.name}, modules = ${res.modules.length}`;
        output(message);

        let modules = res.modules;

        modules.forEach(function (md, i, arr) {
            var message = `module id = ${md.id}, name = ${md.name}`;
            output(message);
        })
    });

    var deviceId = document.getElementById('get_device_id').value;
    let reqParams = { deviceId }
    let sendStr = JSON.stringify(reqParams)

    request.send(sendStr);
});

document.getElementById("get_module_header").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/get_module_header", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let res = JSON.parse(request.response);
        console.log(res);
        var module = res;

        var message = `module id = ${module.id}, name = ${module.name}, params = ${module.params.length}`;
        output(message);

        module.params.forEach(function (item) {
            var message = `param id = ${item.id}, name = ${item.name}`;
            output(message);
        })
    });

    let deviceId = document.getElementById('get_module_deviceId').value;
    let moduleId = document.getElementById('get_module_id').value;
    let reqParams = { deviceId, moduleId }
    let sendStr = JSON.stringify(reqParams)

    request.send(sendStr);
});

document.getElementById("get_params").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/get_params", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let params = JSON.parse(request.response);
        console.log(params);

        params.forEach(function (item, i, arr) {
            var message = `param id = ${item.id}, name = ${item.name}, desc = ${item.desc}`;
            output(message);
        })
    });

    let deviceId = document.getElementById('get_params_deviceId').value;
    let moduleId = document.getElementById('get_params_moduleId').value;
    let reqParams = { deviceId, moduleId }
    let sendStr = JSON.stringify(reqParams)

    request.send(sendStr);
});

document.getElementById("get_param_header").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/get_param_header", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let res = JSON.parse(request.response);
        console.log(res);
        var param = res;

        var message = `param id = ${param.id}, name = ${param.name}, desc = ${param.desc}, dev id = ${param.deviceId}, mod id = ${param.moduleId}`;
        output(message);
    });

    let deviceId = document.getElementById('get_param_deviceId').value;
    let paramId = document.getElementById('get_param_id').value;
    let reqParams = { deviceId, paramId }
    let sendStr = JSON.stringify(reqParams)

    request.send(sendStr);
});

document.getElementById("get_param_data").addEventListener('click', function (e) {
    e.preventDefault();

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/get_param_data", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        let res = JSON.parse(request.response);
        console.log(res);
        var paramValue = res;

        var message = `param id = ${paramValue.paramId}, value = ${paramValue.value}, value format = ${paramValue.valueFormat}`;
        output(message);
    });

    let deviceId = document.getElementById('get_param_data_deviceId').value;
    let paramId = document.getElementById('get_param_data_id').value;
    let reqParams = { deviceId, paramId }
    let sendStr = JSON.stringify(reqParams)

    request.send(sendStr);
});

document.getElementById("stream_param_data").addEventListener('click', function (e) {
    e.preventDefault();

    startTime_ = new Date().getTime();
    console.time(`The stream elapsed time(${startTime_}):`);

    let request = new XMLHttpRequest();
    // посылаем запрос на адрес "/load"
    request.open("POST", "/stream_param_data", true);
    request.setRequestHeader("Content-Type", "application/json");
    request.addEventListener("load", function () {
        // получаем и парсим ответ сервера
        /*
                let valStream = request.response.str;
                valStream.on('data', chunk => {
                    console.log(`Received: ${chunk.toString()}`);
                  });
        */

        /*
                let res = JSON.parse(request.response);
                console.log(res);
        
                var paramValue = res;
                var message = `param id = ${paramValue.paramId}, value = ${paramValue.value}, value format = ${paramValue.valueFormat}`;
                output(message);
        */
    });

    let deviceId = document.getElementById('get_param_data_deviceId').value;
    let paramId = document.getElementById('get_param_data_id').value;
    let reqParams = { deviceId, paramId }
    let sendStr = JSON.stringify(reqParams)

    request.send(sendStr);
});

//------------------------------------------------------------------------------------
dataSocket.onclose = function () {
    console.error("web channel closed");
};

dataSocket.onerror = function (error) {
    console.error("web channel error: " + error);
};

dataSocket.onopen = function () {
    output("WebSocket connected");
    dataSocket.onmessage = function (message) {

        if (message.data === null) {
            return;
        }

        var obj = JSON.parse(message.data);
        var message = `device_id = ${obj.deviceId}, param_id = ${obj.paramId}, value = ${obj.value}`;

//      output(message);
        console.log("data received: " + message);
        if (obj.value == -1) {
            console.log(`The stream is finished. Received ${msgCounter_} objects.`);
            console.timeEnd(`The stream elapsed time(${startTime_}):`);
        }

        msgCounter_++;
    };
}


