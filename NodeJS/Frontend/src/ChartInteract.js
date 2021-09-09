import * as React from "react";
import { SciChartSurface } from "scichart/Charting/Visuals/SciChartSurface";
import { NumericAxis } from "scichart/Charting/Visuals/Axis/NumericAxis";
import { FastLineRenderableSeries } from "scichart/Charting/Visuals/RenderableSeries/FastLineRenderableSeries";
import { XyDataSeries } from "scichart/Charting/Model/XyDataSeries";
import {ZoomPanModifier} from "scichart/Charting/ChartModifiers/ZoomPanModifier";


// import classes from "../../../../Examples/Examples.module.scss";
// import image from "./javascript-line-chart.jpg";
const divElementId = "scichart-root1";
const LICENSE_KEY = "Ms4s7kPNHtGh/C9XitPAzhFJxW+ZFPkuVWnfhrARTR8oaE/D/8/yqLbAgcp2sQdVuva50wv5PkKiiJ0Asd7iYW3xGE9OvtnG6TZwbI4PPVq773RfvLjesvEkaB0u56BbKnAfsg/pCgTGbfRUgDCNqxs5DWUwXGXJPf3WIeTGHAU58XU4o1p6yQ3gN8/3kjE5wYIRJmebm1+Z+NCq67sol9pNpoOUfx13Mo4kURoFw24t+lwtTOmvpbLadWLn1v/wR7QlMf0kf9IrgnW8D5aoytdjoZs+XXVzRBF7EmjT5if72KszBF8EkX4aFYaEbsuhNqcVN+ovRhrYLFy8KfcGuGTZsiI0cv51IAKvHOkrLFeokv64vlKEBehLLkiWfcKkAuMUMMBrdars6BDo2nCHltAh2E01GNGG4Ah8OmFlQ7+x1rtol4LmxsIfB5CAhXHSGkt75XAlfiYv827ljmvNBGRNXu/JOHwBqlLdIesFeQtWz8PBAwowPQ/5c+mA+6oXt/heSmxUg0IgK6YdOHsj/7/SbW+OGqYLjhEn90aDA1YhPPqPaSbE3qhD";

async function initSciChart() {
    // Below find a trial / BETA key for SciChart.js.
    // This Expires in 30 days - or 14th November 2020
    // Set this license key once in your app before calling SciChartSurface.create, e.g.
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
    const xAxis = new NumericAxis(wasmContext);
    const yAxis = new NumericAxis(wasmContext);
  
    sciChartSurface.xAxes.add(xAxis);
    sciChartSurface.yAxes.add(yAxis);
  
    const lineSeries = new FastLineRenderableSeries(wasmContext, {
      stroke: "orange",
    });
    lineSeries.strokeThickness = 3;
    sciChartSurface.renderableSeries.add(lineSeries);
  
    lineSeries.dataSeries = new XyDataSeries(wasmContext, {
      xValues: [150, 700, 9050],
      yValues: [150, 700, 1050],
    });
    // console.log(lineSeries);
    // console.log(lineSeries);
    //lineSeries.dataSeries[0].appendRange(1000,900)
  
    // Create 100 dataseries, each with 10k points
    for (let seriesCount = 0; seriesCount < 100; seriesCount++) {        
      const xyDataSeries = new XyDataSeries(wasmContext);

      const opacity = ((1 - ((seriesCount / 120)))*0.5).toFixed(2);

      // Populate with some data
      for(let i = 0; i < 10; i++) {
          xyDataSeries.append(i, Math.sin(i* 0.01) * Math.exp(i*(0.00001*(seriesCount+1))));
      }

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

    sciChartSurface.chartModifiers.add(new ZoomPanModifier());
  
    // That's it! You just created your first SciChartSurface!
  }

export default function Chart() {
    React.useEffect(() => {
        console.log("on load");
        initSciChart();
      }, []);

    return (
        <div id="chart1" style={{ visibility:"hidden" }}>
            <h1> Hi </h1>
            <div id={divElementId} style={{ width: 800, margin: "auto" }} ></div>
           
        </div>
    );
}