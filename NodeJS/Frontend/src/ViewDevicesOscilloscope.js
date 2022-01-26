import React from "react";
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import { $$, template } from 'webix';
import { Context } from './Context';
import Chart, { ChartControls, SyncCharts } from './Chart';
import ViewDevicesSelectChart  from "./ViewDevicesSelectChart";

export const chartsArrVisible = [];
export const chartsArrHidden = ["chart3","chart4","chart5"];

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
  // console.log("addButtonClick");
  if (chartsArrVisible.length < 3) {
      let comp = $$("showSelectChartWindowData");
      // console.log(comp);

      let indexDevice = Context.states.indexDevice;
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
          // console.log(dataForChoose);
        })
      }
      // if (chartsArrHidden.length > 0) {
        $$("showSelectChartWindow").show();
        comp.define({"data":dataForChoose});
    // }
       
  }
}

const removeButtonClick = () => {
  console.log("removeButtonClick");
  console.log(chartsArrVisible);
  if (chartsArrVisible.length > 0) {
    if (document.getElementById(chartsArrVisible.at(-1)) == undefined) return;
  
    let chartToHidden = chartsArrVisible.pop();
    chartsArrHidden.unshift(chartToHidden);

    let charts = ChartControls();    
    charts[chartToHidden].SwitchPreview(0);

    charts[chartToHidden].setVisibility('hidden', chartsArrVisible.length);
    document.documentElement.style.setProperty('--chartcount', chartsArrVisible.length);
  }
  // then stop process — add
}

async function startOsc(indexDevice) {
  let device = Context.model.device(indexDevice);
  let osc = await device.getOsc();

  let chart = chartsArrVisible.slice(-1);
  if (chart == undefined || Context.paramToCharts[chart] == undefined) {
    return;
  }

  let chart2 = chartsArrVisible.slice(-2, -1);
  let chart3 = chartsArrVisible.slice(-3, -2);

  let isChart2 = Context.paramToCharts[chart2] !== undefined;
  let isChart3 = Context.paramToCharts[chart3] !== undefined;

  let charts = ChartControls();

  let allChannels = [];
  let channelItems1 = [];
  Context.paramToCharts[chart].forEach(function(item, index, array) {
    charts[chart].clearChart(index + 1);
    let chItem = osc.channel(item.channel)
    channelItems1.push(chItem);
    allChannels.push(item.channel)
  })

  let channelItems2 = [];
  if (isChart2) {
    Context.paramToCharts[chart2].forEach(function(item, index, array) {
      charts[chart2].clearChart(index + 1);
      let chItem = osc.channel(item.channel)
      channelItems2.push(chItem);
      allChannels.push(item.channel)
    })
  }

  let channelItems3 = [];
  if (isChart3) {
    Context.paramToCharts[chart3].forEach(function(item, index, array) {
      charts[chart3].clearChart(index + 1);
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
  let charts = ChartControls();
  chartsArrVisible.forEach(function(chart, index, array) {
    charts[chart].Scale(-0.25, -0.25);
  });
}

async function ScaleMinus() {
  let charts = ChartControls();
  chartsArrVisible.forEach(function(chart, index, array) {
    charts[chart].Scale(0.25, 0.25);
  });
}

async function SwitchCursor(value) {
  let charts = ChartControls();
  chartsArrVisible.forEach(function(chart, index, array) {
    charts[chart].SwitchCursor(!value);
  });
}

async function SwitchPreview(value) {
  let charts = ChartControls();
  chartsArrVisible.forEach(function(chart, index, array) {
    charts[chart].SwitchPreview(!value);
  });
}

async function startDrawData(chart, channelItems) {
  let charts = ChartControls();

  let freqHz = 50;

  let timerId = setInterval(() => {
    channelItems.forEach(function(ch, index, array) {
      let values = ch.stream.read();
      if (values) {
        let xValues = values.map(valObj => { return valObj.time });
        let yValues = values.map(valObj => { return valObj.val });
        charts[chart].addVarPointRange(xValues, yValues, index + 1);
      }
    })
  }, 1000/freqHz);

  channelItems.forEach(function(item, index, array) {
    item.stream.on('end', values => {
      if (timerId != 0) {
        clearInterval(timerId);
        timerId = 0;
        SyncCharts();
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
  console.log('View devices props = ' + props.data);

  return (
    
    <div id={props.id} className="pages">
      <WebixComponent ui={toolBar()} />
      <ViewDevicesSelectChart />
      {
      Context.oscilloscopeChartList.map((item) => (
                        <Chart id={item} title="&nbsp;" addFunction={addFunction} />
                ))
      }
     </div>
  );
}

export default ViewDevicesOscilloscope;