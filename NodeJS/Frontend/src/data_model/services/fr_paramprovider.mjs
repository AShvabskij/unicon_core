// import {RequestHelper} from "./fr_requesthelper.mjs"
const { RequestHelper } = require("./fr_requesthelper.mjs");

const REQ_GET_PARAMS = "GET_PARAMS"
const REQ_GET_PARAMS_DATA = "GET_PARAMS_DATA"
const REQ_SET_PARAMS_DATA = "SET_PARAMS_DATA"
const REQ_GET_PARAMS_STREAM_OPEN = "GET_PARAMS_STREAM_OPEN"
const REQ_GET_PARAMS_STREAM_CLOSE = "GET_PARAMS_STREAM_CLOSE"

export class ParamProvider {
    static streamIdList = [];

    constructor(reqHelper = RequestHelper) {
        this.m_reqHelper = reqHelper;
    }

    requestHelper() {
        return this.m_reqHelper;
    }

    setRequestHelper(reqHelper = RequestHelper) {
        this.m_reqHelper = reqHelper;
    }

    request(cmd) {
        return this.requestHelper().request(cmd);
    }

    async reqParams(deviceId, moduleId) {
        let reqCmd = this._createParamReqCmd(REQ_GET_PARAMS, deviceId, moduleId);

        return this.request(reqCmd);
    };

    async reqParam(deviceId, moduleId, paramId) {
        let reqCmd = this._createParamReqCmd(REQ_GET_PARAMS, deviceId, moduleId, paramId);

        return this.request(reqCmd);
    };

    async reqParamValue(deviceId, moduleId, paramId, stream, args) {
        let cmd = REQ_GET_PARAMS_DATA
        let openStream = null;
        if (stream === "true" || stream === "on") {
            cmd = REQ_GET_PARAMS_STREAM_OPEN;
            openStream = true;
        } else if (stream === "false" || stream === "off") {
            cmd = REQ_GET_PARAMS_STREAM_CLOSE;
            openStream = false;
        }

        let reqCmd = this._createParamReqCmd(cmd, deviceId, moduleId, paramId, args);

        if (openStream !== null) {
            if (openStream === true) {
                reqCmd.request_id = this._generateReqId(deviceId, paramId);
                ParamProvider.streamIdList.push(reqCmd.request_id);
            } else if (openStream === false) {
                reqCmd.request_id = this._generateReqId(deviceId, paramId);
                let index = ParamProvider.streamIdList.indexOf(reqCmd.request_id);
                if (index > -1) {
                    ParamProvider.streamIdList.splice(index, 1);
                }
            }
        }

        return this.request(reqCmd);
    };

    async setParamValue(deviceId, paramId, value) {
        let cmd = REQ_SET_PARAMS_DATA
        let args = {'value' : value};

        let reqCmd = this._createParamReqCmd(cmd, deviceId, null, paramId, args);

        return this.request(reqCmd);
    };

    _createParamReqCmd(reqName, deviceId, moduleId, paramId, args) {
        let req = {
            request_id: this._generateReqId(deviceId, paramId),
            cmd: this._paramCmd(reqName),
            body: this._paramBody(deviceId, moduleId, paramId, args)
        };

        return req;
    }

    _generateReqId(deviceId, paramId) {
        const PARAM_MAX_ID = 65536;
        let base = deviceId * PARAM_MAX_ID;
        let offset = (isNaN(paramId) ? 0 : paramId);

        return base + offset;
    }

    _paramCmd(reqName) {
        let res;
        if (reqName == REQ_GET_PARAMS) {
            res = {
                name: "param_header",
                type: "get"
            };
        }

        if (reqName == REQ_GET_PARAMS_DATA) {
            res = {
                name: "param_data",
                type: "get"
            };
        }

        if (reqName == REQ_SET_PARAMS_DATA) {
            res = {
                name: "param_data",
                type: "set"
            };
        }

        if (reqName == REQ_GET_PARAMS_STREAM_OPEN) {
            res = {
                name: "param_data",
                type: "open_stream"
            };
        }

        if (reqName == REQ_GET_PARAMS_STREAM_CLOSE) {
            res = {
                name: "param_data",
                type: "close_stream"
            };
        }

        return res;
    }

    _paramBody(deviceId, moduleId, paramId, args) {
        let res = {
            device_id: deviceId,
            module_id: moduleId,
            param_id: paramId
        };

        res = {...res, ...args}

        return res;
    }
}
/*
module.exports = {
    ParamProvider: ParamProvider
};
*/