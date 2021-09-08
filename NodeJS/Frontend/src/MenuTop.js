import 'webix/webix.css';
import Webix from './Webix';
import { $$ } from 'webix';
import * as webix from 'webix/webix.js';
// import Chart from "./Chart2";
import React from "react";
import Chart from './ChartInteract';
import Chart2 from './Chart2';
import { ChartControls } from './Chart2';
import ChartList from './ChartList';
import DataView from './DataView';

import { Model } from "./data_model/fr_model.mjs";
import { SysInterfacesEnum } from "./data_model/fr_model.mjs";
import { AddCounter, Info } from "./Context"

let model = new Model('127.0.0.1');
Info.model = model;
model.init();

setTimeout(() => {
  model.load().then(result => {
    console.log("loadDataModel result");
    console.log(model.m_devices);
    // Info.actions.setDevicesName(leftMenuInfo1());

    Info.actions.updateLeftMenu();
    // loadDataInterface(model.m_devices);
    // model.enablePeriodicCheck();      
  }, error => {
    console.log(error);
  });
}, 1000);

const interfaceName = { 1: 'Can', 2: 'MBus', 3: 'FO' };



async function loadDataModel() {

  console.log("try to load data model...");

  model.clear();

  model.load().then(result => {
    // console.log("loadDataModel1 result");
    // console.log(model.m_devices);
    // // loadDataInterface(model.m_devices);
    // Info.actions.updateLeftMenu(model.m_devices);
    // model.enablePeriodicCheck();      
  }, error => {
    console.log(error);
  });

  model.on('system_status', (res) => {
    console.log(`System status changed to ${res}`);
    model.disablePeriodicCheck();
  });

}


async function stopValues(deviceId, paramId) {
  let param = model.device(deviceId).param(paramId);
  await param.closeValueStream();
}

async function startValues(deviceId, paramId, line) {
  console.log("try to get value streams...");
  let startDate = new Date();

  let param = model.device(deviceId).param(paramId);
  await param.closeValueStream();

  let resStream = await param.openValueStream();
  if (resStream === undefined || resStream === null) {
    console.error("Что-то пошло не так...");
  }

  resStream.on('data', chunk => {

    let values = chunk;
    let xValues = [];
    let yValues = [];

    for (var j = 0; j < values.length; j++) {
      let value = values[j];

      // let xValue = value.valueTime - startDate.getTime();
      let xValue = value.valueTime - Info.startTime;
      let yValue = value.value;

      if (yValue == -1) {
        break;
      }

      xValues.push(xValue);
      yValues.push(yValue);
    }

    if (xValues.length == 0 || yValues.length == 0) {
      return;
    }

    let charts = ChartControls();
    for (var chart in charts) {
      drawValueRange(charts[chart], xValues, yValues, line);
    }
  });
}

let _interval = 5;
function drawValueRange(chart, xValues, yValues, line) {
  _interval = _interval >= 15 ? 5 : _interval + 5;
  setTimeout(() => {
    if (line == 1) {
      chart.addVarPointRange(xValues, yValues)
    } else {
      chart.addVarPointRange2(xValues, yValues)
    }
  }, _interval);
}

async function startOsc(deviceId, line) {
  console.log("try to get osc streams...");

  // let param = device.param(deviceId + 1);
  let streamSocketUrl = "ws://" + "127.0.0.1" + ":" + 1237;
  let streamSocket = new WebSocket(streamSocketUrl);

  let startDate = new Date();

  let osc = model.device(deviceId).osc;

  streamSocket.onopen = async (event) => {
    console.log(`Stream socket ${streamSocket.url} opened successfully.`);
    osc.openDataStream();
    startDate = new Date();
  };

  streamSocket.onmessage = (message) => {

    var messageData = JSON.parse(message.data);

    let res = osc.parse(messageData, 0);
    if (res < 0) {
      return;
    }

    let chNum = 0;
    let charts = ChartControls();
    let endOfData = false;
    for (var chart in charts) {
      for (var i = 1; i <= 2; ++i) {
        let values = osc.parse(messageData, chNum++);
        if (values == null) {
          endOfData = true;
          continue;
        }

        let xValues = values.map(valObj => { return valObj.time });
        let yValues = values.map(valObj => { return valObj.val });

        if (i == 1) {
          // console.log("values() = " + JSON.stringify(values));
        }
        if (chart == 'chart3') {
          drawValueRange(charts[chart], xValues, yValues, i);
        }

      }

    }

    if (endOfData) {
      streamSocket.close();
      let endDate = new Date();

      var diff = (endDate.getTime() - startDate.getTime())
      console.log("The work time is = " + diff);
    }
  }

  //  console.log(output + "\n");
};

