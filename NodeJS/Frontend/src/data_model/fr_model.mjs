// import cbor from 'cbor' // from 'cbor-web'
// import { createRequire } from "module";
// const require = createRequire(import.meta.url);
import React, { useEffect } from "react";
import * as cbor from './../cbor.js';
import Oscilloscope from './oscilloscope.mjs'
import {Param, SysModule, SysInterfacesEnum} from './device.mjs'
import Device from './device.mjs'

// import {ParamProvider} from "./services/paramprovider.mjs"
// import {DeviceProvider} from "./services/deviceprovider.mjs"
const { RequestHelper } = require("./services/fr_requesthelper.mjs");
const { ParamProvider } = require("./services/fr_paramprovider.mjs");
const { DeviceProvider } = require("./services/fr_deviceprovider.mjs");
const Events = require('events');

const STATUS_OK = 200;
const ERROR_RESPONSE = {
    status: 500, // any server error
    msg: ""
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
            // cbor.decode([message],1,1);
            // cbor.decodeFirst(message, {float: true, preferWeb: true}).then(o => {
            //     console.log(JSON.stringify(o, null, 2))
            //   });
            /*
                        cbor.decodeFirst(message, {float: true, preferWeb: true}).then(o => {
                            console.log(JSON.stringify(o, null, 2))
                          });
            */
            var messageData = JSON.parse(message.data);

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

        param.streamValue(valueData, _messageDataLength);
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
                    device.osc = this.createOsc(oscHeader)

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

/*
module.exports = {
    Model,
    SysInterfacesEnum
};
*/