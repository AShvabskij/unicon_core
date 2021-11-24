// const RequestHelper = require("./fr_requesthelper.mjs");
import RequestHelper from "./fr_requesthelper.mjs";

const REQ_GET_DEVICES = "GET_DEVICE"
const REQ_GET_STATUS = "GET_STATUS"
const REQ_GET_OSC = "GET_OSC"
const REQ_GET_OSC_STREAM_OPEN = "GET_OSC_STREAM_OPEN"
const REQ_GET_OSC_STREAM_CLOSE = "GET_OSC_STREAM_CLOSE"

export class DeviceProvider {
    m_reqHelper = null;

    constructor(reqHelper = RequestHelper) {
        this.m_reqHelper = reqHelper;
    }

    requestHelper() {
        return this.m_reqHelper;
    }

    setRequestHelper(reqHelper = RequestHelper) {
        this.m_reqHelper = reqHelper;
    }
    
    request(cmd, timeout) {
        return this.requestHelper().request(cmd, timeout);
    }

    async requestDevice(deviceId, moduleId) {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_DEVICES, deviceId, moduleId);

        return this.request(reqCmd);
    };

    async reqDevices() {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_DEVICES, 0);

        return this.request(reqCmd);
    };

    async reqModule(deviceId, moduleId) {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_DEVICES, deviceId, moduleId);

        return this.request(reqCmd);
    };

    async reqOsc(deviceId) {
        let reqCmd = this._createOscReqCmd(REQ_GET_OSC, deviceId);
        return this.request(reqCmd, 10000); // 10000 for demo purpose only
    }

    async reqOpenOscStream(deviceId, oscId, channels) {
        let cmd = REQ_GET_OSC_STREAM_OPEN;
        let reqCmd = this._createDeviceReqCmd(cmd, deviceId, oscId, channels);

        return this.request(reqCmd);
    }

    async reqCloseOscStream(deviceId, oscId) {
        let cmd = REQ_GET_OSC_STREAM_CLOSE;
        let reqCmd = this._createDeviceReqCmd(cmd, deviceId, oscId);

        return this.request(reqCmd);
    }


    async reqStatus() {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_STATUS);

        return this.request(reqCmd);
    };

    _createDeviceReqCmd(reqName, deviceId, moduleId, channels) {
        let req = {
            request_id: this._genReqId(deviceId, moduleId),
            cmd: this._deviceCmd(reqName),
            body: this._deviceBody(deviceId, moduleId, channels)
        };

        return req;
    }

    _createOscReqCmd(reqName, deviceId, oscId) {
        let req = {
            request_id: this._genReqId(deviceId),
            cmd: this._deviceCmd(reqName),
            body: this._deviceOscBody(deviceId, oscId)
        };

        return req;
    }

    _genReqId(deviceId, moduleId) {
        const MODULE_MAX_ID = 65536;
        const MAX_ID = 65536;
        const MIN_ID = 1000;

        let base = (isNaN(deviceId) ? 0 : deviceId) * MODULE_MAX_ID;
        let offset = (isNaN(moduleId) ? 0 : moduleId);
        if (base == 0) {
            return Math.floor(Math.random() * (MAX_ID - MIN_ID + 1)) + MIN_ID
        }

        return base + offset;
    }

    _deviceCmd(reqName) {
        let res;
        if (reqName == REQ_GET_DEVICES) {
            res = {
                name: "device_header",
                type: "get"
            };
        } else if (reqName == REQ_GET_STATUS) {
            res = {
                name: "system_status",
                type: "get"
            };
        } else if (reqName == REQ_GET_OSC_STREAM_OPEN) {
            res = {
                name: "osc_data",
                type: "open_stream"
            };
        } else if (reqName == REQ_GET_OSC_STREAM_CLOSE) {
            res = {
                name: "osc_data",
                type: "close_stream"
            }
        } else if (reqName == REQ_GET_OSC) {
            res = {
                name: "osc_header",
                type: "get"
            };
        }

        return res;
    }

    _deviceBody(deviceId, moduleId, channels) {
        let res = {
            device_id: deviceId,
            module_id: moduleId,
            channels: channels
        };

        return res;
    }

    _deviceOscBody(deviceId, oscId) {
        let res = {
            device_id: deviceId
        };
        if (oscId !== undefined) {
            res.oscId = oscId;
        }

        return res;
    }
}

/*
module.exports = {
    DeviceProvider: DeviceProvider
};
*/