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
import ParametersView from './ParametersView';
import Config from './.config.js';
import { Model } from "./data_model/fr_model.mjs";
import { SysInterfacesEnum } from "./data_model/fr_model.mjs";
import { AddCounter, Info } from "./Context"

let model = new Model(Config.ip);
Info.model = model;
model.init();
console.log("model.m_devices");
console.log(model.m_devices);

setTimeout(() => {
  // console.log("setTimeout result");
  // console.log(model.load);
  model.load().then(result => {
    console.log("loadDataModel result");
    // console.log(model.m_devices);
    Info.actions.updateLeftMenu();
  }, error => {
    console.log("loadDataModel error");
    console.log(error);
  });
}, 1000);

const chartsArrVisible = [];
const chartsArrHidden = ["chart3","chart4","chart5"];

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
  if (line == 1) {
    chart.addVarPointRange(xValues, yValues)
  } else {
    chart.addVarPointRange2(xValues, yValues)
  }
  /*
    _interval = _interval >= 15 ? 5 : _interval + 5;
    setTimeout(() => {
      if (line == 1) {
        chart.addVarPointRange(xValues, yValues)
      } else {
        chart.addVarPointRange2(xValues, yValues)
      }
    }, _interval);
  */
}

async function startOsc(deviceId, line) {
  console.log("try to get osc streams...");

  // let param = device.param(deviceId + 1);
  let streamSocketUrl = "ws://" + Config.ip + ":" + 1237;
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

      var diff = (endDate.getTime() - Info.startTime)
      console.log("The work time is = " + diff);
    }
  }

  //  console.log(output + "\n");
};

async function startOsc2(deviceId, line) {
  console.log("try 2 to get osc streams...");

  // let param = device.param(deviceId + 1);

  let osc = model.device(deviceId).osc;
  console.log('osc config: ');
  osc.channels.forEach(channel => {
    console.log(`channel = ${channel.num}, param id = ${channel.param_id}, name = ${channel.name}`);
  });

  osc.openDataStream();

  let charts = ChartControls();

  let chNum1 = 2;
  console.log(`Drawing osc line 1 for the channel = ${osc.channels[chNum1].num}, 
  param id = ${osc.channels[chNum1].param_id}, 
  name = ${osc.channels[chNum1].name}`);

  osc.channels[chNum1].stream.on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });

    drawValueRange(charts['chart3'], xValues, yValues, 1);
  });

  let chNum2 = 11;
  console.log(`Drawing osc line 2 for the channel = ${osc.channels[chNum2].num}, 
  param id = ${osc.channels[chNum2].param_id}, 
  name = ${osc.channels[chNum2].name}`);

  osc.channels[chNum2].stream.on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });

    drawValueRange(charts['chart3'], xValues, yValues, 2);
  });
}

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
    id: "tabview1",
    height: 800,
    css:"tabParam",
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
          if (id == "oscilloscopeContent") {
            showChart("chart2", "memo1");
            // showElementChart("chart3");
            // showElementChart("chart4")
            // showElementChart("chart5")
          }

          if (id == "controlContent") {
            showChart("chart1", "memo2")
          }
          Info.resize();
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

