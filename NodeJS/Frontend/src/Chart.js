import * as React from "react";
import { SciChartSurface } from "scichart/Charting/Visuals/SciChartSurface";
import { NumericAxis } from "scichart/Charting/Visuals/Axis/NumericAxis";
import { FastLineRenderableSeries } from "scichart/Charting/Visuals/RenderableSeries/FastLineRenderableSeries";
import { XyDataSeries } from "scichart/Charting/Model/XyDataSeries";
import {ZoomPanModifier} from "scichart/Charting/ChartModifiers/ZoomPanModifier";


// import classes from "../../../../Examples/Examples.module.scss";
// import image from "./javascript-line-chart.jpg";
const divElementId = "scichart-root";
const LICENSE_KEY = "JebThLVzGedXCDlzciqgNot7DqccqCsEy353wNG/duRRZbVHtULY5LkVxriRE9pvdR5Rb6d9n/cPeIlSl+oHHoiPvEe0Ne9GvOi2ca4jPZOcT7LBhfMm8jnbw6HlTQ6ULpqGVLnBgYReikDAPICyhlZjo/wgqLW/eQuzl5OUnRd7OJ3UfpBj8mvNQtx9OZttFrE8ARTLYgEDFcnzK49g8Z/86pgEb5/sm3w62bumsJM8h0aTg9S+UJTRqhYZC3lUBnfAMFFmcAXlTvs6c64Ln/KVM56TwrvUNhJ1pp6PZgqsOtRIyWDO/JkxoTwltPEFdcov3UcO2ElU2dHCIhTIhD6LB1qdciAZmBgTHoygmnaB/WmVZYcY5vPeI3u+/x6YtT+6cE5Nz7IOThNjTmzhcfC+JcVNwYzURbDssxYYayY5ct78AcnsN4bvDofvxrw9ASfBqRDIFSnguJzxEar9TUZwu7ZMOYSkuQFK0JIagW1E9xMAD4SvCd1K2naAlKkCoB05ovSPohiHqpVMn15t0UEpoJfjmrSOnkyj8Nbxe0P7F4zZ+Jdue1w96nec8g==";

// const {Model, SysInterfacesEnum}  = require("./data_model/model.js");
// let model = new Model();

// const deviceId = 12;
// const paramId = 65;


// let param = model.device(deviceId).param(paramId);
//   let resStream = await param.openValueStream();

//   resStream.on('data', chunk => {
//     let stringifiedRes = chunk.toString();
//     console.log(`Received from stream: ${stringifiedRes}`);

//     // if (frontWebSocket) {
//     //   frontWebSocket.send(stringifiedRes);
//     // }
//   });

async function initSciChart1() {
    //SciChartSurface.setRuntimeLicenseKey("UUCri6yRUBOQF2fHKc2q541S3QcDhDILUXA1Sw7ieeu3dzMO2OtPuz0Q0FoPcJtRo7/TgRSGNoWE2m/jrjpHavfZzn2mvC4Nm4ZswDBG7siecqTntUjY259CX8gZwbpRWIPetmGpeZbjqPVDKFAphZNxZKiE/q1pe0mU6o/uQITAOIzJQMFR26jXvGfQmlCyaJ/iHGsmmEGYqpGrr3yzHkk4J9CjvdFA4cq3lq54WJ0GuC4BYM/hjRKbOG3mASb8naAE2PAsBUdkWNH8HmUmBqgEoYqfhmFu0ynU/RAZbHr0/BY3s1TMsex2jJP7l0gdIFjP79Fhvnrp13FxwzP3jTsQ5e2YQNllaO8gAJXttNk5zBESSVkpf4g6VCz0ZjKhVIET7fJ4N6w342ELKJIRRR+rsSI6e8chszgQhIVMkxKvmfoLxN+Iap67Z46MOFkxVPyTy4Sy/XBVemkm+JRE4fD6u2bioqqWAApE5Krk7s8740UpU3+s/Szo0fxNMjAsGZS4VlDcSNL6FfQ1oSBpOgOuZIleUUnlwkbgvRhGN1rjfDD5");
    //
    // Also, once activated (trial or paid license) having the licensing wizard open on your machine
    // will mean any or all applications you run locally will be fully licensed.

    // Create the SciChartSurface in the div 'scichart-root'
    // The SciChartSurface, and webassembly context 'wasmContext' are paired. This wasmContext
    // instance must be passed to other types that exist on the same surface.
    const { sciChartSurface, wasmContext } = await SciChartSurface.create(
        "scichart-root"
      );
    // Create an X,Y Axis and add to the chart
    const xAxis = new NumericAxis(wasmContext);
    const yAxis = new NumericAxis(wasmContext);
    
    sciChartSurface.xAxes.add(xAxis);
    sciChartSurface.yAxes.add(yAxis);    
    
    // Create 100 dataseries, each with 10k points
    for (let seriesCount = 0; seriesCount < 100; seriesCount++) {        
        const xyDataSeries = new XyDataSeries(wasmContext);

        const opacity = ((1 - ((seriesCount / 120)))*0.5).toFixed(2);

        // Populate with some data
        for(let i = 0; i < 10000; i++) {
            xyDataSeries.append(i, Math.sin(i* 0.01) * Math.exp(i*(0.00001*(seriesCount+1))));
        }

        // Add and create a line series with this data to the chart
        // Create a line series        
        const lineSeries = new FastLineRenderableSeries(wasmContext, {
            dataSeries: xyDataSeries, 
            stroke: `rgba(176,196,222,${opacity})`,
            strokeThickness:1
        });
        console.log("xyDataSeries-----------------------------------------");
        console.log(xyDataSeries);
        sciChartSurface.renderableSeries.add(lineSeries);
    }
};
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
      "scichart-root"
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
    // sciChartSurface.renderableSeries.add(lineSeries);
  
    lineSeries.dataSeries = new XyDataSeries(wasmContext, {
      xValues: [50, 300, 9050],
      yValues: [50, 300, 1050],
    });
  
    // Create 100 dataseries, each with 10k points
    for (let seriesCount = 0; seriesCount < 100; seriesCount++) {        
      const xyDataSeries = new XyDataSeries(wasmContext);

      const opacity = ((1 - ((seriesCount / 120)))*0.5).toFixed(2);

      // Populate with some data
      for(let i = 0; i < 5000; i++) {
          xyDataSeries.append(i, Math.sin(i* 0.01+1)*seriesCount + Math.cos(i* 0.01+1)*seriesCount);
      }

      // Add and create a line series with this data to the chart
      // Create a line series        
      const lineSeries = new FastLineRenderableSeries(wasmContext, {
          dataSeries: xyDataSeries, 
          stroke: `rgba(176,196,222,${opacity})`,
          strokeThickness:1
      });
      console.log("xyDataSeries-----------------------------------------");
      console.log(xyDataSeries);
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
        <div >
            
            <div id={divElementId} style={{ width: 600, margin: "auto" }} ></div>
           
        </div>
    );
}