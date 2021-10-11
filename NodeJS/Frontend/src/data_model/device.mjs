import Oscilloscope from './oscilloscope.mjs'

const Stream = require('stream-browserify');
const { RequestHelper } = require("./services/fr_requesthelper.mjs");
const { ParamProvider } = require("./services/fr_paramprovider.mjs");
const { DeviceProvider } = require("./services/fr_deviceprovider.mjs");

const STREAM_BUFFER_OBJECTS = 1;

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

export default class Device {
    constructor() {
        this.id = ''
        this.name = ''
        this.desc = ''
        this.image = 0
        this.info = ''
        this.osc = null
        this.interface = SysInterfacesEnum.Can
        this.interfaceName = 'Can';
        this.modules = []
        this.params = []

        this.deviceProvider = new DeviceProvider(RequestHelper);
    }

    module(moduleId) {
        let res = this.modules.find(item => item.id == moduleId);
        return res;
    }

    param(paramId) {
        let res = this.params.find(item => item.id == paramId);
        return res;
    }

    async getOsc() {
        
        if (this.osc != null) {
            this.deviceProvider.reqOsc(this.id); // for demo purpose only (need to reload osc data file)
            return this.osc;
        }
        
        let oscHeader = await this.deviceProvider.reqOsc(this.id);
        this.osc = this.createOsc(oscHeader)
        return this.osc;
    }

    createOsc(oscHeader) {
        let osc = new Oscilloscope();
        osc.id = oscHeader.id;
        osc.deviceId = oscHeader.device_id;
        osc.name = oscHeader.name;
        osc.resolution_ns = oscHeader.resolution_ns;
        osc.desc = oscHeader.desc;
        osc.channels = oscHeader.channels;

        return osc;
    }
}

export class SysModule {
    constructor() {
        this.id = 0;
        this.deviceId = 0;
        this.name = '';
        this.desc = '';
        this.params = [];
    }
}

export class Param {
    constructor() {
        this.id = 0;
        this.deviceId = 0;
        this.moduleId = 0;
        this.name = '';
        this.desc = '';
        this.rw = 'R';
        this.unit = '';
        this.value = new ParamValue();

        this.stream = null;
        this.buffer = [];
        this.buffIndex = 0;
        this.buffObjectCount = STREAM_BUFFER_OBJECTS;

        this.lastError = 0;

        this.paramProvider = new ParamProvider(RequestHelper);

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

    async openValueStream(frequency, buffObjectCount) {
        try {

            this._timeLabel = new Date().getTime();
            console.time(`The stream elapsed time(${this._timeLabel})`);
            this._byteCount = 0;
            this._msgCount = 0;

            this.buffIndex = 0;
            this.buffer = [];
            this.buffObjectCount = (buffObjectCount !== undefined) ? buffObjectCount : this.buffObjectCount;
            if (this.buffObjectCount === 0) {
                this.buffObjectCount === 1
            }

            for (var i = 0; i < this.buffObjectCount; i++) {
                this.buffer[i] = new ParamValue();
            }

            this.stream = new Stream.Readable({
                highWaterMark: this.buffObjectCount,
                objectMode: true,
                read() { }
            });

            let stream = "on";
            this.lastError = 0;
            this.value = new ParamValue()

            await this.paramProvider.reqParamValue(this.deviceId, this.moduleId, this.id, stream, { frequency: frequency });

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

    streamValue(valueData, messageDataLength) {

        if (this.stream === null || this.stream.destroyed) {
            console.warn(`Receiving value error. The param stream is deactivated now. Device id = ${this.deviceId}, param id = ${this.id}, value = ${JSON.stringify(valueData)}`);
            return;
        }

        if (this.value.value == valueData.value) {
            return;
        }

        let pValue = this.buffer[this.buffIndex];//new ParamValue();
        pValue.paramId = this.id;
        pValue.deviceId = this.deviceId;
        pValue.value = valueData.value;
        pValue.valueFormat = valueData.format;
        pValue.valueTime = valueData.time;
        pValue.scale = valueData.scale;

        this.value = this._paramValueFromStreamData(valueData);

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
            if (this.buffer.length > 0) {
                this.stream.push(this.buffer);
            }
            this.stream.push(null);
            this.stream.destroy();

            return;
        }

        // console.log(`Received: ${JSON.stringify(pValue)}`);
        this._byteCount += messageDataLength;
        this._msgCount++;

        //      this.buffer[this.buffIndex] = pValue;
        this.buffIndex++;

        if (this.buffIndex >= this.buffObjectCount) {
            this.stream.push(this.buffer);
            this.buffIndex = 0;
        }
    }

    lastError() {
        return this.lastError;
    }

    _paramValueFromStreamData(data) {
        this.value.paramId = this.id;
        this.value.deviceId = this.deviceId;
        this.value.value = data.value;
        this.value.valueFormat = data.format;
        this.value.valueTime = data.time;
        this.value.scale = data.scale;

        return this.value;
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

class ValueFormatEnum {
    static Int = 1;
    static Float = 2;
}