import { Model } from "./data_model/fr_model.mjs"
import Config from './.config.js';

export const model = new Model(Config.ip);
model.init();

model.on('error', (text) => { console.log(text);})


