// import cbor from 'cbor' // from 'cbor-web'
// import { createRequire } from "module";
// const require = createRequire(import.meta.url);

const Events = require('events');

// import {ParamProvider} from "./services/paramprovider.mjs"
// import {DeviceProvider} from "./services/deviceprovider.mjs"

const { RequestHelper } = require("./services/fr_requesthelper.mjs");
const { ParamProvider } = require("./services/fr_paramprovider.mjs");
const { DeviceProvider } = require("./services/fr_deviceprovider.mjs");

const Stream = require('stream-browserify');

const STATUS_OK = 200;
const ERROR_RESPONSE = {
    status: 500, // any server error
    msg: ""
}

const OSC_MAX_CHANNELS = 8;

export class SysInterfacesEnum {
    static Can = 1;
    static CanOpen = 2;
    static ModBus = 3;
    static FO = 4;

    static toString(arg) {
        switch (arg) {
            case SysInterfacesEnum.Can: return 'Can';
            case SysInterfacesEnum.CanOpen: return 'CanOpen';
            case SysInterfacesEnum.ModBus: return 'ModBus';
            case SysInterfacesEnum.FO: return 'FO';
        }
    }
}

class ValueFormatEnum {
    static Int = 1;
    static Float = 2;
}

class StatusEnum {
    static Changed = 'DATA_CHANGED';
    static Cancelled = 'DATA_CANCELLED';
    static UnChanged = 'DATA_UNCHANGED';
}

let checkstatusIntervalId = 0;
let _messageDataLength = 0

const STREAM_SERVER_PORT = 1237;
const RECEIVED_DATA_ERROR = "Received data error!";
const STREAM_BUFFER_OBJECTS = 5;
const CAPTURED_PARAMS_MAX = 12;

export class Model extends Events {
    constructor(srvHost) {
        super();

        this.m_name = 'Unicon';
        this.m_devices = [];
        this.m_oscs = [];
        this.m_trends = [];
        this.m_inited = false;
        this.m_host = srvHost;

        this._capturedParams = [];

        this.paramProvider = new ParamProvider(RequestHelper);
        this.deviceProvider = new DeviceProvider(RequestHelper);
    }

    init() {
        if (this.m_inited) {
            return;
        }

        RequestHelper.initConnection(this.m_host);

        let streamSocketUrl = "ws://" + this.m_host + ":" + STREAM_SERVER_PORT;

        this.streamSocket = new WebSocket(streamSocketUrl);

        this.streamSocket.onopen = (event) => {
            console.log(`Stream socket ${this.streamSocket.url} opened successfully.`);
            this.m_inited = true;
        };

        this.streamSocket.onerror = (error) => {
            console.log('Stream error: ' + error.message);
        };

        this.streamSocket.onclose = () => {
            console.log('Stream closed.');

            this.m_inited = false;
            this.m_socket = null;

            setTimeout(async () => {
                this.init();
            }, 5000);

            return;
        };

        this.streamSocket.onmessage = (message) => {
            if (!this.loaded()) {
                console.warn("The model is not loaded!");
                return;
            }
/*
            cbor.decodeFirst(message, {float: true, preferWeb: true}).then(o => {
                console.log(JSON.stringify(o, null, 2))
              });
*/
            var messageData = JSON.parse(message.data);
//          var messageData = CBOR.decode(message.data);

            let paramId = messageData.p_id;
            if (paramId != undefined) {
                let valueData = messageData.value;
                let deviceId = messageData.d_id;

                _messageDataLength = message.data.length;

                this._streamParamValue(deviceId, paramId, valueData);
                return;
            }

            // let oscId = messageData.o_id;
            let deviceId = messageData.d_id;
            if (deviceId != undefined) {
                this._streamOscValue(deviceId, messageData);
                return;
            }
        };

        this.m_inited = true;
    }

    _streamParamValue(deviceId, paramId, valueData) {
        let param = this._capturedParams.find((item) => {
            return item.id === paramId && item.deviceId === deviceId
        });

        if (param === undefined) {
            param = this.device(deviceId).param(paramId);
            if (param !== undefined) {
                if (this._capturedParams.length > CAPTURED_PARAMS_MAX) {
                    console.info("Clearing captured params cache");
                    this._capturedParams = [];
                }

                this._capturedParams.push(param);
            }
        }

        if (param === undefined || valueData == undefined) {
            console.log(RECEIVED_DATA_ERROR);
            this.emit('error', RECEIVED_DATA_ERROR);
            return;
        }

        param.streamValue(valueData);
    }

    _streamOscValue(deviceId, valueData) {
        let osc = this._capturedOscilloscope

        if (osc === undefined || osc.deviceId !== deviceId) {
            osc = this.device(deviceId).osc;
            if (osc !== undefined) {
                this._capturedOscilloscope = osc;
            }
        }

        if (osc === undefined || valueData == undefined) {
            console.log(RECEIVED_DATA_ERROR);
            this.emit('error', RECEIVED_DATA_ERROR);
            return;
        }

        osc.stream(valueData);
    }

