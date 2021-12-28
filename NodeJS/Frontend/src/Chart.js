import * as React from "react";
// import { Grid, Row, Col } from 'react-flexbox-grid';
import { SciChartSurface } from "scichart/Charting/Visuals/SciChartSurface";
import { NumericAxis } from "scichart/Charting/Visuals/Axis/NumericAxis";
import { FastLineRenderableSeries } from "scichart/Charting/Visuals/RenderableSeries/FastLineRenderableSeries";
import { XyDataSeries } from "scichart/Charting/Model/XyDataSeries";
import { ZoomPanModifier } from "scichart/Charting/ChartModifiers/ZoomPanModifier";
import { RubberBandXyZoomModifier } from "scichart/Charting/ChartModifiers/RubberBandXyZoomModifier";
import { MouseWheelZoomModifier } from "scichart/Charting/ChartModifiers/MouseWheelZoomModifier";
import { XAxisDragModifier } from "scichart/Charting/ChartModifiers/XAxisDragModifier";
import { YAxisDragModifier } from "scichart/Charting/ChartModifiers/YAxisDragModifier";
import { EDragMode } from "scichart/types/DragMode";
import { ZoomExtentsModifier } from "scichart/Charting/ChartModifiers/ZoomExtentsModifier";
import { ChartModifierBase2D } from "scichart/Charting/ChartModifiers/ChartModifierBase2D";

import { ENumericFormat } from "scichart/types/NumericFormat";
import { EAutoRange } from "scichart/types/AutoRange";
import { SciChartLegend } from "scichart/Charting/Visuals/Legend/SciChartLegend";
import { LegendModifier } from "scichart/Charting/ChartModifiers/LegendModifier";
import { ELegendOrientation, ELegendPlacement } from "scichart/Charting/Visuals/Legend/SciChartLegendBase";

import { NumberRange } from "scichart/Core/NumberRange"; import { WaveAnimation } from "scichart/Charting/Visuals/RenderableSeries/Animations/WaveAnimation";
import {EZoomState} from "scichart/types/ZoomState";
import { EXyDirection } from "scichart/types/XyDirection";
import { EExecuteOn } from "scichart/types/ExecuteOn";
import { easing } from "scichart/Core/Animations/EasingFunctions";

// import { EllipsePointMarker } from "scichart/Charting/Visuals/PointMarkers/EllipsePointMarker";
import { RolloverModifier } from "scichart/Charting/ChartModifiers/RolloverModifier";
import { TSciChart } from "scichart/types/TSciChart";
import { SciChartOverview } from "scichart/Charting/Visuals/SciChartOverview";
import { SciChartVerticalGroup } from "scichart/Charting/LayoutManager/SciChartVerticalGroup";
// import { IXyDataSeriesOptions} from "scichart/Charting/Model/XyDataSeries";

// import Webix from './Webix';
// import * as webix from 'webix/webix.js';
// import Config from './.config.js';
import { Context } from "./Context"

// import classes from "../../../../Examples/Examples.module.scss";
// import image from "./javascript-line-chart.jpg";
console.log("view chart");
const LICENSE_KEY = Context.chartkey;
// let scs: SciChartSurface;
// let timerId: NodeJS.Timeout;
const visiblePoints = 10000;
const intervalAddPoint = 40;
const suffixChartID = "scichart-root";
const suffixChartOverviewID = "scichart-overview";
let chartControls = [];
let chartSurfaces = [];

const colorsArrDefaults = ["#f6bf02","#0aa547","#eb4646", "blue", "#368BC1", "#eeeeee", "#ff6600", "#9b2dce", "#228B22", "#ff0000","orange","#be0000", "white"];
const colorsTitleArr = ["black","black","black", "black", "black", "black", "white", "white", "white", "white","white","white"];
const verticalGroup = new SciChartVerticalGroup();

class MyRubberBandZoomModifier extends RubberBandXyZoomModifier // ChartModifierBase2D
{
  modifierMouseDown(args) {
    console.log("args.button = " + args.button)
    console.log("args.ctrlKey = " + args.ctrlKey)
    if (args.ctrlKey == true)  {
      var pointTo = args.mousePoint;
      pointTo.x = args.mousePoint.x + 1000;
//    this.performZoom(args.mousePoint, pointTo)
      var xAxis = this.parentSurface.getXAxisById(this.xAxisId)
      if (xAxis) {
        xAxis.zoomBy(-0.25, -0.25);
      }
    }

    if (args.shiftKey == true)  {
      var xAxis = this.parentSurface.getXAxisById(this.xAxisId)
      if (xAxis) {
        xAxis.zoomBy(0.25, 0.25);
      }
    }

    args.handled = false;
    super.modifierMouseDown(args)
  }
}

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
    const divOverviewId = chartID+"_"+suffixChartOverviewID;    
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

// SciChartOverview.create(sciChartSurface, divOverviewId);

  // Create an X,Y Axis and add to the chart
  const xAxis = new NumericAxis(wasmContext, { autoRange: EAutoRange.Once });
