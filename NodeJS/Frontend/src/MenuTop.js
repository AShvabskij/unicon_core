import 'webix/webix.css';
import Webix from './Webix';
import * as webix from 'webix/webix.js';
// import Chart from "./Chart2";
import React from "react";
import Chart from './ChartInteract';
import Chart2 from './Chart2';
import ChartList from './ChartList';
import DataView from './DataView';

import { Model } from "./data_model/fr_model.mjs";
import { SysInterfacesEnum } from "./data_model/fr_model.mjs";


let model = new Model();

  async function loadDataModel() {

    console.log("try to load data model...");

    model.clear();

    model.load().then(result => {
      console.log(result);
      // model.enablePeriodicCheck();      
    }, error => {
      console.log(error);
    });

    model.on('system_status', function(res) {
      console.log(`System status changed to ${res}`);
      model.disablePeriodicCheck();      
    });

  }

async function getValue(deviceId, paramId) {

    console.log("try to get value...");
    let startDate = new Date();
    
    model.device(deviceId).param(paramId).lastValue().then(result => {
      var paramValue = result;
      var message = `param id = ${paramValue.paramId}, value = ${paramValue.value}, value format = ${paramValue.valueFormat}`;
      console.log(message);

    });

    let param = model.device(deviceId).param(paramId);
    await param.closeValueStream();
    let resStream = await param.openValueStream();
  
    resStream.on('data', chunk => {
      let stringifiedRes = chunk.toString();
      // console.log(`Received from stream: ${stringifiedRes}`);
      let value = JSON.parse(stringifiedRes);
      // let xValue = (value.valueTime & 0xFFFF) * 0.05;
      let xValue = value.valueTime - startDate.getTime();
      let yValue = value.value;
      console.log (startDate.getTime());
      // console.log (xValue +","+ yValue);
      if (yValue != -1) {
        window.chartEvents["chart3"].addVarPoint(xValue,yValue);
      }
    });
  }


const paramData = [
	{ id:9, num: "1", name:"Parameter 1 (2110)", value:"1.008", dimension:"W", time:"11:56",chart:"+",numchart:1},
	{ id:10, num: "2", name:"Parameter 2 (2120)", value:"2.7896", dimension:"A", time:"11:56",chart:"+",numchart:1},
	{ id:11, num: "3", name:"Parameter 3 (2130)", value:"8", dimension:"kHz", time:"11:00",chart:"+",numchart:"2"},
	{ id:7, num: "4", name:"Parameter 4 (2140)", value:"356", dimension:"NO/NC", time:"11:20",chart:"–",numchart:""}
];

function disableEnableElement(id,show) {
    const elem = document.getElementById(id);
        if (elem != null) {
            if (show) {
              elem.style.display = 'block';
            } else {
              elem.style.display = 'none';
            }
      }
}

function getUImainMenu(props) {
  return {
    "view": "tabbar",
    "options": [
      { value:"Parameters", id:"dataview",  icon:"wxi-pencil" },
      { value:"Оscilloscope ", id:"scichart-root",  icon:"wxi-pencil" },
      { value:"Control", id:"device_manage",  icon:"wxi-pencil" },
      { value:"Info", id:"data_m",  icon:"wxi-pencil" },
    ],
    
    on:{
      onChange: function(newValue, oldValue, config){
        // config is {yourProperty: "yourValue"} 
        console.log(this);
        console.log(oldValue+" " +newValue);
        console.log(props);
        const elem = document.getElementById(newValue);
        disableEnableElement(oldValue,false);
        disableEnableElement(newValue,true);
      }
      }

  }
}

function showElementChart(chartID,visibility = "visible") {
  var sc =  document.getElementById(chartID);
  sc.style.setProperty("visibility",visibility);
}

function showChart(chartID,parentID) {
  var sc =  document.getElementById(chartID);
  sc.style.setProperty("visibility","visible");
  const m =  document.getElementById(parentID);
  if (sc.parentElement.id != parentID) {
    m.appendChild(sc)
  }
  console.log(sc.parentElement);
  
}

