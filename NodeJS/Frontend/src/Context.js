import React, {useState,useEffect} from "react";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';

// export const Context = React.createContext();
let startDate = new Date();

const updateParameters = (indexDevice) => {
    // console.log(Info);
    // console.log("indexDevice="+indexDevice);
   
    let deviceItem = Info.model.devices()[indexDevice];
    
    let dt1 = [];
    let dtt = [];
    let i = 1;
    let j = 1;
    let infoCurrentDivice = deviceItem.desc + ". Channel: " + deviceItem.interfaceName;
    Info.states.indexDevice = indexDevice;
    let desc = $$("descriptionDevice");
    desc.setValue(infoCurrentDivice);
    deviceItem.modules.forEach(function(item, index, array) {
        // console.log(item.params);
        dtt = [];
        item.params.forEach(function(itemP, indexP, array) {
            infoCurrentDivice = infoCurrentDivice + " [" + Number(itemP.deviceId).toString(16) + "]" + "</br>Chanal: " + item.interfaceName;
            // if(=="Can")
            dtt.push({id: "m"+i, // name: itemP.moduleId,  //"[" + itemP.deviceId + "] " + item.name + " [" + itemP.moduleId + "]", 
            name:itemP.name + " ["+ Number(itemP.moduleId).toString(16) + "." + Number(itemP.id).toString(16) + "]", 
            value:" ", dimension:itemP.unit, time:" ", 
            desc:itemP.desc,
            chart:0, 
            numchart:"",
            param:itemP,
            // rw: itemP.RW,
        })
            i++;
        });
        dt1.push({"id":"modul"+j, "value":"", "name": item.name + " [" + Number(item.id).toString(16) + "]",
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
        devices.cols.push( { view:"toggle", label:item.name + "</br>Channel: " + item.interfaceName, minWidth: 120, height: 80, css: "webix_primary", modules: item.modules,
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
    $$("descriptionDevice").$view.children[0].style.color = "white";
} 

const updateLeftMenu = () => {
    updateLeftMenuBase(Info.model.devices())
}

export const Info = {startTime:startDate.getTime(),model:{},
    actions:{updateLeftMenu:updateLeftMenu,updateParameters:updateParameters},
    elements:{},
    states:{indexDevice:1},
    gridParameters:{},
    paramToChart:[],
    paramToCharts:{"chart3":{}, "chart4":{}, "chart5":{}},
    chartList:{"chart3":{setNamesArr: () => {}, setColorsArr: () => {}}, 
            "chart4":{setNamesArr: () => {}, setColorsArr: () => {}}, 
            "chart5":{setNamesArr: () => {}, setColorsArr: () => {}}
        },
    resize :function(event) {
        
        let elements = ["accmain","tabview1","parametersGrid","descriptionDevice"]
        elements.forEach(function(item, index, array) {
            //if(item == "controlview") $$(item).resize();
           $$(item).adjust();
        });
    }
}

// export const AddCounter = () => {

// };

