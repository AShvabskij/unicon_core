import * as React from "react";
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

import { Context, setStyleByID } from "../Context"

// import image from "./javascript-line-chart.jpg";
const LICENSE_KEY = Context.chartkey;
const visiblePoints = 10000;
const intervalAddPoint = 40;
const suffixChartID = "scichart-root";
const suffixChartOverviewID = "scichart-overview";
let chartSurfaces = [];

// const colorsArrDefaults = ["#f6bf02","#0aa547","#eb4646", "blue", "#368BC1", "#eeeeee", "#ff6600", "#9b2dce", "#228B22", "#ff0000","orange","#be0000", "white"];
const colorsTitleArr = ["black","black","black", "black", "black", "black", "white", "white", "white", "white","white","white"];

let verticalGroup = new SciChartVerticalGroup();

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

async function initSciChart(chartID , params) {
    const divElementId = chartID + "_" + suffixChartID;
    const divOverviewId = chartID + "_" + suffixChartOverviewID;    
    let timerLocalID
    var overviewElem = null;
/*
    SciChartSurface.setServerLicenseEndpoint("http://localhost:3000/api/license");
*/    
    SciChartSurface.setRuntimeLicenseKey(
      LICENSE_KEY
    );

/*
SciChartSurface.setLicenseCallback(function () { return fetch("localhost:3000/api/license",  {mode: "no-cors"}).then(function (r) {
  if (r.ok) {
      console.log(r.text())
      return r.text();
  }
  return "";
}); });
*/
  // Create the SciChartSurface in the div 'scichart-root'
  // The SciChartSurface, and webassembly context 'wasmContext' are paired. This wasmContext
  // instance must be passed to other types that exist on the same surface.
  const { sciChartSurface, wasmContext } = await SciChartSurface.create(
    divElementId
  );
  
  // Create an X,Y Axis and add to the chart
  const xAxis = new NumericAxis(wasmContext, { autoRange: EAutoRange.Once });
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
  cursor.isEnabled = false;

  const lm = new LegendModifier({
    placement: ELegendPlacement.TopLeft,
    orientation: ELegendOrientation.Vertical,
    showLegend: true,
    showCheckboxes: true,
    showSeriesMarkers: true
  });

  sciChartSurface.chartModifiers.add(
//    new RubberBandXyZoomModifier({ xyDirection: EXyDirection.XyDirection, executeOn: EExecuteOn.MouseLeftButton }),
    new MouseWheelZoomModifier({ xyDirection: EXyDirection.XyDirection }),
    new XAxisDragModifier({ dragMode: EDragMode.Panning }),
    new YAxisDragModifier({ dragMode: EDragMode.Scaling }),
    new ZoomExtentsModifier({isAnimated: true, animationDuration: 400, easingFunction: easing.outExpo}),
    new ZoomPanModifier({executeOn: EExecuteOn.MouseRightButton}),
    new MyRubberBandZoomModifier({ xyDirection: EXyDirection.XyDirection, executeOn: EExecuteOn.MouseLeftButton, receiveHandledEvents: true}),
    lm,
    cursor
  );

  let counter = 0;
  

  if (params) {
    let namesArr = []; 
    let colorsArr = [];
    
    for (let param of params) {
      namesArr.push(param.name);
      colorsArr.push(param.color);
    }

    await parametriseSciChart(sciChartSurface, wasmContext, namesArr, colorsArr);
  }

  async function parametriseSciChart(sciChartSurface /*: SciChartSurface*/, wasmContext /*: TSciChart*/, namesArr = [], colorsArr = []) {

    const seriesArr = [];
    for (let k = 0; k < namesArr.length; k++) {
        seriesArr.push({color:colorsArr[k],name:namesArr[k],colorText:colorsTitleArr[k]});
    }
  
    if (sciChartSurface.renderableSeries) {
      sciChartSurface.renderableSeries.clear();
    }
  
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
  
      await sciChartSurface.renderableSeries.add(arrayLines[m]);
    }
  }
  
  const stopDemo = () => {
    clearInterval(timerLocalID);
    xAxis.autoRange = EAutoRange.Once;
  };

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

  const defFuncts = [defFunct1, defFunct2, defFunct3, defFunct4, defFunct5];

  const addPoint = () => {
    let arrayLines = sciChartSurface.renderableSeries.items;
    const step = 100;

    for(let i = counter; i < (counter+step); i++) {
      for (let m = 0; m < arrayLines.length; m++) {
        console.log("defFunct5" + defFuncts[m]);
        arrayLines[m].dataSeries.append(i, defFuncts[m](i));
      }

      if (i>1000000) {
        arrayLines[0].dataSeries.removeAt(0);
      }
    }
    xAxis.visibleRange = new NumberRange(counter-visiblePoints, counter+step);
    counter = counter +step;
  }

  const clearChart = (line) => {
    let arrayLines = sciChartSurface.renderableSeries.items;
    if (arrayLines[line-1]) {
      arrayLines[line-1].dataSeries.clear();
    } else {
      console.log("Error clearChart arrayLines not exists, line = " + line);
    }
  }

  const addVarPoint = (x,y,line) => {
    let arrayLines = sciChartSurface.renderableSeries.items;
      arrayLines[line-1].dataSeries.append(x, y);
      xAxis.visibleRange = new NumberRange(x-visiblePoints,x);
  }

  const addVarPointRange = (xValues = [], yValues = [], line) => {
    let arrayLines = sciChartSurface.renderableSeries.items;
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

  const addVarPointMegaRange = (xValues = [], yValues = [], line) => {
    let arrayLines = sciChartSurface.renderableSeries.items;
    arrayLines[line-1].dataSeries.appendRange(xValues, yValues);
  }

  const zoomExtents = () => {
    sciChartSurface.zoomExtents();
  }

  const Scale = (factorX, factorY) => {
    xAxis.zoomBy(factorX, factorX);
    yAxis.zoomBy(factorY, factorY);
  };

  const SwitchCursor = (value) => {
    cursor.isEnabled = value;
  };

  const SwitchPreview = async (isVisible) => {
    var visibility = isVisible ? "visible" : "hidden";
//  document.getElementById(divOverviewId).style["visibility"] = visibility;

    if (isVisible) {
      document.documentElement.style.setProperty('--overview-size', '70px');
      
      if (!overviewElem) {
        setTimeout( async ()=> {
          overviewElem = await SciChartOverview.create(sciChartSurface, divOverviewId);
        }, 100)
      }
      
    } else {
      document.documentElement.style.setProperty('--overview-size', '1px');
    }
  };

  const startDemo = () => {
    clearInterval(timerLocalID);
    timerLocalID = setInterval(addPoint, intervalAddPoint);
  };

  verticalGroup.addSurfaceToGroup(sciChartSurface);
  chartSurfaces.push(sciChartSurface);
  return { wasmContext, sciChartSurface, controls: { startDemo, stopDemo, addVarPoint, addVarPointRange, addVarPointMegaRange, zoomExtents, clearChart, Scale, SwitchCursor, SwitchPreview} };
}

