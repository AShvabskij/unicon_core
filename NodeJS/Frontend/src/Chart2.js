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
import { SciChartLegend } from "scichart/Charting/Visuals/Legend/SciChartLegend";
import { LegendModifier } from "scichart/Charting/ChartModifiers/LegendModifier";
import { ELegendOrientation, ELegendPlacement } from "scichart/Charting/Visuals/Legend/SciChartLegendBase";

import { NumberRange } from "scichart/Core/NumberRange";import { WaveAnimation } from "scichart/Charting/Visuals/RenderableSeries/Animations/WaveAnimation";

// import { EllipsePointMarker } from "scichart/Charting/Visuals/PointMarkers/EllipsePointMarker";
import { RolloverModifier } from "scichart/Charting/ChartModifiers/RolloverModifier";
import { TSciChart } from "scichart/types/TSciChart";
// import { IXyDataSeriesOptions} from "scichart/Charting/Model/XyDataSeries";

import Webix from './Webix';
import * as webix from 'webix/webix.js';

import { AddCounter, Info } from "./Context"

// import classes from "../../../../Examples/Examples.module.scss";
// import image from "./javascript-line-chart.jpg";

const LICENSE_KEY = "Ms4s7kPNHtGh/C9XitPAzhFJxW+ZFPkuVWnfhrARTR8oaE/D/8/yqLbAgcp2sQdVuva50wv5PkKiiJ0Asd7iYW3xGE9OvtnG6TZwbI4PPVq773RfvLjesvEkaB0u56BbKnAfsg/pCgTGbfRUgDCNqxs5DWUwXGXJPf3WIeTGHAU58XU4o1p6yQ3gN8/3kjE5wYIRJmebm1+Z+NCq67sol9pNpoOUfx13Mo4kURoFw24t+lwtTOmvpbLadWLn1v/wR7QlMf0kf9IrgnW8D5aoytdjoZs+XXVzRBF7EmjT5if72KszBF8EkX4aFYaEbsuhNqcVN+ovRhrYLFy8KfcGuGTZsiI0cv51IAKvHOkrLFeokv64vlKEBehLLkiWfcKkAuMUMMBrdars6BDo2nCHltAh2E01GNGG4Ah8OmFlQ7+x1rtol4LmxsIfB5CAhXHSGkt75XAlfiYv827ljmvNBGRNXu/JOHwBqlLdIesFeQtWz8PBAwowPQ/5c+mA+6oXt/heSmxUg0IgK6YdOHsj/7/SbW+OGqYLjhEn90aDA1YhPPqPaSbE3qhD";
// let scs: SciChartSurface;
// let timerId: NodeJS.Timeout;
const visiblePoints = 8000;
const intervalAddPoint = 40;
const suffixChartID = "scichart-root2";
let chartControls = [];

const colorsArr = ["#f6bf02","#0aa547","#eb4646", "blue", "#368BC1", "#eeeeee", "#ff6600", "#9b2dce", "#228B22", "#ff0000","orange","#be0000"];

const colorsTitleArr = ["black","white","white", "white", "white", "black", "white", "white", "white", "white","white","white"];

// const namesArr = ["Param 1","Param 2","Param 3","Param 4", "Param 5", "Param 6"];
// const namesArr = ["Param 1","Param 2","Param 3","Param 4"];