function tabview1(props) {
  return {
    view:"tabview",
    height: 800,
    pading: "0",
    cells:[     
      {
        id: "parameters",
        header:"Parameters",
        body:{
          id:"parametersContent",  
          view:"htmlform",
          content: "dataview" ,
          
          // view:"datatable",
          // scroll:"y",
          // select:true,
          // height:400,
          // hover:"myhover",
          // columns:[
          //   {id:"num", header:"Number"},
          //   {id:"name", header:"Name", width:"300"},
          //   {id:"value", header:"Value", width:"130"},
          //   {id:"dimension", header:"Dimension"},
          //   {id:"time", header:"Time"},
          //   {id:"chart", header:"Show on chart", width:"150"},
          //   {id:"numchart", header:"Number of chart", width:"200"}           
          // ]
          // template:"<div id='f1'>Form Content<div>"     
        }      
      },
      { 
        id: "oscilloscope",
        header:"Оscilloscope", 
        body:{
          id:"oscilloscopeContent",  
          view:"htmlform",
          content: "memo1" 
        } 
      },
      { 
        id: "control",
        header:"Control", 
        body:{
          id:"controlContent",  
          view:"htmlform",
          content: "memo2" 
        } 
      },
      { 
        id: "info",
        header:"Info", 
        body:{
          id:"infoContent",  
          view:"htmlform",
          content: "memo3" 
        } 
      }
    ],
    tabbar:{
      on:{
        
        onAfterTabClick:function(id,ev){
          // webix.message("tab was clicked 12345"+this.getValue());
          // console.log("----->>>>>>111111");
          // console.log("onAfterTabClick 123"); 
          // console.log(id)
          // console.log("onAfterTabClick 5"); 
          if (id=="oscilloscopeContent") {
            showChart("chart2","memo1");
            showElementChart("chart3");
            showElementChart("chart4")
          
          }

          if (id=="controlContent") {
            showChart("chart1","memo2")
          }

        },
        onChange: function(newValue, oldValue, config){
          // config is {yourProperty: "yourValue"}
          console.log(this);
          console.log("----->>>>>>"+this.getValue());
          // var chart1 = document.getElementById("wwwqqq");
          // console.log(chart1);
          //avp.updateDevice();
        }
      }
    },
    on:{
      onChange: function(newValue, oldValue, config){
        // config is {yourProperty: "yourValue"}
       //console.log(this);
        console.log("!!!!!----->>>>>>"+newValue);
        console.log("!!!!!----->>>>>>"+oldValue);
        // var chart1 = document.getElementById("wwwqqq");
        // console.log(chart1);
        //avp.updateDevice();
    }
  }
}
}

const MenuCenter1 = ({ data }) => (
  <div>
    <Webix ui={getUImainMenu(data)} data={data} />
      {/* <Chart/> */}
    </div>
   
)

const toolBar = () => {
  return {
    view:"toolbar",
    id:"myToolbar",
    cols:[
        { view:"button", value:"Add Chart", width:100, align:"left",
        click:function(id,event){
          addButtonClick();
        }    
      
        },
        { view:"button", value:"Remove Chart", autowidth: true, align:"center" ,
          click:function(id,event){
            removeButtonClick();
          } 
        },
        { view:"button", value:"Start", autowidth: true, align:"center" ,
          click:function(id,event){
            console.log(window.chartEvents);
            for (var chart in window.chartEvents) {
              // console.log(chart);
              window.chartEvents[chart].startDemo();
            }
          } 
        },
        { view:"button", value:"Stop", autowidth: true, align:"center" ,
          click:function(id,event){
            console.log(window.chartEvents);
            for (var chart in window.chartEvents) {
              // console.log(chart);
              window.chartEvents[chart].stopDemo();
            }
          } 
        },

        { view:"button", value:"Load data", autowidth: true, align:"center" ,
          click:function(){
            console.log("Load data");
            loadDataModel();
          } 
        },

        { view:"button", value:"Get values", autowidth: true, align:"center" ,
          click:function(id,event){
            console.log("Get values");
            let deviceId = 12;
            let paramId = 65;
            getValue(deviceId, paramId);
          } 
        },
     
      ]
}
}

const webixButton = (props={width:"100"}) => {
  return (
    {
      view:"button", 
      value:"Button", 
      css:"webix_primary", 
      inputWidth:props.width ,
      click:function(id,event){
        console.log("webixButton");
        console.log(this.onclickMessage);
        this.onclickMessage();
      }     
  }
  )
}

const addButtonClick = () => {
  console.log("addButtonClick");
  showElementChart("chart4");
}

const removeButtonClick = () => {
  console.log("addButtonClick");
  showElementChart("chart4","hidden");
}

function addFunction(x) {
  // console.log("addFunction");
   return Math.sin(x* 0.01)*(1+0.5*Math.random());
 }

function addFunction1(x) {
  // console.log("addFunction");
   return Math.sin(x* 0.01)*Math.cos(x* 0.01)*(1+0.5*Math.random());
}

export default class MenuTop extends React.Component {
  constructor(props) { 
    super(props);
    this.title = "first title"
    this.state = { title: "state title" };
  };
  render() {
    return(
        <div>
        <Webix ui={tabview1(this.props)} data={this.props} />
        <div id="chart2">
{/*             
            <Webix ui={webixButton({width:"150"})} data="Add Chart" click={addButtonClick} />
            <Webix ui={webixButton({width:"150"})} data="Remove Chart" click={removeButtonClick} /> */}
            <Chart2 id="chart3" title="demo chart 3" addFunction={addFunction}/>
            <Chart2 id="chart4" title="demo chart 4" addFunction={addFunction1}/>
        </div>
        
        <Chart/>
        <ChartList/>
      <div id="memo1">Memo 1
            <Webix ui={toolBar()} data="Add Chart" click={addButtonClick}  />
      </div>
      <div id="memo2">Memo 2</div>
      <div id="memo3">Memo 3</div>
      <div id="memo4">Memo 4</div>
        <DataView/>
        </div>
    )
  }
}


// export default MenuCenter;