import Oscilloscope from './oscilloscope.mjs'

const { RequestHelper } = require("./services/fr_requesthelper.mjs");
const { DeviceProvider } = require("./services/fr_deviceprovider.mjs");

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

export class Device {
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
        this.osc = new Oscilloscope();
        this.osc.deserialize(oscHeader);

        return this.osc;
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