async function initSciChart(chartID , onAddFunction = (i) => {return 0}, namesArr = []) {
    const seriesArr = [];
    for (let k = 0; k < namesArr.length; k++) {
      seriesArr.push({color:colorsArr[k],name:namesArr[k],colorText:colorsTitleArr[k]});
      // seriesArr[0].color;
    }
  
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
    const yAxis = new NumericAxis(wasmContext,{ autoRange: EAutoRange.Always });
    sciChartSurface.xAxes.add(xAxis);
    sciChartSurface.yAxes.add(yAxis);

    sciChartSurface.chartModifiers.add(
      new RubberBandXyZoomModifier(),
      new MouseWheelZoomModifier(),
      new XAxisDragModifier({ dragMode: EDragMode.Panning }),
      new YAxisDragModifier({ dragMode: EDragMode.Panning }),
      new ZoomExtentsModifier(),
      new RolloverModifier()
  );

  const arrayLines = [];
  for (let m = 0; m < seriesArr.length; m++) {
    arrayLines.push(new FastLineRenderableSeries(wasmContext, {
      stroke: seriesArr[m].color,
      strokeThickness: 4,
      dataSeries: new XyDataSeries(wasmContext, { dataSeriesName: seriesArr[m].name }),
      animation: new WaveAnimation({ zeroLine: -1, pointDurationFraction: 0.5, duration: 100 })
    }))
    arrayLines[m].rolloverModifierProps.tooltipTitle = seriesArr[m].name;
    arrayLines[m].rolloverModifierProps.tooltipLabelX = "X";
    arrayLines[m].rolloverModifierProps.tooltipLabelY = "Y";
    arrayLines[m].rolloverModifierProps.markerColor = seriesArr[m].color;
    arrayLines[m].rolloverModifierProps.tooltipColor = seriesArr[m].color;
    arrayLines[m].rolloverModifierProps.tooltipTextColor = seriesArr[m].colorText;
    sciChartSurface.renderableSeries.add(arrayLines[m]);
  }
  
    let counter = 0;
    
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

  const defFunct = (j) => {
    return onAddFunction(j);
  }
  const defFunct1 = (j) => {
    return Math.cos(j* 0.01)*Math.sin((j+150)* 0.01)*(1+0.5*Math.random());
  }
  const defFunct2 = (j) => {
    return Math.cos(j* 0.01)*Math.sin((j+250)* 0.01)*(1+0.5*Math.random());
  }
  const defFunct3 = (j) => {
    return Math.cos(j* 0.01)*Math.sin((j+350)* 0.01)*(1+0.5*Math.random());
  }
  const defFunct4 = (j) => {
    return Math.cos(j* 0.01)*Math.sin((j+500)* 0.01)*(1+0.5*Math.random());
  }
  const defFunct5 = (j) => {
    return Math.cos(j* 0.01)*Math.sin((j+650)* 0.01)*(1+0.5*Math.random());
  }

  const defFuncts = [defFunct, defFunct1, defFunct2, defFunct3, defFunct4, defFunct5];

  const addPoint = () => {
    const step = 100;
    
    for(let i = counter; i < (counter+step); i++) {
      // console.log("i="+i);
      let v = onAddFunction(i);
      
      for (let m = 0; m < seriesArr.length; m++) {
        console.log("defFunct5" + defFuncts[m]);
        arrayLines[m].dataSeries.append(i, defFuncts[m](i));
      }
      
      if (i>1000000) {
        // xds.removeAt(0);
        arrayLines[0].dataSeries.removeAt(0);
      }
    }
    xAxis.visibleRange = new NumberRange(counter-visiblePoints, counter+step);
    counter = counter +step;
  }

  const addVarPoint = (x,y) => {
      arrayLines[0].dataSeries.append(x, y);
      xAxis.visibleRange = new NumberRange(x-visiblePoints,x);
  }

  const addVarPointRange = (xValues = [], yValues = []) => {
    arrayLines[0].dataSeries.appendRange(xValues, yValues);
    xAxis.visibleRange = new NumberRange(xValues[0]-visiblePoints, xValues[0]);
  }

  const addVarPoint2 = (x,y) => {
        arrayLines[1].dataSeries.append(x, y);
        xAxis.visibleRange = new NumberRange(x-visiblePoints,x);
  }

  const addVarPointRange2 = (xValues = [], yValues = []) => {
        arrayLines[1].dataSeries.appendRange(xValues, yValues);
        xAxis.visibleRange = new NumberRange(xValues[0]-visiblePoints, xValues[0]);
  }

  const startDemo = () => {
    console.log("startDemo");
    // xds.append(200, 0);
    // lineSeries.dataSeries = xds;
    // lineSeries1.dataSeries = xds1;
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


    return { wasmContext, sciChartSurface, controls: { startDemo, stopDemo, addVarPoint, addVarPoint2, addVarPointRange, addVarPointRange2 } };
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
  const [namesArr, setNamesArr] = React.useState([]);
  const [controls, setControls] = React.useState({ startDemo: () => {}, stopDemo: () => {}, addVarPoint: () =>{}, addVarPoint2: () =>{}, addVarPointRange: () =>{}, addVarPointRange2: () =>{} });
  
  React.useEffect(() => {
    (async () => {
        const res = await initSciChart(props.id,props.addFunction,namesArr);
        // scs = res.sciChartSurface;
        setControls(res.controls);
        chartControls[props.id] = res.controls;
        const lm = new LegendModifier({
          placement: ELegendPlacement.TopLeft,
          orientation: ELegendOrientation.Vertical,
          showLegend: true,
          showCheckboxes: true,
          showSeriesMarkers: true
        });

        res.sciChartSurface.chartModifiers.add(lm);
        
        // if (props.id) { 
        //   this.id = props.id
        chartControls[props.id] = res.controls;
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
}, [namesArr]);
    Info.chartList[props.id].setNamesArr = setNamesArr;
    let currentChartID = props.id+"_"+suffixChartID;
    return (
        <Row id={props.id}  style={{ visibility:"hidden" }} >
              <Col className = "chart1" xs={10}  > 
              {props.title}
              <div id={currentChartID} style={{ width:"auto", height: "calc(var(--chartheight))", margin: "auto"}} ></div>
              {/* <div id={currentChartID} style={{ width:"auto"}} ></div>  */}
              </Col>
              <Col className = "chart2" xs={2}   > 
              <div>&nbsp;</div>
                <Webix ui={webixButton()} data="Start" click={controls.startDemo} />
                <Webix ui={webixButton()} data="Stop" click={controls.stopDemo} />
              </Col>
              
        </Row>
    );
}

function ChartControls() {
  return chartControls;
}

export { ChartControls };
export { colorsArr, colorsTitleArr };
