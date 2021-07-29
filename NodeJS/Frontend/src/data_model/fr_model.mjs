const Events = require ('events');

// import {ParamProvider} from "./services/paramprovider.mjs"
// import {DeviceProvider} from "./services/deviceprovider.mjs"

const {ParamProvider}  = require("./services/fr_paramprovider.mjs");
const {DeviceProvider}  = require("./services/fr_deviceprovider.mjs");

const Stream = require('stream-browserify');

const STATUS_OK = 200;
const ERROR_RESPONSE = {
    status: 500, // any server error
    msg: ""
}

export class SysInterfacesEnum
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
    static UnChanged = 'DATA_UNCHANGED';
}

let checkstatusIntervalId = 0;
let streamServerPort = 1237;
let host =  '192.168.7.111';
let streamSocketUrl = "ws://" + host + ":" + streamServerPort; 

// Переменные для измерения производительности
let byteCount = 0;
let msgCount = 0;

let timeLabel = new Date().getTime();
let startTime = new Date().getTime();

const RECEIVED_DATA_ERROR = "Received data error!";

export class Model extends Events
{
    constructor() 
    {
        super();

        this.m_name = 'Unicon';
        this.m_devices = [];
        this.m_trends= [];
        this.m_capturedParam = new Param();
        this.m_inited = false;

        this.paramProvider = new ParamProvider();
        this.deviceProvider = new DeviceProvider();
    }

    init() 
    {
        if (this.m_inited) {
            return;
        }
        
        this.streamSocket = new WebSocket(streamSocketUrl);

        this.streamSocket.onopen = event => {
            console.log('Stream socket opened successfully.');
        };

        this.streamSocket.onerror = function(error) {
            console.log('Stream error: ' + error.message);
        };
        
        this.streamSocket.onclose = function() {
            console.log('Stream closed.');
        };

        this.streamSocket.onmessage = message => {
            var messageData = JSON.parse(message.data);
            // console.log(message.data);

            let deviceId = messageData.d_id;
            let paramId = messageData.p_id;
            // let paramId = messageData.id;     

            if (this.m_capturedParam.id != paramId || this.m_capturedParam.deviceId != deviceId) {
                console.log("devices = " + this.m_devices.length);

                let device = this.device(deviceId);
                if (device == undefined || device == null) {
                    console.warn(`device ${deviceId} error!`)
                    return;
                }

                this.m_capturedParam = this.device(deviceId).param(paramId);
            }

            let valueData = messageData.value;
            if (valueData === undefined) {
                console.log(RECEIVED_DATA_ERROR);
                this.emit('error', RECEIVED_DATA_ERROR);
                return;
            }

            let pValue = new ParamValue();
            pValue.paramId = this.m_capturedParam.id;
            pValue.deviceId = this.m_capturedParam.deviceId;
            pValue.value = valueData.value;
            pValue.valueFormat = valueData.format;
            pValue.valueTime = valueData.time;
            pValue.scale = valueData.scale;
    
            byteCount += message.data.length;
            msgCount ++;

            let currTime = new Date().getTime();            
            let pValueDeltaTime = pValue.valueTime > 0 ? currTime - pValue.valueTime : 0

            if (pValueDeltaTime > 50) {
                console.warn (`Param value actuality = ${pValueDeltaTime}`)
            }

            const timeDelta = currTime - startTime;
            const isFinished = (pValue.value == -1);

            if (timeDelta >= 1000 || isFinished === true) {
                console.log(`Received ${byteCount} bytes, ${msgCount} items per ${timeDelta}ms`);
                startTime = currTime;
            }

            if (isFinished === true) {
                console.log(`The stream is finished`);
                console.timeEnd(`The stream elapsed time(${timeLabel}):`);

                byteCount = 0;
                msgCount = 0;

                let pValue = new ParamValue();
                pValue.paramId = this.m_capturedParam.id;
                pValue.deviceId = this.m_capturedParam.deviceId;
                pValue.value = -1;
    
                this.m_capturedParam.stream.push(JSON.stringify(pValue));
                this.m_capturedParam.stream.push(null);
                this.m_capturedParam = new Param();
            }

//          console.log(`Received: ${JSON.stringify(pValue)}`);
            this.m_capturedParam.stream.push(JSON.stringify(pValue));
        };        

        this.m_inited = true;
    }

    async load() 
    {
        return new Promise(async (resolve, reject) => {
            try {

                if (!this.m_inited) {
                    this.init();
                }

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

                console.log("loaded devices  = " + this.m_devices.length);

                resolve({ result:'true', status:200});

            } catch(err) {
                console.log(err);
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
            let res = await this.deviceProvider.reqStatus();
            if (res.system_status == StatusEnum.Changed) {
//              this.clear()
            }

            if (res.system_status == StatusEnum.Cancelled) {
                this.clear()
            }

            this.emit('system_status', res.system_status);

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
        
        this.stream = new Stream.Readable({
            read() {}
        });
        
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
            this.lastError = err;
        }

        return this.value;
    }

    async openValueStream() {
        try {
            timeLabel = new Date().getTime();
            startTime = new Date().getTime();
            console.time(`The stream elapsed time(${timeLabel}):`);

            let stream = "on";
            this.stream = new Stream.Readable({
                read() {}
            });

            this.lastError = 0;
            let valueData = await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream);
            this.value = Param.paramValueFromJson(valueData);
            
        } catch(error) {
            this.value = new ParamValue()
            this.lastError = error;
            console.log(error);
        }

        return this.stream;
    }

    async closeValueStream() {
        try {
            let stream = "off";
            await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream)
            this.stream.push(null);

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
/*
module.exports = {
    Model,
    SysInterfacesEnum
};
*/
