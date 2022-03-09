import React, {useEffect} from "react";
import {Context, setStyleByID, addCssClass} from '../Context';
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

  await loadDrawData(chart, channelItems);

  setTimeout(async () => {
    SyncCharts();
  }, 0);

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

  actions.zoomExtents();
}

//create your forceUpdate hook
function useForceUpdate(){
  const [value, setValue] = React.useState(0); // integer state
  console.log("forse update");
  return () => setValue(value => value + 1); // update the state to force render
}

const OscilloscopeChartsObserver = observer(({  }) => {

  const forceUpdate = useForceUpdate();

    useEffect(() => {
      console.log("Render oscilloscope for the device = " + Context.states.indexDevice);
    });
  
    return (
        <OscilloscopeCharts deviceId = {Context.states.indexDevice} visibleCharts = {Context.deviceCharts()} chartsLength = {Context.deviceCharts().length}
        chartControls = {Context.chartControls} chartPool = {Context.oscilloscopeChartList} params = {Context.paramToCharts}/>
    );
});
  
  function OscilloscopeCharts(props) {
    const _showCharts = () => {
      let chartControls = props.chartControls;
  
      let difference = props.chartPool.filter(x => !props.visibleCharts.includes(x));
/*
      for (let chartId of difference) {
        if (!chartControls.hasOwnProperty(chartId)) continue;
        chartControls[chartId].setVisibility('hidden');
      };
*/
      for (let chartId of props.visibleCharts) {
        let chartControls = props.chartControls;

        chartControls[chartId].setVisibility('visible');
        document.documentElement.style.setProperty('--chartcount', props.visibleCharts.length);
      };

    };
    
    React.useEffect(() => {
      _showCharts();    
    }, [props.charts, props.chartsLength]);

    let params = props.params?.get(props.deviceId);
    let oscilloscopes = props.visibleCharts.map((item) => (
      <Chart id = {item} title = "&nbsp;" loadData = {loadOscData} params = {params?.get(item)} chartCount = {props.visibleCharts.length} />
    ));

    return (
      <div id = "container" className = 'wrapper'>
        {oscilloscopes}
      </div>
    );
  }
  
  export default OscilloscopeChartsObserver;