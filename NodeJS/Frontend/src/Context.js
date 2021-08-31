import React, {useState,useEffect} from "react";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';

export const Context = React.createContext();
let startDate = new Date();
const interfaceName = {1: 'Can', 2: 'MBus', 3: 'FO'};

const updateLeftMenuBase = (devicesArr) => {
    let options = [];
    // devices.forEach(function(item, index, array) {
    //     // console.log(item, index);
    //     let label = item.name + "</br>Channal: " + interfaceName[item.interface]
    //     options.push( { id:index, value:label, css:"devices"})
 
    //     }); 
    // let comp = {view:"segmented", css:"devices", height:100, multiview:true, value:1, options:options,
    //     click:function(id,event){
    //         // console.log(id)
    //     },
    //     on:{
    //         onChange: function(newValue, oldValue, config){
    //             console.log(newValue)
    //         },
    //         onItemClick:function(id,ev){
    //             console.log(id)
    //             console.log(ev)
    //         }
    //     }
    // }

    let  devices = { margin:10, padding:0, type:"wide",
    view:"flexlayout",cols:[]};
    devicesArr.forEach(function(item, index, array) {
        // console.log(item, index);
        devices.cols.push( { view:"toggle", label:item.name + "</br>Chanal: " + interfaceName[item.interface], minWidth: 110, height: 70, css: "webix_primary", modules: item.modules,
        click:function(id,event){
            // console.log(id,event);
            // console.log($$(id));
            let s = $$(id);
            console.log(s.config.modules);
            let dt1 = [];
            let dtt = [];
            let i = 1;
            let j = 1;
            let infoCurrentDivice = item.name;
            s.config.modules.forEach(function(item, index, array) {
                console.log(item.params);
                dtt = [];
                item.params.forEach(function(itemP, indexP, array) {
                    infoCurrentDivice = infoCurrentDivice + " [" + itemP.deviceId + "]" + "</br>Chanal: " + interfaceName[item.interface];
                    // if(=="Can")
                    dtt.push({id: "m"+i, modul: itemP.moduleId,  //"[" + itemP.deviceId + "] " + item.name + " [" + itemP.moduleId + "]", 
                    name:itemP.name + " [" + itemP.id + "]", 
                    value:"1.008", dimension:"W", time:"11:56", chart:"+", numchart:1})
                    i++;
                });
                dt1.push({"id":"modul"+j, "modul":"[" + item.deviceId + "] " + item.name + " [" + item.id + "]",
                "open":false, "data":dtt 
                });
                j++;
            });
            console.log(dt1);
            //  console.log(component);

            Info.elements.menuTop.setState((state, props) => ({
                    // dt: [{"id":"can","programmInt":"Can", "open":"false", "data":dt1}]
                    dt: {"id":"can","data":dt1}
                    }));

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

export const Info = {startTime:startDate.getTime(),model:{},actions:{updateLeftMenu:updateLeftMenu},elements:{}}

export const AddCounter = () => {

};

