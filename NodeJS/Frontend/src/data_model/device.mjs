import Oscilloscope from './oscilloscope.mjs'

import { DeviceProvider } from "./services/fr_deviceprovider.mjs";
import {default as RequestHelper} from "./services/fr_requesthelper.mjs";

export class SysInterfacesEnum {
    static CanUav = 1;
    static CanOpen = 2;
    static ModBus = 3;
    static ModBusFO = 4;

    static toString(arg) {
        switch (arg) {
            case SysInterfacesEnum.CanUav: return 'UAV CAN';
            case SysInterfacesEnum.CanOpen: return 'CAN Open';
            case SysInterfacesEnum.ModBus: return 'ModBus';
            case SysInterfacesEnum.ModBusFO: return 'ModBus+FO';
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