async function startOsc2(deviceId, line) {
  console.log("try 2 to get osc streams...");

  // let param = device.param(deviceId + 1);

  let osc = model.device(deviceId).osc;
  osc.openDataStream();

  let charts = ChartControls();

  let chNum = 0;
  osc.channelStreams[chNum].on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });

    drawValueRange(charts['chart3'], xValues, yValues, 1);
  });

  chNum = 1;
  osc.channelStreams[chNum].on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });

    drawValueRange(charts['chart3'], xValues, yValues, 2);
  });

  osc.channelStreams[2].on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });

//    console.log (xValues[0] +','+ yValues[0]);

  });

  osc.channelStreams[3].on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });

//    console.log (xValues[0] +','+ yValues[0]);
  });

  osc.channelStreams[4].on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });
    
//    console.log (xValues[0] +','+ yValues[0]);
  });

  osc.channelStreams[5].on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });
    
//    console.log (xValues[0] +','+ yValues[0]);
  });

}

function devicesWebix(devicesArr, component) {
  console.log("devicesWebix");
  //   console.log(devicesArr);
  console.log("interfaceName");
  // console.log(model);
  console.log(interfaceName);
  let devices = {
    margin: 10, padding: 0, type: "wide",
    view: "flexlayout", cols: []
  };
  devicesArr.forEach(function (item, index, array) {
    // console.log(item, index);
    devices.cols.push({
      view: "toggle", label: item.name + "</br>Chanal: " + interfaceName[item.interface], minWidth: 110, height: 70, css: "webix_primary", modules: item.modules,
      click: function (id, event) {
        // console.log(id,event);
        // console.log($$(id));
        let s = $$(id);
        console.log(s.config.modules);
        let dt1 = [];
        let dtt = [];
        let i = 1;
        let j = 1;
        let infoCurrentDivice = item.name;
        s.config.modules.forEach(function (item, index, array) {
          console.log(item.params);
          dtt = [];
          item.params.forEach(function (itemP, indexP, array) {
            infoCurrentDivice = infoCurrentDivice + " [" + itemP.deviceId + "]" + "</br>Chanal: " + interfaceName[item.interface];
            // if(=="Can")
            dtt.push({
              id: "m" + i, modul: itemP.moduleId,  //"[" + itemP.deviceId + "] " + item.name + " [" + itemP.moduleId + "]", 
              name: itemP.name + " [" + itemP.id + "]",
              value: "1.008", dimension: "W", time: "11:56", chart: "+", numchart: 1
            })
            i++;
          });
          dt1.push({
            "id": "modul" + j, "modul": "[" + item.deviceId + "] " + item.name + " [" + item.id + "]",
            "open": false, "data": dtt
          });
          j++;
        });
        console.log(dt1);
        //  console.log(component);

        Info.elements.menuTop.setState((state, props) => ({
          // dt: [{"id":"can","programmInt":"Can", "open":"false", "data":dt1}]
          dt: { "id": "can", "data": dt1 }
        }));

        let s1 = $$(id).getParentView();
        s1._cells.forEach(element => {
          element.setValue(0);
        });

      }
    })
  });
  return devices;
};


function disableEnableElement(id, show) {
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
      { value: "Parameters", id: "dataview", icon: "wxi-pencil" },
      { value: "Оscilloscope ", id: "scichart-root", icon: "wxi-pencil" },
      { value: "Control", id: "device_manage", icon: "wxi-pencil" },
      { value: "Info", id: "data_m", icon: "wxi-pencil" },
    ],

    on: {
      onChange: function (newValue, oldValue, config) {
        // config is {yourProperty: "yourValue"} 
        console.log(this);
        console.log(oldValue + " " + newValue);
        console.log(props);
        const elem = document.getElementById(newValue);
        disableEnableElement(oldValue, false);
        disableEnableElement(newValue, true);
      }
    }

  }
}

