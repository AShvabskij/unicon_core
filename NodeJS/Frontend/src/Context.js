import React, {useState,useEffect} from "react";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';

// export const Context = React.createContext();
let startDate = new Date();

const updateParameters = (indexDevice) => {
    // console.log(Info);
    // console.log("indexDevice="+indexDevice);
   
    let deviceItem = Info.model.m_devices[indexDevice];
    
    let checkbox = [{view:"checkbox", label:" ", value:1, uncheckValue:"off", checkValue:"on"}];
    let dt1 = [];
    let dtt = [];
    let i = 1;
    let j = 1;
    let infoCurrentDivice = deviceItem.desc + ". Channel: " + deviceItem.interfaceName;
    let desc = $$("descriptionDevice");
    desc.setValue(infoCurrentDivice);
    deviceItem.modules.forEach(function(item, index, array) {
        // console.log(item.params);
        dtt = [];
        item.params.forEach(function(itemP, indexP, array) {
            infoCurrentDivice = infoCurrentDivice + " [" + itemP.deviceId + "]" + "</br>Chanal: " + item.interfaceName;
            // if(=="Can")
            dtt.push({id: "m"+i, // name: itemP.moduleId,  //"[" + itemP.deviceId + "] " + item.name + " [" + itemP.moduleId + "]", 
            name:itemP.name + " [" + itemP.deviceId + "." + itemP.moduleId + "." + itemP.id + "]", 
            value:" ", dimension:itemP.unit, time:" ", 
            chart:0, 
            numchart:1,
            param:itemP
        })
            i++;
        });
        dt1.push({"id":"modul"+j, "name": item.name + " [" + item.deviceId + "." + item.id + "]",
        "open":false, "data":dtt 
        });
        j++;
    });
    
    Info.elements.menuTop.setState((state, props) => ({
            // dt: [{"id":"can","programmInt":"Can", "open":"false", "data":dt1}]
            dt: {"id":"can","data":dt1}
     }));
}

const updateLeftMenuBase = (devicesArr) => {
    let options = [];
    let  devices = { margin:10, padding:0, type:"wide",
    view:"flexlayout",cols:[]};
    devicesArr.forEach(function(item, index, array) {
        // console.log(item, index);
        devices.cols.push( { view:"toggle", label:item.name + "</br>Channel: " + item.interfaceName, minWidth: 100, height: 60, css: "webix_primary", modules: item.modules,
        click:function(id,event){
            updateParameters(index);
            
            let s1 = $$(id).getParentView();
            s1._cells.forEach(element => {
                element.setValue(0);
            });
        }
        })
  });
   
    webix.ui(devices,$$("DeviceInit"), 0);
} 

const updateLeftMenu = () => {
    updateLeftMenuBase(Info.model.m_devices)
}

export const Info = {startTime:startDate.getTime(),model:{},
    actions:{updateLeftMenu:updateLeftMenu,updateParameters:updateParameters},
    elements:{},
    gridParameters:{}
}

// export const AddCounter = () => {

// };

