import React, {useEffect} from "react";
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import {$$, template} from 'webix';
import {Context} from './Context';
import {SyncCharts} from './components/Chart';
import ViewDevicesSelectChart  from "./ViewDevicesSelectChart";
import OscilloscopeChartsObserver  from "./components/oscilloscopecharts"

const toolBar = () => {
  return {
    cols: [
      {
        view: "toolbar",
        // id: "myToolbar",
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
            view: "button", value: "Start osc", autowidth: true, align: "center",
            click: async function (id, event) {
              console.log("Start osc");
              startOsc(Context.states.indexDevice);
            }
          },
          {
            view: "button", value: "Stop osc", autowidth: true, align: "center",
            click: async function (id, event) {
              console.log("Stop osc");
              stopOsc(Context.states.indexDevice);
            }
          },
        ]        
      },
      {
        view: "toolbar",
        // id: "myToolbar2",
        margin:10, paddingX:0,
        cols: [
          {},
          {
            view: "button", value: "+", width: 40, align: "left",
            click: async function (id, event) {
              await ScalePlus();
            }
    
          },
          {
            view: "button", value: "-", width: 40, align: "left",
            click: async function (id, event) {
              ScaleMinus();
            }
    
          },
          {
            view: "toggle", id: "cursor", onLabel: "Cursor on", offLabel: "Cursor off", width: 100, align: "left",
            click: async function (id, event) {
              await SwitchCursor( $$(id).getValue());
              $$(id).blur()
            }
          },
          {
            view: "toggle", id: "preview", label: "Preview", width: 100, align: "left",
            click: async function (id, event) {
              await SwitchPreview( $$(id).getValue());
              $$(id).blur()
            }
          },          
          {}
        ]
      }
    ]

  }
}

const addButtonClick = async () => {
  let indexDevice = Context.states.indexDevice;

  if (Context.deviceCharts(indexDevice).length < 3) {
      let comp = $$("showSelectChartWindowData");
      let device = Context.model.device(indexDevice);
      let dataForChoose = [];

      if (device != undefined) {
        let osc = await device.getOsc();
        osc.channels.forEach(function(item, index, array) {
          let name1 = item.name + " [" + device.id + "."+ item.var_id + "]";
          if (item.name == "") name1 = "—";
          dataForChoose.push({ id:index, channel:item.num, name:name1,  
            color:item.color, status:0, idParam: item.var_id,
          });
        })
      }

      $$("showSelectChartWindow").show();
      comp.define({"data":dataForChoose});
  }
}

const removeButtonClick = () => {
  let indexDevice = Context.states.indexDevice;
  Context.popChart(indexDevice);
}

async function startOsc(indexDevice) {
  let visibleCharts = Context.deviceCharts(indexDevice);
  let params = Context.paramToCharts.get(indexDevice);
  if (!params) return;

  let chart = visibleCharts.slice(-1).toString();
  let chart2 = visibleCharts.slice(-2, -1).toString();
  let chart3 = visibleCharts.slice(-3, -2).toString();

  let isChart2 = chart2 !== '';
  let isChart3 = chart3 !== '';

  let paramsChart = params.get(chart);
   if (chart == undefined || paramsChart == undefined) {
    return;
  }

  let device = Context.model.device(indexDevice);
  let osc = await device.getOsc();
  let actions = Context.chartControls;

  let allChannels = [];
  let channelItems1 = [];
  params.get(chart).forEach(function(item, index, array) {
    actions[chart].clearChart(index + 1);
    let chItem = osc.channel(item.channel)
    channelItems1.push(chItem);
    allChannels.push(item.channel)
  })

  let channelItems2 = [];
  if (isChart2) {
    params.get(chart2).forEach(function(item, index, array) {
      actions[chart2].clearChart(index + 1);
      let chItem = osc.channel(item.channel)
      channelItems2.push(chItem);
      allChannels.push(item.channel)
    })
  }

  let channelItems3 = [];
  if (isChart3) {
    params.get(chart3).forEach(function(item, index, array) {
      actions[chart3].clearChart(index + 1);
      let chItem = osc.channel(item.channel)
      channelItems3.push(chItem);
      allChannels.push(item.channel)
    })
  }

  if (allChannels.length == 0) {
    console.log("There is no any selected channel!")
    return;
  }

  osc.openDataStream(allChannels);

  startDrawData(chart, channelItems1);
  startDrawData(chart2, channelItems2);
  startDrawData(chart3, channelItems3);
}

async function ScalePlus() {
  let charts = Context.deviceCharts();

  let actions = Context.chartControls;
  charts.forEach(function(chart, index, array) {
    actions[chart].Scale(-0.25, -0.25);
  });
}

async function ScaleMinus() {
  let charts = Context.deviceCharts();
  let actions = Context.chartControls;

  charts.forEach(function(chart, index, array) {
    actions[chart].Scale(0.25, 0.25);
  });
}

async function SwitchCursor(value) {
  let charts = Context.deviceCharts();
  let actions = Context.chartControls;

  charts.forEach(function(chart, index, array) {
    actions[chart].SwitchCursor(!value);
  });
}

async function SwitchPreview(value) {
  let charts = Context.deviceCharts();
  let actions = Context.chartControls;

  charts.forEach(function(chart, index, array) {
    actions[chart].SwitchPreview(!value);
  });
}

async function startDrawData(chart, channelItems) {
  let actions = Context.chartControls;
  let freqHz = 50;

  let timerId = setInterval(() => {
    channelItems.forEach(function(ch, index, array) {
      let values = ch.stream.read();
      if (values) {
        let xValues = values.map(valObj => { return valObj.time });
        let yValues = values.map(valObj => { return valObj.val });
        actions[chart].addVarPointRange(xValues, yValues, index + 1);
      }
    })
  }, 1000/freqHz);

  channelItems.forEach(function(item, index, array) {
    item.stream.on('end', values => {
      if (timerId != 0) {
        clearInterval(timerId);
        timerId = 0;
        actions[chart].zoomExtents();
        setTimeout(async () => {
          SyncCharts();
        }, 0);
      }
    })
  });
    
/*  
  Info.paramToCharts[chart].forEach(function(item, index, array) {
    let ch = osc.channel(item.channel);
      ch.stream.on('data', values => {
          let xValues = values.map(valObj => { return valObj.time });
          let yValues = values.map(valObj => { return valObj.val });
          drawValueRange(charts[chart], xValues, yValues, index + 1);

      })
  })
*/

}

async function stopOsc(deviceId) {
  let device = Context.model.device(deviceId);
  let osc = device.osc;

  if (osc != null) {
    osc.closeDataStream();
  }
}

//Временная генерация данных
function addFunction(x) {
    // console.log("addFunction");
    return Math.sin(x * 0.01) * (1 + 0.5 * Math.random());
}

function ViewDevicesOscilloscope(props) {

  return (
    <div id={props.id} className="pages">
      <WebixComponent ui={toolBar()} />
      <ViewDevicesSelectChart />
      <OscilloscopeChartsObserver />
    </div>
  );
}

export default ViewDevicesOscilloscope;