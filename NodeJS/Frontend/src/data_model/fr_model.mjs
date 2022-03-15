import EventEmitter from 'events'
import cbor from 'cbor'
import Config from '../.config.js';

import { Device, SysModule, SysInterfacesEnum } from './device.mjs'
import { Param } from './param.mjs'

import { ParamProvider } from "./services/fr_paramprovider.mjs";
import { DeviceProvider } from "./services/fr_deviceprovider.mjs";
import {default as RequestHelper} from "./services/fr_requesthelper.mjs";

const STATUS_OK = 200;
const ERROR_RESPONSE = {
    status: 500, // any server error
    msg: ""
}
const CONNECTION_TIMEOUT_MSC = 5000;

export class StatusEnum {
    static Loaded = 'DATA_LOADED';
    static Inited = "MODEL_INITED";
    static ReInited = "MODEL_REINITED";
    static Changed = 'DATA_CHANGED';
    static Cancelled = 'DATA_CANCELLED';
    static UnChanged = 'DATA_UNCHANGED';
}

let checkstatusIntervalId = 0;
let _messageDataLength = 0

const STREAM_SERVER_PORT = 1237;
const RECEIVED_DATA_ERROR = "Received data error!";
const CAPTURED_PARAMS_MAX = 32;

export class Model extends EventEmitter {
    constructor(srvHost) {
        super();

        this.m_name = 'Unicon';
        this.m_devices = [];
        this.m_devices_hash = ""; 
        this.m_oscs = [];
        this.m_trends = [];
        this.m_inited = false;
        this.m_firstInited = false;
        this.m_host = srvHost;

        this._capturedParams = [];

        this.paramProvider = new ParamProvider(RequestHelper);
        this.deviceProvider = new DeviceProvider(RequestHelper);
    }

    init() {
        if (this.m_inited) {
            return;
        }

        if (!this.m_firstInited) {
            RequestHelper.initConnection(this.m_host);
        }

        let streamSocketUrl = "ws://" + this.m_host + ":" + STREAM_SERVER_PORT;
        this.streamSocket = new WebSocket(streamSocketUrl);

        this.streamSocket.onopen = async (event) => {
            console.log(`Stream socket ${this.streamSocket.url} opened successfully.`);
            try {
                await RequestHelper.waitIsConnected()
                this.m_inited = true;
                if (!this.m_firstInited) {
                    this.m_firstInited = true;
                    this.emit('system_status', StatusEnum.Inited);
                } else {
                    this.emit('system_status', StatusEnum.ReInited);
                }    
            } catch(error) {
                console.error(error);
            }
        };

        this.streamSocket.onerror = (error) => {
            console.log('Stream error: ' + error.message);
        };

        this.streamSocket.onclose = (event) => {
            console.log('Stream closed, reason = ' + event.reason);

            this.m_inited = false;
            this.streamSocket = null;

            setTimeout(async () => {
                this.init();
            }, CONNECTION_TIMEOUT_MSC);

            return;
        };

        this.streamSocket.onmessage = async (message) => {
            if (!this.loaded()) {
                console.warn("The model is not loaded on receive message!");
                return;
            }

            var messageData;
            var data = message.data;
            if (data instanceof Blob) {
                var buffer = await data.arrayBuffer();
                messageData = cbor.decode(buffer);
            } else {
                messageData = JSON.parse(message.data);
            }

            let paramId = messageData.p_id;
            if (paramId != undefined) {
                _messageDataLength = message.data.length;

                let valueData = messageData.value;
                let deviceId = messageData.d_id;
                await this._streamParamValue(deviceId, paramId, valueData);
                return;
            }

            let deviceId = messageData.d_id;
            if (deviceId != undefined) {
                await this._streamOscValue(deviceId, messageData);
                return;
            }
        };

        this.m_inited = true;
    }

    async _streamParamValue(deviceId, paramId, valueData) {
        if (valueData == undefined) {
            console.warn(RECEIVED_DATA_ERROR);
            return;
        }

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

        if (param === undefined) {
            await this.forceCloseStream(deviceId, paramId);
            console.warn(RECEIVED_DATA_ERROR);
            return;
        }

        await param.streamValue(valueData, _messageDataLength);
    }

    async forceCloseStream(deviceId, paramId) {
        try {
            await this.paramProvider.reqParamValue(deviceId, null, paramId, "off")
        } catch (error) {
            console.log(error);
        }
    }

    async _streamOscValue(deviceId, valueData) {
        if (!this._capturedOscilloscope || this._capturedOscilloscope.deviceId !== deviceId) {
            let device = this.device(deviceId);
            if (device) {
                this._capturedOscilloscope = device.osc;
            }
        }

        let osc = this._capturedOscilloscope
        if (osc === undefined || osc === null || valueData == undefined) {
            console.warn("Unable to receive osc stream data. The osc is deactivated now");
            await this.deviceProvider.reqCloseOscStream(deviceId);
            return;
        }

        await osc.stream(valueData);
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
                            let param = this._createParamFromJson(paramInfoList[iii]);
                            module.params.push(param);
                            device.params.push(param);
                        }

                        device.modules.push(module);
                    }
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

    _createParamFromJson(paramInfo) {
        let res = new Param();
        res.deserialize(paramInfo);

        return res;
    }
}

/*
module.exports = {
    Model,
    SysInterfacesEnum
};
*/