function showElementChart(chartID, visibility = "visible") {
  var sc = document.getElementById(chartID);
  sc.style.setProperty("visibility", visibility);
}

function showChart(chartID, parentID) {
  var sc = document.getElementById(chartID);
  sc.style.setProperty("visibility", "visible");
  const m = document.getElementById(parentID);
  if (sc.parentElement.id != parentID) {
    m.appendChild(sc)
  }
  // console.log(sc.parentElement);

}

function tabview1(props) {
  return {
    view: "tabview",
    height: 800,
    pading: "0",
    cells: [
      {
        id: "parameters",
        header: "Parameters",
        body: {
          id: "parametersContent",
          view: "htmlform",
          content: "dataview",
        }
      },
      {
        id: "oscilloscope",
        header: "Оscilloscope",
        body: {
          id: "oscilloscopeContent",
          view: "htmlform",
          content: "memo1"
        }
      },
      {
        id: "control",
        header: "Control",
        body: {
          id: "controlContent",
          view: "htmlform",
          content: "memo2"
        }
      },
      {
        id: "info",
        header: "Info",
        body: {
          id: "infoContent",
          view: "htmlform",
          content: "memo3"
        }
      }
    ],
    tabbar: {
      on: {

        onAfterTabClick: function (id, ev) {
          // webix.message("tab was clicked 12345"+this.getValue());
          // console.log("----->>>>>>111111");
          // console.log("onAfterTabClick 123"); 
          // console.log(id)
          // console.log("onAfterTabClick 5"); 
          if (id == "oscilloscopeContent") {
            showChart("chart2", "memo1");
            showElementChart("chart3");
            showElementChart("chart4")
            showElementChart("chart5")
          }

          if (id == "controlContent") {
            showChart("chart1", "memo2")
          }

        },
        onChange: function (newValue, oldValue, config) {
          // config is {yourProperty: "yourValue"}
          // console.log(this);
          // console.log("----->>>>>>" + this.getValue());
          // var chart1 = document.getElementById("wwwqqq");
          // console.log(chart1);
          //avp.updateDevice();
        }
      }
    },
    on: {
      onChange: function (newValue, oldValue, config) {
        // config is {yourProperty: "yourValue"}
        //console.log(this);
        // console.log("!!!!!----->>>>>>" + newValue);
        // console.log("!!!!!----->>>>>>" + oldValue);
        // var chart1 = document.getElementById("wwwqqq");
        // console.log(chart1);
        //avp.updateDevice();
      }
    }
  }
}

const toolBar = () => {
  return {
    view: "toolbar",
    id: "myToolbar",
    cols: [
      {
        view: "button", value: "Add Chart", width: 100, align: "left",
        click: function (id, event) {
          addButtonClick();
        }

      },
      {
        view: "button", value: "Remove Chart", autowidth: true, align: "center",
        click: function (id, event) {
          removeButtonClick();
        }
      },
      {
        view: "button", value: "Start", autowidth: true, align: "center",
        click: function (id, event) {
          let charts = ChartControls();
          for (var chart in charts) {
            // console.log(chart);
            charts[chart].startDemo();
          }
        }
      },
      {
        view: "button", value: "Stop", autowidth: true, align: "center",
        click: async function (id, event) {
          let charts = ChartControls();
          for (var chart in charts) {
            // console.log(chart);
            let deviceId = 1;
            let paramId = 65;
            charts[chart].stopDemo();
            await stopValues(deviceId, paramId);
          }
        }
      },

      {
        view: "button", value: "Start values", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Start values");
          let deviceId = 1;
          let paramId = 65;
          startValues(deviceId, paramId, 1);
          startValues(23, paramId + 1, 2);
        }
      },
      {
        view: "button", value: "Start osc", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Start values");
          let deviceId = 1;
          let paramId = 65;
          startOsc2(deviceId, paramId);
        }
      },

      // {
      //   view: "button", value: "Load data", autowidth: true, align: "center",
      //   click: function () {
      //     // console.log("Load data");
      //     loadDataModel();
      //   }
      // },

    ]
  }
}

