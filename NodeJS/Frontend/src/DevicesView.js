import 'webix/webix.css';
import Webix from './Webix';
import { $$, template } from 'webix';
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
import { AddCounter, Info } from "./Context"
import { colorsArr, colorsTitleArr } from './Chart2';

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
  chart.addVarPointRange(xValues, yValues, line)
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

async function startOsc(deviceId) {
  console.log("try 2 to get osc streams...");

  // let param = device.param(deviceId + 1);

  let osc = model.device(deviceId).osc;
  console.log('osc config: ');
  osc.channels.forEach(channel => {
    console.log(`channel = ${channel.num}, param id = ${channel.param_id}, name = ${channel.name}`);
  });

  let channels = [3, 19]
  osc.openDataStream(channels);
  console.log("Start osc for channels: " + channels);

  let charts = ChartControls();
  
  osc.channels[channels[0]].stream.on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });
  //  console.log("val = " + yValues);
  
    drawValueRange(charts['chart3'], xValues, yValues, 1);
  });

  osc.channels[channels[1]].stream.on('data', values => {
    let xValues = values.map(valObj => { return valObj.time });
    let yValues = values.map(valObj => { return valObj.val });
  //  console.log("val = " + yValues);
  
    drawValueRange(charts['chart3'], xValues, yValues, 2);
  });
}


async function startOsc2(deviceId) {
  let osc = model.device(deviceId).osc;
  let channels = [];
  Info.paramToCharts['chart3'].forEach(function(item, index, array) {
    channels.push(item.channel);
  })
  osc.openDataStream(channels);
  let charts = ChartControls();
  
  Info.paramToCharts['chart3'].forEach(function(item, index, array) {
      osc.channels[item.channel].stream.on('data', values => {
          let xValues = values.map(valObj => { return valObj.time });
          let yValues = values.map(valObj => { return valObj.val });
          drawValueRange(charts['chart3'], xValues, yValues, index + 1);
      })
  })
}

async function stopOsc(deviceId) {
  let osc = model.device(deviceId).osc;
  osc.closeDataStream();
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
          console.log("Start osc");
          // let deviceId = 1;
          // startOsc(deviceId);
          startOsc2(Info.states.indexDevice);
        }
      },
      {
        view: "button", value: "Stop osc", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Stop osc");
          let deviceId = Info.states.indexDevice;
          stopOsc(deviceId);
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


function mark_votes(value, config){
  if (value > 0 )
      return { "background":colorsArr[value-1], "color":colorsTitleArr[value-1] };
};

const paramToCharts = [];
// const paramToCharts = (paramArr) => {
//   return paramArr;
// }; 

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
          height:500, 
          columns:[
            { id:"channel", header:"Channel", width:120, css:"center"},
            { id:"name", header:"Name", fillspace:1 },
            { id:"status", header:"Show", width:80, css:"center", 
              template:"{common.checkbox()}"},
            { id:"line",	editor:"combo", 
             cssFormat:mark_votes,
              options:[
                {id:"a", value: ""},
                {id:1, value: "1"},
                {id:2, value: "2"},
                {id:3, value: "3"},
                {id:4, value: "4"},
                {id:5, value: "5"},
                {id:6, value: "6"} // {id:2, $css: "colorChart1", value: "2"}
              ], //collection:colorsArr,
              // css: "colorChart1",
              header:"Num Line", width:300},
          ],
          editable:true,
          // autoheight:true,
          data: [],
          on: {
            onAfterEditStop: function(state, editor, ignoreUpdate){
               console.log(editor.row);
               let table1 = this.getColumnConfig("line").collection;
               if ((editor.value) && (editor.value != "a")) table1.config.data[editor.value].disabled = false;
               if ((state.value) && (state.value != "a")) table1.config.data[state.value].disabled = true;
               let item = this.getItem(editor.row);
                console.log(item);
                let showParam = {name: item.name, channel: item.channel, idParam: item.idParam};
                paramToCharts.push(showParam);
            }
          }
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
                    let chartToVisible = chartsArrHidden.shift();
                    // let paramArr = ["Param 1","Param 2","Param 3","Param 4"];
                    let paramArr = [];
                    paramToCharts.forEach(function(item, index, array) {
                        paramArr.push(item.name);
                      })
                    Info.paramToCharts[chartToVisible] = paramToCharts;
                    Info.chartList[chartToVisible].setNamesArr(paramArr);
                    chartsArrVisible.push(chartToVisible);
                    document.documentElement.style.setProperty('--chartcount', chartsArrVisible.length);
                    showElementChart(chartToVisible);
                  $$("showSelectChartWindow").hide();
                }
            }
          ]
        }
      ]
    }
  }
  return v;
}

const addButtonClick = () => {
  // console.log("addButtonClick");
  if (chartsArrVisible.length < 3) {
      let comp = $$("showSelectChartWindowData");
      // console.log(comp);

      let osc = model.device(Info.states.indexDevice);
      let dataForChoose = [];
      if (osc != undefined) {
        // console.log(osc.osc);
        osc.osc.channels.forEach(function(item, index, array) {
          dataForChoose.push({ id:index, channel:item.num, name:item.name + " [" + item.param_id + "]",  
            status:0, idParam: item.param_id,
          });
          // console.log(dataForChoose);
        })
      }
      // console.log(dataForChoose);
      
      // if (chartsArrHidden.length > 0) {
        $$("showSelectChartWindow").show();
        comp.define({"data":dataForChoose});
        
        // console.log("indexDev " + Info.states.indexDevice);

        showSelectChartWindow();
      // }
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