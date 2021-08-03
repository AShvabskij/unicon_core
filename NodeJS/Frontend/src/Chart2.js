import * as React from "react";
import { Grid, Row, Col } from 'react-flexbox-grid';
import { SciChartSurface } from "scichart/Charting/Visuals/SciChartSurface";
import { NumericAxis } from "scichart/Charting/Visuals/Axis/NumericAxis";
import { FastLineRenderableSeries } from "scichart/Charting/Visuals/RenderableSeries/FastLineRenderableSeries";
import { XyDataSeries } from "scichart/Charting/Model/XyDataSeries";
import {ZoomPanModifier} from "scichart/Charting/ChartModifiers/ZoomPanModifier";
import { RubberBandXyZoomModifier } from "scichart/Charting/ChartModifiers/RubberBandXyZoomModifier";
import { MouseWheelZoomModifier } from "scichart/Charting/ChartModifiers/MouseWheelZoomModifier";
import { XAxisDragModifier } from "scichart/Charting/ChartModifiers/XAxisDragModifier";
import { EDragMode } from "scichart/types/DragMode";
import { YAxisDragModifier } from "scichart/Charting/ChartModifiers/YAxisDragModifier";
import { ZoomExtentsModifier } from "scichart/Charting/ChartModifiers/ZoomExtentsModifier";
import { ENumericFormat } from "scichart/types/NumericFormat";
import { EAutoRange } from "scichart/types/AutoRange";
import { NumberRange } from "scichart/Core/NumberRange";
import Webix from './Webix';
import * as webix from 'webix/webix.js';

// import classes from "../../../../Examples/Examples.module.scss";
// import image from "./javascript-line-chart.jpg";

const LICENSE_KEY = "JebThLVzGedXCDlzciqgNot7DqccqCsEy353wNG/duRRZbVHtULY5LkVxriRE9pvdR5Rb6d9n/cPeIlSl+oHHoiPvEe0Ne9GvOi2ca4jPZOcT7LBhfMm8jnbw6HlTQ6ULpqGVLnBgYReikDAPICyhlZjo/wgqLW/eQuzl5OUnRd7OJ3UfpBj8mvNQtx9OZttFrE8ARTLYgEDFcnzK49g8Z/86pgEb5/sm3w62bumsJM8h0aTg9S+UJTRqhYZC3lUBnfAMFFmcAXlTvs6c64Ln/KVM56TwrvUNhJ1pp6PZgqsOtRIyWDO/JkxoTwltPEFdcov3UcO2ElU2dHCIhTIhD6LB1qdciAZmBgTHoygmnaB/WmVZYcY5vPeI3u+/x6YtT+6cE5Nz7IOThNjTmzhcfC+JcVNwYzURbDssxYYayY5ct78AcnsN4bvDofvxrw9ASfBqRDIFSnguJzxEar9TUZwu7ZMOYSkuQFK0JIagW1E9xMAD4SvCd1K2naAlKkCoB05ovSPohiHqpVMn15t0UEpoJfjmrSOnkyj8Nbxe0P7F4zZ+Jdue1w96nec8g==";
let scs: SciChartSurface;
// let timerId: NodeJS.Timeout;
const visiblePoints = 1000;
const intervalAddPoint = 40;
const suffixChartID = "scichart-root2";