const webixButton = (props = { width: "100" }) => {
  return (
    {
      view: "button",
      value: "Button",
      css: "webix_primary",
      inputWidth: props.width,
      click: function (id, event) {
        // console.log("webixButton");
        // console.log(this.onclickMessage);
        this.onclickMessage();
      }
    }
  )
}

const addButtonClick = () => {
  // console.log("addButtonClick");
  showElementChart("chart4");
}

const removeButtonClick = () => {
  // console.log("addButtonClick");
  showElementChart("chart4", "hidden");
}

function addFunction(x) {
  // console.log("addFunction");
  return Math.sin(x * 0.01) * (1 + 0.5 * Math.random());
}

function addFunction1(x) {
  // console.log("addFunction");
  return Math.sin(x * 0.01) * Math.cos(x * 0.01) * (1 + 0.5 * Math.random());
}

const leftMenuInfo1 = () => {
  // arr_devices = loadDataInterface([]);
  let arr_devices = [];
  // console.log("model.m_devices");
  // console.log(model.m_devices);
  // console.log(model);

  // arr_devices = loadDataInterface(model.m_devices);

  return (
    [
      { header: "Graphic trends", body: "" },
      { header: "PLC", body: "" },
      { header: "CPLotWeb", body: "" },
      { header: "Devices", id: "DeviceInit", body: devicesWebix(model.m_devices, this) },
    ]
  )
}

export default class MenuTop extends React.Component {
  constructor(props) {
    super(props);
    this.title = "first title"
    this.state = { title: "state title", dt: [] };
    // this.state = dataViewtable([]);
    this.updateDevices = props.updateDevices;

  };


  render() {

    let component = this;
    Info.elements = { ...Info.elements, menuTop: this };
    const leftMenuInfo = () => {
      // arr_devices = loadDataInterface([]);
      let arr_devices = [];
      // console.log("model.m_devices");
      // console.log(model.m_devices);
      // console.log(model);

      // arr_devices = loadDataInterface(model.m_devices);

      return (
        [
          { header: "Graphic trends", body: "" },
          { header: "PLC", body: "" },
          { header: "CPLotWeb", body: "" },
          { header: "Devices", id: "Devices", body: devicesWebix(model.m_devices, component) },
        ]
      )
    }

    // const leftMenuInfo = [
    //   { header:"Graphic trends", body: ""},
    //   { header:"PLC", body: "" },
    //   { header:"Logs", body: ""},
    //   { header:"Devices", id:"Devices", body: devicesWebix([1,2,3,4,5,6,7,8,9,10,112,114,116,118,145,1323])}];


    return (
      <div>
        <Webix ui={webixButton(150)} id="q1" data="Get Devices" click={() => {
          // Info.actions.setDevicesName(leftMenuInfo())
          Info.actions.updateLeftMenu(model.m_devices);
          // this.updateDevices(leftMenuInfo());
          console.log("webixButton click");
          // this.setState((state, props) => ({
          //   dt: dt1
          // }));

        }
        } />

        <Webix ui={tabview1(this.props)} />

        <div id="chart2">

          <Chart2 id="chart3" /* title="demo chart 3" */ addFunction={addFunction} />
          <Chart2 id="chart4" title="&nbsp;" addFunction={addFunction1} />
          <Chart2 id="chart5" title="&nbsp;" addFunction={addFunction1} />
        </div>

        <Chart />
        <ChartList />
        <div id="memo1">{/* Memo 1 */}
          <Webix ui={toolBar()} data="Add Chart" click={addButtonClick} />
        </div>
        <div id="memo2">Memo 2</div>
        <div id="memo3">Memo 3</div>
        <div id="memo4">Memo 4</div>
        <DataView data={this.state.dt} />
      </div>
    )
  }
};


// export default MenuCenter;