const showSelectChartWindow = () => {
  let v = {
    view:"window",
    id:"showSelectChartWindow",
    height:600,
    width:1000,
    left:50, top:50,
    // css:"showSelectChartWindowData",
    move:true,
    modal:true,
    head:"This window can be moved",
    body:{
      rows: [
        {
          id: "showSelectChartWindowData",   
          view:"datatable",
          height:100, 
          columns:[
            { id:"status", header:"Is Active", width:80, css:"center", 
              template:"{common.checkbox()}"},
            { id:"value", header:"Records", fillspace:1 },
          ],
          data: [
            { id:1, status:0, value:"Record A"},
            { id:2, status:1, value:"Record B"},
            { id:3, status:0, value:"Record C"}
          ]
        },
        {
          height: 38,
          cols: [
            { "label": "Cancel", "view": "button", "height": 0, 
                click: function (id, event) {
                  // console.log("webixButton");
                  $$("showSelectChartWindow").hide();
                }
            },
            { "label": "Apply", "view": "button", "height": 0, 
                click: function (id, event) {
                  // then form process — add
                  
                  // if (chartsArrHidden.length > 0) {
                    let chartToVisible = chartsArrHidden.shift();
                    let paramArr = ["Param 1","Param 2","Param 3","Param 4"];
                    Info.chartList[chartToVisible].setNamesArr(paramArr);
                    // Info.chartList[chartToVisible].namesArr = ["Param 1","Param 2","Param 3","Param 4", "Param 5", "Param 6"];
                    chartsArrVisible.push(chartToVisible);
                    document.documentElement.style.setProperty('--chartcount', chartsArrVisible.length);
                    showElementChart(chartToVisible);
                  // }
                  $$("showSelectChartWindow").hide();
                }
            }
          ]
        }
      ]
    }
  };
  return v;
}

const addButtonClick = () => {
  // console.log("addButtonClick");
  
  let comp = $$("showSelectChartWindowData");
  console.log(comp);
  let dataCollection = [
    { id:1, status:0, value:"Record A1"},
    { id:2, status:1, value:"Record B1"},
    { id:3, status:0, value:"Record C1"}
  ];
  
  if (chartsArrHidden.length > 0) {
    $$("showSelectChartWindow").show();
    comp.define({"data":dataCollection});
    
    // console.log("indexDev " + Info.states.indexDevice);

    showSelectChartWindow();
  }
}

const removeButtonClick = () => {
  // console.log("removeButtonClick");
  if (chartsArrVisible.length > 0) {
    let chartToHidden = chartsArrVisible.pop();
    chartsArrHidden.unshift(chartToHidden);
    
    document.documentElement.style.setProperty('--chartcount', chartsArrVisible.length);
    showElementChart(chartToHidden, "hidden");
  }
  // then stop process — add
}

function addFunction(x) {
  // console.log("addFunction");
  return Math.sin(x * 0.01) * (1 + 0.5 * Math.random());
}

// function addFunction1(x) {
//   // console.log("addFunction");
//   return Math.sin(x * 0.01) * Math.cos(x * 0.01) * (1 + 0.5 * Math.random());
// }

export default class DevicesView extends React.Component {
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

    return (
      <div>
        <Webix ui={{ "label": " ", "view": "label", "css":"deviceLabel", "id":"descriptionDevice"}} />
        {/* <Webix ui={webixButton(150)} id="q1" data="Get Devices" click={() => {
          let acc = $$("accmain");
          acc.adjust();
          acc = $$("tabview1");
          acc.adjust();
          window.addEventListener('resize', function (event) {
            // let l = $$("descriptionDevice");
            // l.setValue("123");
            let acc = $$("accmain");
            acc.adjust();
            acc = $$("tabview1");
            acc.adjust();
            acc = $$("parametersGrid");
            acc.adjust();

          }, true);
        }
        } /> */}

        <Webix ui={tabview1(this.props)} />
        <Webix ui={showSelectChartWindow()}  />
        <div id="chart2">

          <Chart2 id="chart3" /* title="demo chart 3" */  addFunction={addFunction} />
          <Chart2 id="chart4" title="&nbsp;" addFunction={addFunction} />
          <Chart2 id="chart5" title="&nbsp;" addFunction={addFunction} />
        </div>

        <Chart />
        {/* <ChartList /> */}
        <div id="memo1">{/* Memo 1 */}
          <Webix ui={toolBar()} />
        </div>
        <div id="memo2">Memo 2</div>
        <div id="memo3">Memo 3</div>
        <div id="memo4">Memo 4</div>
        <ParametersView data={this.state.dt} />
      </div>
    )
  }
};


// export default MenuCenter;