async function initSciChart(chartID , onAddFunction = () => {}) {
    // Below find a trial / BETA key for SciChart.js.
    // This Expires in 30 days - or 14th November 2020
    // Set this license key once in your app before calling SciChartSurface.create, e.g.
    const divElementId = chartID+"_"+suffixChartID;
    let timerLocalID 
    SciChartSurface.setRuntimeLicenseKey(
      LICENSE_KEY
    );
  
    // Create the SciChartSurface in the div 'scichart-root'
    // The SciChartSurface, and webassembly context 'wasmContext' are paired. This wasmContext
    // instance must be passed to other types that exist on the same surface.
    const { sciChartSurface, wasmContext } = await SciChartSurface.create(
      divElementId
    );
  
    // Create an X,Y Axis and add to the chart
    const xAxis = new NumericAxis(wasmContext,{ autoRange: EAutoRange.Once });
    //xAxis.autoRange = EAutoRange.Once;
    //xAxis.visibleRangeLimit = new NumberRange(1, 10000);

   
    const yAxis = new NumericAxis(wasmContext,{ autoRange: EAutoRange.Always });

    //const xAxis = new NumericAxis(wasmContext);
    //const yAxis = new NumericAxis(wasmContext);
    sciChartSurface.xAxes.add(xAxis);
    sciChartSurface.yAxes.add(yAxis);



    sciChartSurface.chartModifiers.add(
      new RubberBandXyZoomModifier(),
      new MouseWheelZoomModifier(),
      new XAxisDragModifier({ dragMode: EDragMode.Panning }),
      new YAxisDragModifier({ dragMode: EDragMode.Panning }),
      new ZoomExtentsModifier()
  );
  
    const lineSeries = new FastLineRenderableSeries(wasmContext, {
      stroke: "orange",
      strokeThickness: 4,
    });

    const lineSeries1 = new FastLineRenderableSeries(wasmContext, {
      stroke: "blue",
      strokeThickness: 2,
    });

    lineSeries.strokeThickness = 3;
    sciChartSurface.renderableSeries.add(lineSeries);
    sciChartSurface.renderableSeries.add(lineSeries1);
    //const xds = XyDataSeries;
    const xds = new XyDataSeries(wasmContext);
    const xds1 = new XyDataSeries(wasmContext);
    let counter = 0;
    //xds.appendRange([0, 50], [0, 150]);
    xds.append(0, 0);
    xds.append(1, 1);
    // xds.append(200, 0);
    lineSeries.dataSeries = xds;
    lineSeries1.dataSeries = xds1;
    // lineSeries.dataSeries = new XyDataSeries(wasmContext, {
    //   xValues: [150, 700, 9050],
    //   yValues: [150, 700, 1050],
    // });
    // console.log(lineSeries);
    console.log("lineSeries");
    console.log(lineSeries);
    //lineSeries.dataSeries[0].appendRange(1000,900)
  
    // Create 100 dataseries, each with 10k points
    for (let seriesCount = 0; seriesCount < 0; seriesCount++) {        
      const xyDataSeries = new XyDataSeries(wasmContext);

      const opacity = ((1 - ((seriesCount / 120)))*0.5).toFixed(2);

      // Populate with some data
/*      
      for(let i = 0; i < 10000; i++) {
          xyDataSeries.append(i, Math.sin(i* 0.01) * Math.exp(i*(0.00001*(seriesCount+1))));
      }
*/
      // Add and create a line series with this data to the chart
      // Create a line series        
      const lineSeries = new FastLineRenderableSeries(wasmContext, {
          dataSeries: xyDataSeries, 
          stroke: `rgba(006,096,222,${opacity})`,
          strokeThickness:1
      });
      // console.log("xyDataSeries-----------------------------------------");
      // console.log(xyDataSeries);
      sciChartSurface.renderableSeries.add(lineSeries);
  }

    //sciChartSurface.chartModifiers.add(new ZoomPanModifier());
  
    // That's it! You just created your first SciChartSurface!

    const stopDemo = () => {
      console.log("stopDemo");
      console.log("timerId = stopDemo");
      console.log(timerLocalID);
      clearInterval(timerLocalID);

      console.log(timerLocalID);
      xAxis.autoRange = EAutoRange.Once;
      // console.log("sciChartSurface.renderableSeries");
      // console.log(sciChartSurface.renderableSeries.items[0].isVisible);
      // sciChartSurface.renderableSeries.items[0].isVisible = ! sciChartSurface.renderableSeries.items[0].isVisible;
  };

  const addPoint = () => {
    const step = 100;
    
    for(let i = counter; i < (counter+step); i++) {
      // console.log("i="+i);
      let v= onAddFunction(i);
      xds.append(i, v);
      xds1.append(i, Math.cos(i* 0.01)*Math.sin((i+150)* 0.01)*(1+0.5*Math.random()));
      if (i>1000000) {
        xds.removeAt(0);
      }
    }
    xAxis.visibleRange = new NumberRange(counter-visiblePoints, counter+step);
    counter = counter +step;
  }

  const addVarPoint = (x,y) => {
    xds.append(x, y);
    xAxis.visibleRange = new NumberRange(x-visiblePoints,x);

  }

  const startDemo = () => {
    console.log("startDemo");
    // xds.append(200, 0);
    lineSeries.dataSeries = xds;
    lineSeries1.dataSeries = xds1;
    //xds.clear();
    //counter = 0;
    //xAxis.autoRange = EAutoRange.Always;
    console.log("timerId = setInterval");
    console.log(timerLocalID);
    clearInterval(timerLocalID);
    timerLocalID = setInterval(addPoint, intervalAddPoint);
    console.log(timerLocalID);
    
  //   for(let i = 0; i < 10000; i++) {
  //     xds.append(i, Math.sin(i* 0.01));
  // }
    // xds.append(0, 0);
    // xds.append(50, 50);
    // xds.append(150, 150);
    // xds.append(250, 0);
    // xAxis.autoRange = EAutoRange.Once;
    //sciChartSurface.renderableSeries.add(lineSeries);
  };


  const addLine = () => {
    sciChartSurface.renderableSeries.items[0].isVisible = true;
  }

  const removeLine = () => {
    console.log("removeLine");
    sciChartSurface.renderableSeries.items[0].isVisible = false
  }

  const removeLine1 = () => {
    console.log("removeLine1");
    // sciChartSurface.renderableSeries.items[0].isVisible = false
  }

    return { wasmContext, sciChartSurface, controls: { startDemo, stopDemo, addLine, addPoint, removeLine, addVarPoint } };
  }

  const webixButton = () => {
    
  return (
    {
      view:"button", 
      // id:"my_button", 
      value:"Button", 
      css:"webix_primary", 
      //inputWidth:100 ,
      click:function(id,event){
        console.log("webixButton");
        console.log(this.onclickMessage);
        this.onclickMessage();
      }
      // click: props.removeLine
      
  }
  )
}