    async load() {
        return new Promise(async (resolve, reject) => {
            try {

                if (!this.m_inited) {
                    let err = "The data model is not inited yet. Will be inited now."
                    console.warn(err);
                    this.init();
                    //                  reject({ status: 500, msg: err });
                }

                let devices = await this.deviceProvider.reqDevices();

                for (var i = 0; i < devices.length; i++) {

                    let item = devices[i];
                    let device = new Device();

                    device.name = item.name;
                    device.id = item.id;
                    device.desc = item.desc;
                    device.interface = item.channel;
                    device.interfaceName = SysInterfacesEnum.toString(item.channel);

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

                    let oscHeader = await this.deviceProvider.reqOsc(device.id);

                    device.osc.id = oscHeader.id;
                    device.osc.deviceId = oscHeader.device_id;
                    device.osc.name = oscHeader.name;
                    device.osc.resolution_ns = oscHeader.resolution_ns;
                    device.osc.desc = oscHeader.desc;

                    this.m_devices.push(device)
                }

                console.log("loaded devices  = " + this.m_devices.length);

                resolve({ result: 'true', status: 200 });

            } catch (err) {
                console.log(err);
                reject(err);
            }
        });
    }

    loaded() {
        return this.m_devices.length > 0;
    }


    enablePeriodicCheck() {
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

    disablePeriodicCheck() {
        if (checkstatusIntervalId === 0) {
            return;
        }

        clearInterval(checkstatusIntervalId);
        checkstatusIntervalId = 0;
    }

    clear() {
        this.disablePeriodicCheck();

        this.m_devices = [];
        this.m_trends = [];
        this._capturedParams = [];
    }

    device(id) {
        let result = this.m_devices.find(item => item.id == id);
        return result;
    }

    devices(sysInterface) {
        if (sysInterface == undefined) {
            return this.m_devices;
        }

        let result = this.m_devices.filter(item => item.interfaceName === sysInterface);
        return result;
    }

    sysInterfaces() {
        let result = this.m_devices.reduce((res, current) => {
            if (!(current.interfaceName in res)) {
                res.push(current.interfaceName);
            }
        }, []);

        return result;
    }

    //----------------------------------------------------------------------------------------
    _createModuleFromJson(deviceId, moduleInfo) {
        let res = new SysModule();

        res.id = moduleInfo.id;
        res.deviceId = deviceId;
        res.name = moduleInfo.name;
        res.desc = moduleInfo.desc;

        return res;
    }

    _createParamFromJson(deviceId, moduleId, paramInfo) {
        let res = new Param();

        res.deviceId = deviceId;
        res.moduleId = moduleId;
        res.id = paramInfo.param_id;
        res.name = paramInfo.name;
        res.desc = paramInfo.desc;
        res.unit = paramInfo.value_unit;

        return res;
    }
}

export class Device {
    constructor() {
        this.id = ''
        this.name = ''
        this.desc = ''
        this.image = 0
        this.osc = new Osciloscope()
        this.interface = SysInterfacesEnum.Can
        this.interfaceName = 'Can';
        this.modules = []
        this.params = []
    }

    module(moduleId) {
        let res = this.modules.find(item => item.id == moduleId);
        return res;
    }

    param(paramId) {
        let res = this.params.find(item => item.id == paramId);
        return res;
    }
}

export class Osciloscope {
    constructor() {
        this.id = 0
        this.deviceId = 0
        this.name = ''
        this.desc = ''
        this.resolution_ns = 0;
        this.lastError = 0;
        this.channelStreams = []

        this.deviceProvider = new DeviceProvider();

        // Переменные для измерения производительности
        this._byteCount = 0;
        this._msgCount = 0;
        this._timeLabel = new Date().getTime();
    }

    async openDataStream() {

        this.channelStreams = [];
        for (var i = 0; i < 16; ++i) {
            this.channelStreams.push(new Stream.Readable({
                highWaterMark: 1, //STREAM_BUFFER_OBJECTS,
                objectMode: true,
                read() { }
            }));
        }

        this._byteCount = 0;
        this._msgCount = 0;
        this._timeLabel = new Date().getTime();
        console.time(`The stream elapsed time(${this._timeLabel})`);

        try {
            let openStream = true;
            await this.deviceProvider.reqDataStream(this.deviceId, this.id, openStream);
        } catch (error) {
            this.lastError = error;
            console.error(error);
        }
    }

    async closeDataStream() {
        try {
            let openStream = false;
            await this.deviceProvider.reqDataStream(this.deviceId, this.id, openStream);
            for (var i = 0; i < 16; ++i) {
                this.channelStreams[i].push(null);
                this.channelStreams[i].destroy;
            }
            this.channelStreams = [];
        } catch (error) {
            this.lastError = error;
            console.error(error);
            this.channelStreams = [];
        }
    }

    stream(socketData) {
        for (var i = 0; i <= OSC_MAX_CHANNELS; ++i) {
            let data = this.parse(socketData, i);
            if (data == -1) {
                continue;
            }
            this.channelStreams[i].push(data);
        }
    }

