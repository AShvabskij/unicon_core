const Events = require("events");

const {ParamProvider}  = require("./services/paramprovider.js");
const {DeviceProvider}  = require("./services/deviceprovider.js");

const WebSocket = require('faye-websocket');

const STATUS_OK = 200;
const ERROR_RESPONSE = {
    status: 500, // any server error
    msg: ""
}

class SysInterfacesEnum
{
    static Can = 1;
    static CanOpen = 2;
    static ModBus = 3;
    static FO = 4; 
}

class ValueFormatEnum
{
    static Int = 1;
    static Float = 2;
}

class StatusEnum
{
    static Changed = 'DATA_CHANGED';
    static Cancelled = 'DATA_CANCELLED';
}

let checkstatusIntervalId = 0;
let streamServerPort = 1237;
let host =  'localhost';
let streamSocketUrl = "ws://" + host + ":" + streamServerPort; 
let streamSocket = new WebSocket.Client(streamSocketUrl);

class Model extends Events
{
    static events = new Events();

    constructor() 
    {
        super();

        this.m_name = 'Unicon';
        this.m_devices = [];
        this.m_trends= [];

        this.paramProvider = new ParamProvider();
        this.deviceProvider = new DeviceProvider();
    }

    async init() 
    {
        streamSocket = new WebSocket.Client(streamSocketUrl);

        streamSocket.on('open', function(event) {
            console.log('Stream opened successfully.');
            streamSocket.on('message', function(message) {
                var messageData = JSON.parse(message.data);
                let data = messageData.body;
                let value = Param.paramValueFromJson(data);
                Model.events.emit('stream', value);
            });
        });

        streamSocket.on('error', function(error) {
            console.log('Stream error: ' + error.message);
            process.exit(1);
        });
        
        streamSocket.on('close', function() {
            console.log('Stream closed.');
            process.exit(1);
        });        

        return new Promise(async (resolve, reject) => {
            try {
                let devices = await this.deviceProvider.reqDevices();
    
                for (var i = 0; i < devices.length; i++) {

                    let item = devices[i];
                    let device = new Device();
                    device.name = item.name;
                    device.id = item.id;
                    device.desc = item.desc;
    
                    for (var ii = 0; ii < item.modules.length; ii++) {
                        let moduleId = item.modules[ii];
                        if (moduleId === 0) {
                            continue;
                        }

                        let moduleInfo = await this.deviceProvider.reqModule(device.id, moduleId);
                        let paramInfoList = await this.paramProvider.reqParams(device.id, moduleId);
                        
                        let module = this._createModuleFromJson(device.id, moduleInfo);
                        
                        for (var iii = 0; iii < paramInfoList.length; iii++) {
                            let param = this._createParamFromJson(device.id, moduleId, paramInfoList[iii]);
                            module.params.push(param);
                            device.params.push(param);
                        }

                        device.modules.push(module);
                    }
    
                    this.m_devices.push(device)
                }
                resolve(this);
            } catch(err) {
                reject(err);
            }
        });
    }

    enablePeriodicCheck() 
    {
        if (checkstatusIntervalId > 0) {
            return;
        }

        checkstatusIntervalId = setInterval(async () => {
            let status = await this.deviceProvider.reqStatus();
            if (status == StatusEnum.Changed) {
                this.clear()
                Model.events.emit(status);
            }

            if (status == StatusEnum.Cancelled) {
                this.clear()
                Model.events.emit(status);
            }

        }, 5000)
    }

    disablePeriodicCheck() 
    {
        clearInterval(checkstatusIntervalId);
        checkstatusIntervalId = 0;
    }

    clear() 
    {
        this.disablePeriodicCheck();

        this.m_devices = [];
        this.m_trends= [];
    }

    device(id)
    {
        let result = this.m_devices.find (item => item.id == id);
        return result;
    }

    devices(sysInterface)
    {
        if (sysInterface == undefined) {
            return this.m_devices;
        }

        let result = this.m_devices.filter(item => item.interface === sysInterface);
        return result;
    }

    sysInterfaces()
    {
        let result = this.m_devices.reduce((res, current) => {
            if (!(current.interface in res)) {
                res.push(current.interface);
            }
        }, []);

        return result;
    }

//----------------------------------------------------------------------------------------
    _createModuleFromJson(deviceId, moduleInfo)
    {
        let res = new SysModule();

        res.id = moduleInfo.id;
        res.deviceId = deviceId;
        res.name = moduleInfo.name;
        res.desc = moduleInfo.desc;

        return res;
    }

    _createParamFromJson(deviceId, moduleId, paramInfo)
    {
        let res = new Param();

        res.deviceId = deviceId;
        res.moduleId = moduleId;
        res.id = paramInfo.param_id;
        res.name = paramInfo.name;
        res.desc = paramInfo.desc;

        return res;
    }
}

class Device 
{
    constructor() 
    {
        this.id = ''
        this.name = ''
        this.desc = ''
        this.image = 0
        this.osc = new Osciloscope()
        this.interface = SysInterfacesEnum.Can
        this.modules = []
        this.params = []
    }

    module(moduleId)
    {
        let result = this.modules.find(item => item.id == moduleId);
        return result;
    }

    param(paramId)
    {
        let result = this.params.find(item => item.id == paramId);
        return result;
    }
}

class Osciloscope 
{
    constructor() 
    {
        this.id = ''
        this.name = ''
        this.desc = ''
    }
}

class SysModule 
{
    constructor() 
    {
        this.id = 0;
        this.deviceId = 0;
        this.name = '';
        this.desc = '';
        this.params = [];
    }
}

class Param
{
    constructor() 
    {
        this.id = 0;
        this.deviceId = 0;
        this.moduleId = 0;
        this.name = '';
        this.desc = '';
        this.value = new ParamValue();
        this.lastError = 0;

        this.paramProvider = new ParamProvider();
    }

    async lastValue() {
        setTimeout(() => {
           this.currentValue(); 
        }, 0)

        return this.value;
    }

    async currentValue() {

        try {
            let valueData = await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id);
            this.value = Param.paramValueFromJson(valueData)
            this.lastError = 0;
        } catch(err) {
            this.value = new ParamValue()
            this.lastError = error;
        }

        return this.value;
    }

    async openValueStream() {
        try {
            let stream = "on";
            let valueData = await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream);
            this.value = Param.paramValueFromJson(valueData)
            this.lastError = 0;
        } catch(error) {
            this.value = new ParamValue()
            this.lastError = error;
        }

        return this.value;        
    }

    async closeValueStream() {
        try {
            let stream = "off";
            await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream)
            this.lastError = 0;
        } catch(error) {
            this.lastError = error;
        }
    }

    lastError() {
        return this.lastError;
    }

    static paramValueFromJson(data) {
        // формат ожидаемых данных: {param_id: , device_id:, value:{value, format, time, scale}}
        let res = new ParamValue();
        res.paramId = data.param_id;
        res.deviceId = data.device_id;
        res.value = data.value.value;
        res.valueFormat = data.value.format;
        res.valueTime = data.value.time;
        res.scale = data.value.scale;

        return res;
    }
}

class ParamValue 
{
    constructor()
    {
        this.paramId = 0;
        this.deviceId = 0;
        this.valueFormat = ValueFormatEnum.Int
        this.valueTime = 0;
        this.scale = 0.0;
        this.value = -1.0;
    }
}

module.exports = {
    Model,
    SysInterfacesEnum
};