const WebixButton12 = () => {
  // webix.ui({ view:"button", click:handler })
  return ( {
    view:"button" })
}



export default function Chart(props) {
  const [controls, setControls] = React.useState({ startDemo: () => {}, stopDemo: () => {}, addLine: () =>{}, removeLine: () =>{} , addVarPoint: () =>{} });

  React.useEffect(() => {
    (async () => {
        const res = await initSciChart(props.id,props.addFunction);
        scs = res.sciChartSurface;
        setControls(res.controls);
        // if (props.id) { 
        //   this.id = props.id
           window.chartEvents[props.id] = res.controls;
        // }
        //autoStartTimerId = setTimeout(res.controls.startDemo, 3000);
    })();
    // Delete sciChartSurface on unmount component to prevent memory leak
    return () => {
       // controls.stopDemo();
       // clearTimeout(timerId);
       // clearTimeout(autoStartTimerId);
        //scs?.delete();
    };
}, []);

    let currentChartID = props.id+"_"+suffixChartID;
    return (
        <Row id={props.id}  style={{ visibility:"hidden" }} >
              <Col className = "c1" xs={10}  > 
              {props.title}
              <div id={currentChartID} style={{ width:"auto", height: 300, margin: "auto"}} ></div>
              </Col>
              <Col className = "c2" xs={2}   > 
              <div>&nbsp;</div>
                <Webix ui={webixButton()} data="Start" click={controls.startDemo} />
                <Webix ui={webixButton()} data="Stop" click={controls.stopDemo} />
                <Webix ui={webixButton()} data="Add line" click={controls.addLine} />
                <Webix ui={webixButton()} data="Remove line" click={controls.removeLine} />
                
              </Col>
              
        </Row>
    );
}