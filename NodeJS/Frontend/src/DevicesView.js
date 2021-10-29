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
import ControlView from './ControlView';
import InfoView from './InfoView';
import Config from './.config.js';
import { Model } from "./data_model/fr_model.mjs";
import { AddCounter, Info } from "./Context"
import { colorsArrDefaults, colorsTitleArr } from './Chart2';

let colorsArrTab = [];
// colorsArrTab = colorsArrDefaults;

let model = new Model(Config.ip);
// document["model"] = model;
Info.model = model;
model.init();
console.log("model");
console.log(model.devices());

setTimeout(() => {
  model.load().then(result => {
    console.log("loadDataModel result:");
    console.log(model.devices());
    Info.actions.updateLeftMenu();
  }, error => {
    console.log("loadDataModel error");
    console.log(error);
  });
}, 1000);

model.on('error', (text) => { console.log(text);})

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

async function startOsc(indexDevice) {
  let device = Info.model.devices()[indexDevice];
  let osc = await device.getOsc();

  let chart = chartsArrVisible.slice(-1);
  if (chart == undefined || Info.paramToCharts[chart] == undefined) {
    return;
  }

  let charts = ChartControls();

  let channels = [];
  Info.paramToCharts[chart].forEach(function(item, index, array) {
    channels.push(item.channel);
    charts[chart].clearChart(index + 1);
  })
  
  if (channels.length == 0) {
    console.log("There is no any selected channel!")
    return;
  }

  osc.openDataStream(channels);
  
  Info.paramToCharts[chart].forEach(function(item, index, array) {
      osc.channels[item.channel].stream.on('data', values => {
          let xValues = values.map(valObj => { return valObj.time });
          let yValues = values.map(valObj => { return valObj.val });
          drawValueRange(charts[chart], xValues, yValues, index + 1);
      })
  })
}

