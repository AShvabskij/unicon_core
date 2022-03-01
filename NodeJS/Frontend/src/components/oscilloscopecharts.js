import React, {useEffect} from "react";
import {Context, addCssClass} from '../Context';
import Chart, {SyncCharts} from './Chart';
import { observer } from "mobx-react";

async function loadOscData(chart) { // todo: вынести во вьюмодель
  let deviceId = Context.states.indexDevice;
  if (!chart || !deviceId) return;

  let chartParams = Context.chartParams(chart);
  let device = Context.model.device(deviceId);
  let osc = await device?.getOsc();

  if (!osc || !chartParams) return;

  let channelItems = [];
  chartParams.forEach(function(item) {
    let chItem = osc.channel(item.channel)
    channelItems.push(chItem);
  })

  setTimeout(async () => {
    await loadDrawData(chart, channelItems);
    SyncCharts();
  }, 300);

  return;
}

async function loadDrawData(chart, channelItems) {
  let actions = Context.chartControls[chart];
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

/*
function initOscParams(visibleCharts, chartControls, params) {
  if (!params) return;

  for (let chartId of visibleCharts) {
    let chControls = chartControls;
    if (!chControls.hasOwnProperty(chartId)) continue;

    let chartParams = params.get(chartId);
    chControls[chartId].setParams(chartParams);
  }
}
*/

const OscilloscopeChartsObserver = observer(({  }) => {
    useEffect(() => {
      console.log("Render oscilloscope for the device = " + Context.states.indexDevice);
    });
  
    return (
        <OscilloscopeCharts deviceId = {Context.states.indexDevice} charts = {Context.deviceCharts()} chartsLength = {Context.deviceCharts().length}
        chartControls = {Context.chartControls} chartPool = {Context.oscilloscopeChartList} params = {Context.paramToCharts}/>
    );
});
  
  function OscilloscopeCharts(props) {
    const [deviceId, setDeviceId] = React.useState([]);
    
    const _showCharts = () => {
      let chartControls = props.chartControls;
      let charts = props.charts
  
      let difference = props.chartPool.filter(x => !charts.includes(x));
  
      for (let chartId of difference) {
        if (!chartControls.hasOwnProperty(chartId)) continue;
  
        chartControls[chartId].setVisibility('hidden');
  //    setStyleByID(chartId, "visibility", 'hidden');
      };
  
      for (let chartId of charts) {
        chartControls[chartId].setVisibility('visible');
  //    setStyleByID(chartId, "visibility", 'visible');
        
        document.documentElement.style.setProperty('--chartcount', charts.length);
        addCssClass(chartId, "chartSizeControl")
      };
    };
        
    React.useEffect(() => {
      setDeviceId(props.deviceId);
    }, [props.deviceId]);
  
    React.useEffect(() => {
      _showCharts();    
    }, [props.charts, props.chartsLength]);

    let params = props.params?.get(props.deviceId);
    return (
      Context.oscilloscopeChartList.map((item) => (
        <Chart id = {item} title = "&nbsp;" loadData = {loadOscData} params = {params?.get(item)}/>
      ))
    );
  }
  
  export default OscilloscopeChartsObserver;