//   chart1XAxis = new CategoryAxis(wasmContext, {
//     drawLabels: false,
//     drawMajorTickLines: false,
//     drawMinorTickLines: false
// });
  xAxis.drawLabels = true;
  xAxis.drawMajorGridLines = true;
  xAxis.drawMinorGridLines = true;
  xAxis.zoomExtentsToInitialRange = true; 

  let yMin = -4000;
  let yMax = 4000;
  const yAxis = new NumericAxis(wasmContext, { autoRange: EAutoRange.Always });
  yAxis.drawMajorGridLines = true;
  yAxis.drawMinorGridLines = true;
  yAxis.labelProvider.formatLabel = (dataValue: number) => dataValue.toFixed(3);
  yAxis.autoRange = EAutoRange.Once;
  yAxis.visibleRange = new NumberRange(yMin, yMax);
  yAxis.zoomExtentsToInitialRange = true; 
  yAxis.growBy = new NumberRange(0.2, 0.2);

  sciChartSurface.xAxes.add(xAxis);
  sciChartSurface.yAxes.add(yAxis);

  const cursor = new RolloverModifier({modifierGroup: "first"});
  cursor.visibility = true;
  sciChartSurface.chartModifiers.add(
//    new RubberBandXyZoomModifier({ xyDirection: EXyDirection.XyDirection, executeOn: EExecuteOn.MouseLeftButton }),
    new MouseWheelZoomModifier({ xyDirection: EXyDirection.XyDirection }),
    new XAxisDragModifier({ dragMode: EDragMode.Panning }),
    new YAxisDragModifier({ dragMode: EDragMode.Scaling }),
    new ZoomExtentsModifier({isAnimated: true, animationDuration: 400, easingFunction: easing.outExpo}),
    new ZoomPanModifier({executeOn: EExecuteOn.MouseRightButton}),    
    new MyRubberBandZoomModifier({ xyDirection: EXyDirection.XyDirection, executeOn: EExecuteOn.MouseLeftButton, receiveHandledEvents: true}),
    cursor
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
    console.log(clearChart);
    console.log(arrayLines);
    if (arrayLines[line-1]) {
      arrayLines[line-1].dataSeries.clear();
    } else {
      console.log("Error clearChart arrayLines[line-1] - not exists");
    }
  }

  const addVarPoint = (x,y,line) => {
      arrayLines[line-1].dataSeries.append(x, y);
      xAxis.visibleRange = new NumberRange(x-visiblePoints,x);
  }

  const addVarPointRange = (xValues = [], yValues = [], line) => {
    arrayLines[line-1].dataSeries.appendRange(xValues, yValues);

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

    if (sciChartSurface.zoomState !== EZoomState.UserZooming) {
      xAxis.visibleRange = new NumberRange(xValues[0]-visiblePoints, xValues[0]);
      yAxis.visibleRange = new NumberRange(yMin, yMax);
    }   
  }

  const addVarPoint2 = (x,y) => {
    arrayLines[1].dataSeries.append(x, y);
        xAxis.visibleRange = new NumberRange(x-visiblePoints,x);
  }

  const addVarPointRange2 = (xValues = [], yValues = []) => {
    arrayLines[1].dataSeries.appendRange(xValues, yValues);
        xAxis.visibleRange = new NumberRange(xValues[0]-visiblePoints, xValues[0]);
  }

  const ScalePlus = () => {
    xAxis.zoomBy(-0.25, -0.25);
    yAxis.zoomBy(-0.25, -0.25);
  };

  const ScaleMinus = () => {
    xAxis.zoomBy(0.25, 0.25);
    yAxis.zoomBy(0.25, 0.25);
  };

  const SwitchCursor = () => {
    console.log("SwitchCursor");
    cursor.isEnabled = !cursor.isEnabled;
  };
  
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

  verticalGroup.addSurfaceToGroup(sciChartSurface);
  chartSurfaces.push(sciChartSurface);
  return { wasmContext, sciChartSurface, controls: { startDemo, stopDemo, addVarPoint, addVarPoint2, addVarPointRange, addVarPointRange2, clearChart, ScalePlus, ScaleMinus, SwitchCursor} };
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
  const [controls, setControls] = React.useState({ startDemo: () => {}, stopDemo: () => {}, addVarPoint: () =>{}, addVarPoint2: () =>{}, addVarPointRange: () =>{}, addVarPointRange2: () =>{}, clearChart: () =>{}, 
  ScalePlus: () => {}, ScaleMinus: () => {}, SwitchCursor: () => {}});
  console.log(colorsArr);
  React.useEffect(() => {
    (async () => {
      console.log("namesArr");
      console.log(namesArr);
      console.log("colorsArr");
      console.log(colorsArr);
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
      console.log("props.id="+props.id);
      chartControls[props.id] = res.controls;
      console.log(chartControls);
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
  console.log("chart Context.chartList");
  console.log(Context.chartList);
  Context.addNamesColor(props.id,setNamesArr,setColorsArr);
  //Context.chartList[props.id].setNamesArr = setNamesArr;
  //Context.chartList[props.id].setColorsArr = setColorsArr;
  let currentChartID = props.id+"_"+suffixChartID;
  let overviewChartID = props.id+"_"+suffixChartOverviewID;

  return (
        //  <div id={currentChartID} style={{ width:"auto", height: "calc(var(--chartheight))", margin: "auto"}} ></div>
        //  <div id={overviewChartID} style={{ width:"auto", height: 70, margin: "auto"}} ></div>          

        <div id={props.id}  style={{ visibility:"hidden" }} >
          <div id={currentChartID} className="chart" ></div>
        </div>
          );
}

function ChartControls() {
  return chartControls;
}

function SyncCharts() {
  verticalGroup.synchronizeAxisSizes();

  let xAxes = []
  chartSurfaces.forEach(chart => {
    chart.zoomExtents();
    let xAxis = chart.xAxes.items[0];
    xAxes.push(xAxis);
  })

  xAxes.forEach(xAxis => {
    xAxis.visibleRangeChanged.subscribe((data1) => {
      xAxes.forEach(xAxis => {
        xAxis.visibleRange = data1.visibleRange;
      })
    });        
  });
}

export { ChartControls, SyncCharts, colorsArrDefaults, colorsTitleArr };
