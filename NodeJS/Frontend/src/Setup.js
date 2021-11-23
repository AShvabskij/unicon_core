import { Model, StatusEnum } from "./data_model/fr_model.mjs"
import Config from './.config.js';
import {Context} from "./Context"

export const model = new Model(Config.ip);
model.init();

Context.model = model;

model.on('system_status', (status) => {
    if (status === StatusEnum.Inited) {
        model.load().then(result => {
            console.log("loadDataModel result:" + result);
            console.log(model.devices());
        }, error => {
            console.log("loadDataModel error");
            console.log(error);
        });
    } else if (status === StatusEnum.Loaded) {
        Context.actions.update();
    }
})

model.on('error', (text) => { console.log(text);})
