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
	{ header:"Devices", id:"DeviceInit" ,body: "" },
  // { header:"Devices1", id:"DeviceInit1" ,body: "" },
  // { header:"Devices2", id:"DeviceInit2" ,body: "" },
  // { header:"Devices3", id:"DeviceInit3" ,body: "" },
  // { header:"Devices4", id:"DeviceInit4" ,body: "" },
  // { header:"Devices5", id:"DeviceInit5" ,body: "" },
  // { header:"Devices6", id:"DeviceInit6" ,body: "" },
  // { header:"Devices7", id:"DeviceInit7" ,body: "" },
  // { header:"Devices8", id:"DeviceInit8" ,body: "" },
  // { header:"Devices9", id:"DeviceInit9" ,body: "" },
];

function showPage(pageID, parentID = "") {
  var sc = document.getElementById(pageID);
  sc.style.setProperty("visibility", "visible");
}

function hidePages(arrPages,visibeElements) {
  arrPages.forEach(element => {
    document.getElementById(element).style.display = "none";
  });
  visibeElements.forEach(element => {
    document.getElementById(element).style.display = "";
    document.getElementById(element).style.visibility = "visible";
  });
}

function buttons(params) {
  return {view:"scrollview",
    id:"verses",
    scroll:"y", // vertical scrolling
    
    body:{
    rows:[{
      view:"button", 
      id:"my_button0", 
      value:"Button", 
      css:"webix_primary", 
      inputWidth:100 
  },
  {
    view:"button", 
    id:"my_button1", 
    value:"Button", 
    css:"webix_primary", 
    inputWidth:100 
},
{
  view:"button", 
  id:"my_button2", 
  value:"Button", 
  css:"webix_primary", 
  inputWidth:100 
},{
  view:"button", 
  id:"my_button3", 
  value:"Button", 
  css:"webix_primary", 
  inputWidth:100 
},
{
  view:"button", 
  id:"my_button4", 
  value:"Button", 
  css:"webix_primary", 
  inputWidth:100 
}]
    }}
}

 function accordion() {
   let v = scroll(buttons(),"qwe1");
   v= buttons();
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
        {header:"col 1", body:v},
        { body:"Content 2", height: 35}
    ],
    on:{
      onChange: function(newValue, oldValue, config){
          console.log(newValue)
      },
      // onAfterExpand:function(id){
      //     // console.log("onAfterExpand")
      //     let pages = ["deviceView","ViewCPlotWeb","ViewPLC","ViewGraphicTrends"];
      //     console.log(id)
      //     // console.log($$(id)) 
      //     let changeId = $$(id);
      //     let newHeight = 42;
      //     switch(id)  {
      //       case "DeviceInit":
      //         newHeight = 500;
      //         hidePages(pages,["deviceView"]);
      //         break;

      //       case "cPlotWeb":
      //         hidePages(pages,["ViewCPlotWeb","chartCPlotWeb"]);
      //         break;

      //       case "PLC":
      //         hidePages(pages,["ViewPLC"]);
      //         break;

      //       case "GraphicTrends":
      //         hidePages(pages,["ViewGraphicTrends","chartTrends"]);
      //         break;
      //     }
      //     changeId.config.height = newHeight;
      //     changeId.resize();
      // }
    }
  }
}



function demo(params) {
 return {
    view: "scrollview",
    id:"accmain",
    scroll: "y",
    body: {
      type:"wide", 
      cols:[
        { width:30 },
        { type:"wide", 
         rows: [
           { height: 30 },
           {
             multi:true,
             view:"accordion", type:"wide",
             rows:[
               { header:"col 1", body:"content 1", height:150},
               { body:"Content 2", height: 35},
               { 
                 collapsed:false, 
                 header:"col 3",
                 body:{
                   multi:true,
                   view:"accordion", type:"space",
                   cols:[
                     { header:"col 1", body:"content 1", width:150},
                     { body:"Content 2" },
                     { 
                       collapsed:false, 
                       header:"col 3",
                       body:"content 3",
                       width:150
                     },
                     { body:"Content 4" },
                     { header:"col 5", body:"content 5", width:150}
                   ]
                 },
                 height:150
               },
               { body:"Content 4", height: 35 },
               { header:"col 5", body:"content 5", height:150}
             ]
           },
           {
             multi:false,
             view:"accordion", type:"wide",
             cols:[
               { header:"col 1", body:"content 1", width:150 },
               { body:"Content 2"},
               { 
                 header:"col 3",
                 body:"content 3",
                 width:150
               },
               { body:"Content 4" },
               { header:"col 5", body:"content 5", width:150}
             ]
           },
           { height: 30 }
         ]
        },
        { width:30 }
      ]
    }
  }
}

const updateLeftMenuBase = (devicesArr) => {
  let options = [];
  let minWidthBut = 170;
  if ((devicesArr.length % 3) == 0) minWidthBut = 95;
  let devices = { margin:10, padding:0, type:"wide",
  view:"flexlayout",cols:[]};
  devicesArr.forEach(function(item, index, array) {
      // console.log(item, index);
      devices.cols.push( { view:"toggle", label:'<span class="material-icons">' +  '</span> ' + item.name , minWidth: minWidthBut, height: 80, css: "webix_primary",
      // click:function(id,event){
      //     let tree = $$("parametersGrid");
      //     let arr = tree.getOpenItems();
      //     arr.forEach(function (item) {
      //       tree.close(item);
      //     });
      //     tree.clearAll();

      //     updateParameters(index);
          
      //     let s1 = $$(id).getParentView();
      //     s1._cells.forEach(element => {
      //         element.setValue(0);
      //     });
      // }
      })
});
 
  webix.ui(devices,$$("DeviceInit"), 0);
  // $$("descriptionDevice").$view.children[0].style.color = "white";
} 

// const TimerView = observer(({  }) => {
//   useEffect(() => {
//       console.log("Render TimerView");
//   })
//   return (
//   <button >{Context.title} Seconds passed abc: {Context.title}</button>
//   )
// });

// function MenuLeft(props) {
const MenuLeft = observer(({  }) => {
  // let devArr = [{name:"dev1"},{name:"dev2"},{name:"dev3"}];
  useEffect(() => {
    console.log("Render MenuLeft");
    updateLeftMenuBase(Context.devices);
  })
  return ( 
      <WebixComponent ui={accordion()} data={accordionInit} devices={Context.devices} />
  );
});

export default MenuLeft;
