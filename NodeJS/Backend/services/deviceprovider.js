const RequestHelper = require("../requesthelper.js");

const REQ_GET_DEVICES = "GET_DEVICE"
const REQ_GET_STATUS = "GET_STATUS"

class DeviceProvider
{
    constructor() 
    {
    }

    async requestDevice(deviceId, moduleId) 
    {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_DEVICES, deviceId, moduleId);

        return RequestHelper.request(reqCmd);
    };

    async reqDevices() 
    {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_DEVICES, 0);

        return RequestHelper.request(reqCmd);
    };

    async reqModule(deviceId, moduleId) 
    {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_DEVICES, deviceId, moduleId);

        return RequestHelper.request(reqCmd);
    };

    async reqStatus() 
    {
        let reqCmd = this._createDeviceReqCmd(REQ_GET_STATUS);

        return RequestHelper.request(reqCmd);
    };

    _createDeviceReqCmd(reqName, deviceId, moduleId) {
        let req = {
            request_id : this._genReqId(deviceId, moduleId),
            cmd : this._deviceCmd(reqName),
            body : this._deviceBody(deviceId, moduleId)
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
            return Math.floor(Math.random( ) * (MAX_ID - MIN_ID + 1)) + MIN_ID
        }
        
        return base + offset;
    }

    _deviceCmd(reqName) {
        let res;
        if (reqName == REQ_GET_DEVICES) {
            res = {
                name : "device_header",
                type : "get"
            };
        }

        return res;
    }

    _deviceBody(deviceId, moduleId) {
        let res = {
            device_id : deviceId,
            module_id : moduleId
        };

        return res;
    }
}

module.exports = {
    DeviceProvider: DeviceProvider
};