    parse(socketData, ch) {
        if (socketData.values == undefined) {
            return -1;
        }

        if (socketData.error === 2) {
            if (ch == OSC_MAX_CHANNELS) {
                console.log(`The osc stream is finished, device id = ${this.deviceId}, received items = ${this._msgCount}, bytes = ${this._byteCount}`);
                console.timeEnd(`The stream elapsed time(${this._timeLabel})`);
            }
            return null;
        }

        let chValues = socketData.values[ch];
        if (chValues === undefined) {
            return null;
        }

        if (ch == OSC_MAX_CHANNELS) {
            let receivedBytes = JSON.stringify(chValues[0]).length * chValues.length * socketData.values.length;
            this._byteCount += receivedBytes;
            //          console.log('Received bytes = ' + receivedBytes);
        }

        this._msgCount += 1;

        let time_ns = socketData.time - chValues.length * this.resolution_ns;
        let values = chValues.map(val => {
            let time = time_ns * 0.001 - this._timeLabel;
            time_ns += this.resolution_ns;

            return { val, time };
        })
/*
        var result = [];
        var i = 0;
        values.reduce(function (prevRes, item) {
            if (++i % 10 === 0) {
                result.push(item);
            }
            return item;
        });
*/
        return values;
    }
}

class SysModule {
    constructor() {
        this.id = 0;
        this.deviceId = 0;
        this.name = '';
        this.desc = '';
        this.params = [];
    }
}

class Param {
    constructor() {
        this.id = 0;
        this.deviceId = 0;
        this.moduleId = 0;
        this.name = '';
        this.desc = '';
        this.unit = '';
        this.value = new ParamValue();

        this.stream = null;
        this.buffer = [];
        this.buffIndex = 0;

        this.lastError = 0;

        this.paramProvider = new ParamProvider();

        // Переменные для измерения производительности
        this._byteCount = 0;
        this._msgCount = 0;

        this._timeLabel = new Date().getTime();
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
        } catch (err) {
            this.value = new ParamValue()
            this.lastError = err;
        }

        return this.value;
    }

    async openValueStream() {
        try {

            this._timeLabel = new Date().getTime();
            console.time(`The stream elapsed time(${this._timeLabel})`);
            this._byteCount = 0;
            this._msgCount = 0;

            this.buffIndex = 0;
            this.buffer = [];
            for (var i = 0; i < STREAM_BUFFER_OBJECTS; i++) {
                this.buffer[i] = new ParamValue();
            }

            this.stream = new Stream.Readable({
                highWaterMark: STREAM_BUFFER_OBJECTS,
                objectMode: true,
                read() { }
            });

            let stream = "on";
            this.lastError = 0;
            this.value = new ParamValue()

            await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream);

        } catch (error) {
            this.value = new ParamValue()
            this.lastError = error;
            console.error(error);
        }

        return this.stream;
    }

    async closeValueStream() {
        try {

            let stream = "off";

            await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream)

            if (this.stream === null) {
                return;
            }

            if (!this.stream.destroyed) {
                this.stream.push(null);
                this.stream.destroy();
            }
            this.lastError = 0;

        } catch (error) {
            this.lastError = error;
        }
    }

    streamValue(valueData) {

        if (this.stream === null || this.stream.destroyed) {
            console.warn(`Receiving value error. The param stream is deactivated now. Device id = ${this.deviceId}, param id = ${this.id}, value = ${JSON.stringify(valueData)}`);
            return;
        }

        let pValue = this.buffer[this.buffIndex];//new ParamValue();
        pValue.paramId = this.id;
        pValue.deviceId = this.deviceId;
        pValue.value = valueData.value;
        pValue.valueFormat = valueData.format;
        pValue.valueTime = valueData.time;
        pValue.scale = valueData.scale;

        let currTime = new Date().getTime();
        let pValueDeltaTime = pValue.valueTime > 0 ? currTime - pValue.valueTime : 0

        if (pValueDeltaTime > 50) {
            console.warn(`Param value actuality = ${pValueDeltaTime}`)
        }

        const isFinished = (pValue.value == -1);

        if (isFinished === true) {
            console.log(`The param stream is finished, device id = ${this.deviceId}, param id = ${this.id}, received items = ${this._msgCount}, bytes = ${this._byteCount}`);
            console.timeEnd(`The stream elapsed time(${this._timeLabel})`);

            this.buffer.splice(this.buffIndex)
            this.stream.push(this.buffer);
            this.stream.push(null);
            this.stream.destroy();

            return;
        }

        // console.log(`Received: ${JSON.stringify(pValue)}`);
        this._byteCount += _messageDataLength;
        this._msgCount++;

        //      this.buffer[this.buffIndex] = pValue;
        this.buffIndex++;

        if (this.buffIndex >= STREAM_BUFFER_OBJECTS) {
            this.stream.push(this.buffer);
            this.buffIndex = 0;
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

class ParamValue {
    constructor() {
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