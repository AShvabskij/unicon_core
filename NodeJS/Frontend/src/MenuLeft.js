import 'webix/webix.css';
import WebixComponent, {scroll} from './WebixComponent';
import React,{useEffect} from "react";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';
import {Context} from './Context';
import { observer } from "mobx-react"

const accordionInit = [
	{ header:"Graphic trends", id:"GraphicTrends", body: ""},
	{ header:"PLC", id:"PLC", body: "" },
	{ header:"cPlotWeb", id:"cPlotWeb", body: ""},
	{ header:"Devices", id:"DeviceInit" ,body: "" }
];

function setScroll(body) {
  return {
    view:"scrollview",
    scroll:"y", // vertical scrolling
    body:body
  }
}



 function accordion() {
  //  let v = scroll(buttons(),"qwe1");
  //  v= buttons();
  return {
    view:"accordion",
    // width: 0,
    id:"accmain",
    height:0,
    multi : false,
    scroll: "y",
    collapsed:true,
    type:"wide",
    select:true,
    rows:[
        // {header:"col 1", body:"text"},
        // { body:"Content 2", height: 35}
    ],
    on:{
      onChange: function(newValue, oldValue, config){
          // console.log(newValue)
      },
      onAfterExpand:function(id){
          let changeId = $$(id);
          let newHeight = 42;
          switch(id)  {
            case "DeviceInit":
              newHeight = 0;
              Context.showPages(["ViewDevices"]);
              break;

            case "cPlotWeb":
              Context.showPages(["ViewCPlotWeb","chartCPlotWeb"]);
              break;

            case "PLC":
              Context.showPages(["ViewPLC"]);
              break;

            case "GraphicTrends":
              Context.showPages(["ViewGraphicTrends","chartTrends"]);
              break;
            default:
              break;
          }
          changeId.config.height = newHeight;
          changeId.resize();
      }
    }
  }
}




const updateLeftMenuBase = (devicesArr) => {
  let options = [];
  let minWidthBut = "130";
  // if ((devicesArr.length % 3) == 0) minWidthBut = 95;
  let devices = { margin:10, padding:0, type:"wide",
  view:"flexlayout",cols:[]};
  devicesArr.forEach(function(item, index, array) {
      devices.cols.push( { view:"toggle", deviceID: item.id ,label:'<span class="material-icons">' +  '</span> ' + item.name , minWidth: minWidthBut, height: 80, css: "webix_primary",
      click:function(id,event){
               // Изменяем текущий номер устройства
              Context.states.indexDevice = devices.cols[index].deviceID;
          // Подсветка нужной кнопки при нажатии и отжатие остальных
          let s1 = $$(id).getParentView();
          s1._cells.forEach(element => {
              element.setValue(0);
          });
      }
      })
});
  let scrollDev = setScroll(devices);
  webix.ui(scrollDev,$$("DeviceInit"), 0);
} 

function BaseMenuLeft(props) {
  return ( 
    <MenuLeft/>
  );
}

const MenuLeft = observer(({  }) => {
  useEffect(() => {
    // console.log("Render MenuLeft");
    // console.log(Context.model.devices());
    // console.log(Context.model.m_devices);
    updateLeftMenuBase(Context.model.m_devices);
  })
  return ( 
      <WebixComponent ui={accordion()} data={accordionInit} updateModel={ Context.updateModel  } />
  );
});

export default BaseMenuLeft;
