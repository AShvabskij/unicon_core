import * as React from "react";
import { Grid, Row, Col } from 'react-flexbox-grid';
import { SciChartSurface } from "scichart/Charting/Visuals/SciChartSurface";
import { NumericAxis } from "scichart/Charting/Visuals/Axis/NumericAxis";
import { FastLineRenderableSeries } from "scichart/Charting/Visuals/RenderableSeries/FastLineRenderableSeries";
import { XyDataSeries } from "scichart/Charting/Model/XyDataSeries";
import { ZoomPanModifier } from "scichart/Charting/ChartModifiers/ZoomPanModifier";
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

import { NumberRange } from "scichart/Core/NumberRange"; import { WaveAnimation } from "scichart/Charting/Visuals/RenderableSeries/Animations/WaveAnimation";

// import { EllipsePointMarker } from "scichart/Charting/Visuals/PointMarkers/EllipsePointMarker";
import { RolloverModifier } from "scichart/Charting/ChartModifiers/RolloverModifier";
import { TSciChart } from "scichart/types/TSciChart";
// import { IXyDataSeriesOptions} from "scichart/Charting/Model/XyDataSeries";

import Webix from './Webix';
import * as webix from 'webix/webix.js';

import { AddCounter, Info } from "./Context"

// import classes from "../../../../Examples/Examples.module.scss";
// import image from "./javascript-line-chart.jpg";

const LICENSE_KEY = "5obTEufopzpmZoVnuxGH/pLJzUjiLmHopfyX3DyUklHey7Y5M0glbfHMtl45fsdkg9/j35pS2jwXCF1Uuq/kx/xCNz7ykB/9/0eEpQxWt03XI0oPFHplh9sYLfpEfJUqnTjPUFlki4aKYBDe/sj3HDxIl4kmHx/P2jsp02CkImftfuleC3bzDL4joIKl4IOKsTpgi8DL/TDC21w/z0jG5GuT4x6Ts9wB2sBH8J+a07r31wXwDGucUqAtAMJvYcYCLdvKn9qiWgp4fQ9Wqh9KUhK6h83AkQ+5g/gjwxmem5VD7hSgHnloqDTqmeirQnf9UFwmYRyKmDSLifPA1J7ZFwTeNGa3cB7aFg/qlPNIJISmyRt9PBIRkCyeyCjhJhPKx7T1U/G0vUDSASQRzI3TgX+Mwor3DN12cXasdxHIkqaYMfT2QcbpoTY3L3tFAHkWEWKDpo/aYyfgWz80LLBGxfIKy+f8GHfx4gOGZ/hM7EFkjFY3AzgrY8HFepNUw1JdnUXPkEGtY31dq5jwJlWFxv6HG05dTUH1Kpixulp1O/UvZm2mTLvJ7EGdaA==";
// let scs: SciChartSurface;
// let timerId: NodeJS.Timeout;
const visiblePoints = 10000;
const intervalAddPoint = 40;
const suffixChartID = "scichart-root2";
let chartControls = [];

const colorsArrDefaults = ["#f6bf02","#0aa547","#eb4646", "blue", "#368BC1", "#eeeeee", "#ff6600", "#9b2dce", "#228B22", "#ff0000","orange","#be0000", "white"];
// let colorsArr = colorsArrDefaults;

const colorsTitleArr = ["black","black","black", "black", "black", "black", "white", "white", "white", "white","white","white"];

// const namesArr = ["Param 1","Param 2","Param 3","Param 4", "Param 5", "Param 6"];
// const namesArr = ["Param 1","Param 2","Param 3","Param 4"];


async function initSciChart(chartID , onAddFunction = (i) => {return 0}, namesArr = [], colorsArr = []) {
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
  const xAxis = new NumericAxis(wasmContext, { autoRange: EAutoRange.Once });
  xAxis.drawMajorGridLines = true;
  xAxis.drawMinorGridLines = true;

  let yMin = -4000;
  let yMax = 4000;
  const yAxis = new NumericAxis(wasmContext, { autoRange: EAutoRange.Always });
  yAxis.drawMajorGridLines = true;
  yAxis.drawMinorGridLines = true;
  yAxis.labelProvider.formatLabel = (dataValue: number) => dataValue.toFixed(3);
  yAxis.autoRange = EAutoRange.Once;
  yAxis.visibleRange = new NumberRange(yMin, yMax);

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
      strokeThickness: 2,
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
    console.log("m="+m);
    console.log(seriesArr[m]);
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

  const clearChart = (line) => {
    arrayLines[line-1].dataSeries.clear();
  }

  const addVarPoint = (x,y,line) => {
      arrayLines[line-1].dataSeries.append(x, y);
      xAxis.visibleRange = new NumberRange(x-visiblePoints,x);
  }

  const addVarPointRange = (xValues = [], yValues = [], line) => {
    arrayLines[line-1].dataSeries.appendRange(xValues, yValues);
    xAxis.visibleRange = new NumberRange(xValues[0]-visiblePoints, xValues[0]);

    var curMin = Math.min.apply(null, yValues),
    curMax = Math.max.apply(null, yValues);

    var g = curMax < 0 ? -0.1 : 0.1
    if (yMax < (curMax + curMax*g) ) {
      g = curMax < 0 ? -0.5 : 0.5
      yMax = curMax + g*curMax;
    }

    g = curMin < 0 ? -0.1 : 0.12
    if (yMin > (curMin-g*curMin) && curMin / yMin < 2 ) {
      g = curMin < 0 ? -0.5 : 0.5
      yMin = curMin - g*curMin;
    } 

    yAxis.visibleRange = new NumberRange(yMin, yMax);    
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


  return { wasmContext, sciChartSurface, controls: { startDemo, stopDemo, addVarPoint, addVarPoint2, addVarPointRange, addVarPointRange2, clearChart } };
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
  // const [colorsArr, setColorsArr] = React.useState(colorsArrDefaults);
  const [colorsArr, setColorsArr] = React.useState([]);
  const [controls, setControls] = React.useState({ startDemo: () => {}, stopDemo: () => {}, addVarPoint: () =>{}, addVarPoint2: () =>{}, addVarPointRange: () =>{}, addVarPointRange2: () =>{}, clearChart: () =>{}, });
  console.log(colorsArr);
  React.useEffect(() => {
    (async () => {
        const res = await initSciChart(props.id,props.addFunction,namesArr,colorsArr);
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
  }, [namesArr], [colorsArr]);
  Info.chartList[props.id].setNamesArr = setNamesArr;
  Info.chartList[props.id].setColorsArr = setColorsArr;
    let currentChartID = props.id+"_"+suffixChartID;
  return (
        <Row id={props.id}  style={{ visibility:"hidden" }} >
              <Col className = "chart1" xs={12}  > 
        {props.title}
              <div id={currentChartID} style={{ width:"auto", height: "calc(var(--chartheight))", margin: "auto"}} ></div>
        {/* <div id={currentChartID} style={{ width:"auto"}} ></div>  */}
      </Col>
             {/*  <Col className = "chart2" xs={2}   > 
        <div>&nbsp;</div>
        <Webix ui={webixButton()} data="Start" click={controls.startDemo} />
        <Webix ui={webixButton()} data="Stop" click={controls.stopDemo} />
      </Col> */}

    </Row>
  );
}

function ChartControls() {
  return chartControls;
}

export { ChartControls, colorsArrDefaults, colorsTitleArr };