async function stopOsc(deviceId) {
  let device = Info.model.devices()[deviceId];
  let osc = device.osc;

  if (osc != null) {
    osc.closeDataStream();
  }
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
  if (sc != undefined) {
    sc.style.setProperty("visibility", visibility);
  }
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
          content: "controlview"
        }
      },
      {
        id: "info",
        header: "Info",
        body: {
          id: "infoContent",
          view: "htmlform",
          content: "infoview"
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
            let cv = document.getElementById("controlview");
            cv.style.visibility = "visible";
          }

          // if (id == "controlContent") {
          //   showChart("chart1", "memo2")
          // }
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
      /* {
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
      }, */

      /* {
        view: "button", value: "Start values", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Start values");
          let deviceId = 1;
          let paramId = 65;
          startValues(deviceId, paramId, 1);
          startValues(23, paramId + 1, 2);
        }
      }, */
      {
        view: "button", value: "Start osc", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Start osc");
          startOsc(Info.states.indexDevice);
        }
      },
      {
        view: "button", value: "Stop osc", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Stop osc");
          stopOsc(Info.states.indexDevice);
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


// function mark_votes(value, config){
//   if (value > 0 )
//       return { "background":colorsArr[value-1], "color":colorsTitleArr[value-1] };
// };

function mark_votes(value, config){
  if (value > 0)
      // return { "background":colorsArrTab[value-1], "color":colorsTitleArr[value-1] };
      return { "background":colorsArrTab[value-1], "color":colorsArrTab[value-1] };
  else 
      return { "background":colorsArrTab[12], "color":"white" };
};

let paramToChart = [];
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
    head:"Select oscilloscope channels",
    body:{
      rows: [
        {
          id: "showSelectChartWindowData",   
          view:"datatable",
          height:500, 
          // scheme:{
          //   $change:function(item){
          //     if (item.status.state == 1)
          //       item.$cellCss = {line:"green"};
          //   }
          // },
          columns:[
            { id:"channel", header:"Channel", width:120, css:"center"},
            { id:"name", header:["Name", {content:"textFilter" }], fillspace:1,  },
            { id:"status", header:["Show", {content:"masterCheckbox"}], width:80, css:"center", 
                  template:"{common.checkbox()}"},
            { id:"line",	//editor:"combo", 
             cssFormat:mark_votes,
              // options:[
              //   {id:"a", value: ""},
              //   {id:1, value: "1"},
              //   {id:2, value: "2"},
              //   {id:3, value: "3"},
              //   {id:4, value: "4"},
              //   {id:5, value: "5"},
              //   {id:6, value: "6"} // {id:2, $css: "colorChart1", value: "2"}
              // ], //collection:colorsArr,
              // css: "colorChart1",
              header:"Num Line", width:300},
          ],
          //editable:true,
          // checkboxRefresh:true,
          // autoheight:true,
          data: [],
          on: {
            onCheck: function(row, column, state){
              console.log("onCheck");
              let table1 = $$("showSelectChartWindowData");
              let item = table1.getItem(row);
                  
              if (item.name != "—") {
                  let showParam = {name: item.name, channel: item.channel, idParam: item.idParam, row: row};
                  
                  if (state == 1) {
                      Info.paramToChart.push(showParam);
                      //console.log(colorsArrTab);
                      colorsArrTab.push(item.color);
                      item.line = Info.paramToChart.length;
                  }
                  if (state == 0) {
                      let newParamArr = [];
                      let newColorsArrTab = [];
                      Info.paramToChart.forEach(function(item3, index, array) {
                          console.log(item3);  
                          if(item3.channel == item.channel) {
                            console.log("continue");
                          }
                          else {
                            newParamArr.push(item3);
                            newColorsArrTab.push(colorsArrTab[index]);
                          }
                      });
                      Info.paramToChart = newParamArr;
                      colorsArrTab = newColorsArrTab;
                      
                      item.line = 0;
                      Info.paramToChart.forEach(function(item2, index, array) {
                        let item4 = table1.getItem(item2.row);
                        item4.line = index + 1;
                        table1.updateItem(item2.row, item4);
                      })
                    }
                  }
              else {
                      item.status = 0;
                  }
              table1.updateItem(row, item);
           },
            /* onAfterEditStop: function(state, editor, ignoreUpdate){
              //  console.log(editor.row);
              //  console.log("paramToCharts = " + paramToCharts);
               let table1 = this.getColumnConfig("line").collection;
               if ((editor.value) && (editor.value != "a")) table1.config.data[editor.value].disabled = false;
               if ((state.value) && (state.value != "a")) table1.config.data[state.value].disabled = true;
               let item = this.getItem(editor.row);
                // console.log(item);
                let showParam = {name: item.name, channel: item.channel, idParam: item.idParam};
                paramToCharts.push(showParam);
            }, */
            /* onCheck: function(row, column, state){
              console.log("onCheck");
              console.log("paramToCharts init");
              console.log(paramToCharts);
              console.log(row);
              console.log(column);
              console.log(state);
              let item = this.getItem(row);
              // console.log(item);
              let showParam = {name: item.name, channel: item.channel, idParam: item.idParam, row: row};
              let table1 = $$("showSelectChartWindowData");
              let item1 = table1.getItem(row);
              if (state == 1) {
                  paramToCharts.push(showParam);
                  console.log("paramToCharts Add");
                  console.log(paramToCharts);
                  item1.line = paramToCharts.length;
                  table1.updateItem(row, item1);
                  console.log("paramToCharts end add");
                  console.log(paramToCharts);
                  
              }
              if (state == 0) {
                  console.log("paramToCharts init remove");
                  console.log(paramToCharts);
                  item1.line = "a";
                  table1.updateItem(row, item1);
                  console.log(showParam);
                  let posId = paramToCharts.indexOf(showParam);
                  console.log("posId = " + posId);
                  paramToCharts.splice(posId, 1);
                  console.log(paramToCharts.length);
                  paramToCharts.forEach(function(item2, index, array) {
                    // console.log(item2.row);
                    let item4 = table1.getItem(item2.row);
                    // console.log(item4);
                    item4.line = index + 1;
                    table1.updateItem(item2.row, item4);
                  })
              }
              console.log("item1.line="+item1.line);
              // table1.updateItem(row, item1);
           }, */
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
                    let paramArr = [];
                    Info.paramToChart.forEach(function(item, index, array) {
                        paramArr.push(item.name);
                      });
                    Info.paramToCharts[chartToVisible] = Info.paramToChart;
                    Info.chartList[chartToVisible].setColorsArr(colorsArrTab);
                    Info.chartList[chartToVisible].setNamesArr(paramArr);
                    chartsArrVisible.push(chartToVisible);
                    document.documentElement.style.setProperty('--chartcount', chartsArrVisible.length);
                    showElementChart(chartToVisible);
                  $$("showSelectChartWindow").hide();
                  let table1 = $$("showSelectChartWindowData");
                  Info.paramToChart.forEach(function(item, index, array) {
                      let item2 = table1.getItem(item.row);
                      item2.line = 0;
                  });
//                table1.refresh;
                  Info.paramToChart = [];
                }
            }
          ]
        }
      ]
    }
  }
  return v;
}

function setStatus(){
  console.log("!!!");
  /*
  const table = $$("car_rental_table");
  table.filter(); 
  table.showItem(table.getFirstId()); 
  table.setState({filter:{}}); 
*/
}

const addButtonClick = async () => {
  // console.log("addButtonClick");
  if (chartsArrVisible.length < 3) {
      let comp = $$("showSelectChartWindowData");
      // console.log(comp);

      let indexDevice = Info.states.indexDevice;
      let device = Info.model.devices()[indexDevice];
      let dataForChoose = [];
      if (device != undefined) {
        let osc = await device.getOsc();
        osc.channels.forEach(function(item, index, array) {
          let name1 = item.name + " [" + item.module_id + "."+ item.param_id + "]";
          if (item.name == "") name1 = "—";
          dataForChoose.push({ id:index, channel:item.num, name:name1,  
            color:item.color, status:0, idParam: item.param_id,
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
    if (document.getElementById(chartsArrVisible.at(-1)) == undefined) return;
  
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
      <div id="deviceView">
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

        {/* <Chart /> */}
        {/* <ChartList /> */}
        <div id="memo1">{/* Memo 1 */}
          <Webix ui={toolBar()} />
        </div>
        {/* <div id="memo2">Memo 2</div>
        <div id="memo3">Memo 3</div>
        <div id="memo4">Memo 4</div>
         */}
         <div id="controlview" className="w100proc" style={{ visibility:"hidden" }} >
            <ControlView />
         {/* <Webix id="resize1" ui={webixButton()} /> */}
        </div>
        
         <ParametersView data={this.state.dt} />
        
         <InfoView data={Info.states.indexDevice} />
      </div>
    )
  }
};


// export default MenuCenter;
export { removeButtonClick };