import React, {useEffect} from "react";
import {Context, addCssClass} from '../Context';
import Chart, {SyncCharts} from './Chart';
import { observer } from "mobx-react";

function initOscParams(indexDevice) {
    let visibleCharts = Context.deviceCharts(indexDevice);
    let params = Context.paramToCharts.get(indexDevice);
    if (!params) return;
  
    for (let chartId of visibleCharts) {
      let paramArr = [];
      params.get(chartId).forEach(function(item, index, array) {
          paramArr.push(item.name);
      });
    
      Context.chartList[chartId].setNamesArr(paramArr);
    }
  }
  
async function loadOscData() {
let indexDevice = Context.states.indexDevice;
let visibleCharts = Context.deviceCharts(indexDevice);
let params = Context.paramToCharts.get(indexDevice);
if (!params || !visibleCharts || visibleCharts.length == 0) return;

let device = Context.model.device(indexDevice);
let osc = await device.getOsc();

    let chart = visibleCharts.slice(-1).toString();
    let chart2 = visibleCharts.slice(-2, -1).toString();
    let chart3 = visibleCharts.slice(-3, -2).toString();

    let isChart2 = chart2 !== '';
    let isChart3 = chart3 !== '';

    let paramsChart = params.get(chart);
    if (chart == undefined || paramsChart == undefined) {
    return;
    }

    let channelItems1 = [];
    params.get(chart).forEach(function(item, index, array) {
    let chItem = osc.channel(item.channel)
    channelItems1.push(chItem);
    })

    let channelItems2 = [];
    if (isChart2) {
    params.get(chart2).forEach(function(item, index, array) {
        let chItem = osc.channel(item.channel)
        channelItems2.push(chItem);
    })
    }

    let channelItems3 = [];
    if (isChart3) {
    params.get(chart3).forEach(function(item, index, array) {
        let chItem = osc.channel(item.channel)
        channelItems3.push(chItem);
    })
    }

    setTimeout(() => {
    loadDrawData(chart, channelItems1);
    
    if (isChart2) loadDrawData(chart2, channelItems2);
    if (isChart3) loadDrawData(chart3, channelItems3);

    SyncCharts();
    }, 300);

    return;
}

async function loadDrawData(chartId, channelItems) {
    let actions = Context.chartControls[chartId];
    if (!actions) return;

    channelItems.forEach(function(ch, index, array) {
        if (ch.buffer) {
        for (let values of ch.buffer) {
            let xValues = values.map(valObj => { return valObj.time });
            let yValues = values.map(valObj => { return valObj.val });
            actions.addVarPointMegaRange(xValues, yValues, index + 1);
        }
        }
    });
}

const deviceCharts = (indexDevice) => {
    let res = [];
    let ind = indexDevice !== undefined ? indexDevice : Context.states.indexDevice;
    
    if (Context.deviceChartList.has(ind)) {
      res = Context.deviceChartList.get(ind);
    }
  
    return res;
}


const OscilloscopeChartsObserver = observer(({  }) => {
    useEffect(() => {
      console.log("Render oscilloscope for the device = " + Context.states.indexDevice);
  
    });
  
    return (
        <OscilloscopeCharts deviceId = {Context.states.indexDevice} charts = {Context.deviceCharts()} chartsLength = {Context.chartsLength}
        chartControls = {Context.chartControls} chartPool = {Context.oscilloscopeChartList}/>
    );
});
  
  function OscilloscopeCharts(props) {
    const [deviceId, setDeviceId] = React.useState([]);
  // const [charts, setCharts] = React.useState([]);
    
    const _showCharts = () => {
      let chControls = props.chartControls;
      let charts = props.charts
  
      let difference = props.chartPool.filter(x => !charts.includes(x));
  
      for (let chartId of difference) {
        if (!chControls.hasOwnProperty(chartId)) continue;
  
        chControls[chartId].setVisibility('hidden');
  //    setStyleByID(chartId, "visibility", 'hidden');
      };
  
      for (let chartId of charts) {
        chControls[chartId].setVisibility('visible');
  //    setStyleByID(chartId, "visibility", 'visible');
        
        document.documentElement.style.setProperty('--chartcount', charts.length);
        addCssClass(chartId, "chartSizeControl")
      };
  
      initOscParams(props.deviceId);
    };
  
    React.useEffect(() => {
      setDeviceId(props.deviceId);
    }, [props.deviceId]);
  
    React.useEffect(() => {
  /*    
      let charts = [];
      for(let chartId of props.charts) {
        charts.push(chartId);
      }
      setCharts(charts);
  */
      console.log("chart changed = " + props.charts.length)
      _showCharts()    
    }, [props.charts, props.chartsLength]);
  
  /*  
    React.useEffect(() => {
      console.log("showcharts_ when charts changed = " + charts.length)
      _showCharts()
    }, [charts]);
  */
  
    return (
      Context.oscilloscopeChartList.map((item) => (
        <Chart id={item} title="&nbsp;" loadData={loadOscData} />
      ))
    );
  }
  
  export default OscilloscopeChartsObserver;