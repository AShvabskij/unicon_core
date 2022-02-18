import 'webix/webix.css';
import { $$, template } from 'webix';
import * as webix from 'webix/webix.js';
import WebixComponent, {scroll} from './WebixComponent';
import React from "react";
import Chart, {colorsArrDefaults, colorsTitleArr } from './Chart';
import { Model } from "./data_model/fr_model.mjs";
import {Context} from './Context';

let colorsArrTab = [];
colorsArrTab = colorsArrDefaults;
let chartsArrVisible;


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
  let device = Context.model.device(indexDevice);
  let osc = await device.getOsc();

  let chart = chartsArrVisible.slice(-1);
  if (chart == undefined || Context.paramToCharts[chart] == undefined) {
    return;
  }

  let charts = Context.chartControls;

  let channels = [];
  Context.paramToCharts[chart].forEach(function(item, index, array) {
    channels.push(item.channel);
    charts[chart].clearChart(index + 1);
  })
  
  if (channels.length == 0) {
    console.log("There is no any selected channel!")
    return;
  }

  osc.openDataStream(channels);
  
  Context.paramToCharts[chart].forEach(function(item, index, array) {
      osc.channels[item.channel].stream.on('data', values => {
          let xValues = values.map(valObj => { return valObj.time });
          let yValues = values.map(valObj => { return valObj.val });
          drawValueRange(charts[chart], xValues, yValues, index + 1);
      })
  })
}

async function stopOsc(deviceId) {
  let device = Context.model.device(deviceId);
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

const toolBar = () => {
  return {
    view: "toolbar",
    // id: "myToolbar",
    cols: [
      {
        view: "button", value: "Add Chart", width: 100, align: "left",
        click: function (id, event) {
         // addButtonClick();
        }

      },
      {
        view: "button", value: "Remove Chart", autowidth: true, align: "center",
        click: function (id, event) {
         // removeButtonClick();
        }
      },
     
      {
        view: "button", value: "Start osc", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Start osc");
          // startOsc(Info.states.indexDevice);
        }
      },
      {
        view: "button", value: "Stop osc", autowidth: true, align: "center",
        click: async function (id, event) {
          console.log("Stop osc");
          // stopOsc(Info.states.indexDevice);
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
          let name1 = item.name + " [" + item.param_id + "]";
          if (item.name == "") name1 = "—";
          dataForChoose.push({ id:index, channel:item.num, name:name1,  
            color:item.color, status:0, idParam: item.param_id,
          });
          // console.log(dataForChoose);
        })
      }
      // console.log(dataForChoose);
      
      $$("showSelectChartWindow").show();
      comp.define({"data":dataForChoose});
        
  }
}

const removeButtonClick = () => {
  // console.log("removeButtonClick");
  if (chartsArrVisible.length > 0) {
    let chartToHidden = chartsArrVisible.pop();
    
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

export default class ViewCPlotWeb extends React.Component {
  constructor(props) {
    super(props);
  };


  render() {
  return (
      <div id="ViewCPlotWeb" className="pages">
        <WebixComponent ui={toolBar()} />
        <Chart id="chartCPlotWeb" title="&nbsp;" addFunction={addFunction} />
        {/* <Chart id="chartCPlotWeb1" title="&nbsp;" addFunction={addFunction} /> */}
      </div>    
    )
  }
};


// export default MenuCenter;