export default function Chart(props) {
  const [loaded, setLoaded] = React.useState(false);  
  const [inited, setInited] = React.useState(false);  
  const [sciChartSurface, setSciChartSurface] = React.useState(null);
  const [wasmContext, setWasmContext] = React.useState(null);

  const [visibility, _setVisibility] = React.useState('hidden');
  const [controls, setControls] = React.useState({ startDemo: () => {}, stopDemo: () => {}, addVarPoint: () =>{}, addVarPointRange: () =>{}, clearChart: () =>{}, 
  Scale: (factorX, factorY) => {}, SwitchCursor: (value) => {}, SwitchPreview: (value) => {},  setVisibility: (value) => {}});

  const setVisibility = (value) => {
    _setVisibility(value);
  }

  const _switchChartElement = (visibility = "visible") => {
    let elemId = props.id;
    setStyleByID(elemId, "visibility", visibility);
  };

  React.useEffect(() => {
    _switchChartElement(visibility);
  }, [visibility]); 
 
  React.useEffect(() => {
    (async () => {
      if (sciChartSurface?.renderableSeries) {
        for (let line of sciChartSurface.renderableSeries.items) {
          line.dataSeries.clear();
        }
        sciChartSurface.renderableSeries.clear();
      }
    
      sciChartSurface?.delete();
      setSciChartSurface(null);

      setInited(false);
      setLoaded(false);

      const newControls = {setVisibility};
      setControls(newControls);
      Context.chartControls[props.id] = newControls;


      setTimeout(async () => { 
        let res = await initSciChart(props.id, props.params); // отложенная опреацию создания необходима, так как диаграммы требует фиксированной высоты div блока, которая должна успеть рассчитаться
        if (!res) return;
  
        setInited(true);

        const newControls = Object.assign({}, res.controls, {setVisibility});
        setControls(newControls);
        Context.chartControls[props.id] = newControls;
        setSciChartSurface(res.sciChartSurface);
        setWasmContext(res.wasmContext);
      }, 10) 
    })();

    return () => {
    // Delete sciChartSurface on unmount component to prevent memory leak
      verticalGroup = new SciChartVerticalGroup();
      chartSurfaces = [];
      sciChartSurface?.delete();
    };

  }, [props.params, props.chartCount]);

  React.useEffect(() => {
    (async () => {
      if (inited && !loaded && props.loadData) {
        setTimeout(async () => {
          await props.loadData(props.id);
          setLoaded(true);
        }, 300);
      }
    })();
  }, [inited, loaded]);

  const divElementId = props.id + "_" + suffixChartID;
  const divOverviewId = props.id + "_" + suffixChartOverviewID;    

  return (
    //  <div id={divElementId} style={{ width:"auto", height: "calc(var(--chartheight))", margin: "auto"}} ></div>
    //  <div id={divOverviewId} style={{ width:"auto", height: 70, margin: "auto"}} ></div>          
    
    <div id={props.id} className="chartContainer" style={{ display:"grid"}}>
      <div id={divElementId} className="chart"></div>
      <div id={divOverviewId} className="chartoverview"></div>
    </div>
  );
}

function SyncCharts() {
  
  verticalGroup.synchronizeAxisSizes();

  let xAxes = []
  chartSurfaces.forEach(chart => {
    chart.zoomExtents();
    let xAxis = chart.xAxes.items[0];
    xAxes.push(xAxis);

    let yAxis = chart.yAxes.items[0];
    yAxis.growBy = new NumberRange(0.2, 0.2);
    
  })

  xAxes.forEach(xAxis => {
    xAxis.visibleRangeChanged.subscribe((data1) => {
      xAxes.forEach(xAxis => {
        xAxis.visibleRange = data1.visibleRange;
      })
    });        
  });
  
}

export { SyncCharts };
