import React from "react";
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import { $$, template } from 'webix';
import {ChartControls} from "./Chart";
import { Context,showElementChart } from './Context';
import {chartsArrVisible,chartsArrHidden} from "./ViewDevicesOscilloscope"

let colorsArrTab = [];

function mark_votes(value, config){
  if (value > 0)
      // return { "background":colorsArrTab[value-1], "color":colorsTitleArr[value-1] };
      return { "background":colorsArrTab[value-1], "color":colorsArrTab[value-1] };
  else 
      return { "background":colorsArrTab[12], "color":"white" };
};



const clearDataTable = () => {
  let table1 = $$("showSelectChartWindowData");
  Context.paramToChart.forEach(function(item, index, array) {
    let item2 = table1.getItem(item.row);
    item2.line = 0;
  });
  if (table1) {
    table1.refresh();
  } else {
    console.log("Error ViewDevicesSelectChart not exist element #showSelectChartWindowData");
  }
  Context.paramToChart = [];
  colorsArrTab = [];
}

const ViewDevicesSelectChartUI = () => {
  let v = {
    view:"window",
    id:"showSelectChartWindow",
    height:600,
    width:1000,
    left:50, top:50,
    // css:"showSelectChartWindowData",
    move:true,
    modal:true,
    head:"Select oscilloscope channels",
    body:{
      rows: [
        {
          id: "showSelectChartWindowData",   
          view:"datatable",
          height:500, 
          // scheme:{
          //   $change:function(item){
          //     if (item.status.state == 1)
          //       item.$cellCss = {line:"green"};
          //   }
          // },
          columns:[
            { id:"channel", header:"Channel", width:120, css:"center"},
            { id:"name", header:["Name", {content:"textFilter" }], fillspace:1,  },
            { id:"status", header:["Show", {content:"masterCheckbox"}], width:80, css:"center", 
                  template:"{common.checkbox()}"},
            { id:"line",	//editor:"combo", 
             cssFormat:mark_votes,
              // options:[
              //   {id:"a", value: ""},
              //   {id:1, value: "1"},
              //   {id:2, value: "2"},
              //   {id:3, value: "3"},
              //   {id:4, value: "4"},
              //   {id:5, value: "5"},
              //   {id:6, value: "6"} // {id:2, $css: "colorChart1", value: "2"}
              // ], //collection:colorsArr,
              // css: "colorChart1",
              header:"Num Line", width:300},
          ],
          //editable:true,
          // checkboxRefresh:true,
          // autoheight:true,
          data: [],
          on: {
            onCheck: function(row, column, state){
              console.log("onCheck");
              let table1 = $$("showSelectChartWindowData");
              let item = table1.getItem(row);
              if (item.name !== "—") {
                  let showParam = {name: item.name, channel: item.channel, idParam: item.idParam, row: row};
                  
                  if (state == 1) {
                    Context.paramToChart.push(showParam);
                      //console.log(colorsArrTab);
                      colorsArrTab.push(item.color);
                      item.line = Context.paramToChart.length;
                  }
                  if (state == 0) {
                      let newParamArr = [];
                      let newColorsArrTab = [];
                      Context.paramToChart.forEach(function(item3, index, array) {
                          //console.log(item3);  
                          if(item3.channel == item.channel) {
                            //console.log("continue");
                          }
                          else {
                            newParamArr.push(item3);
                            newColorsArrTab.push(colorsArrTab[index]);
                          }
                      });
                      Context.paramToChart = newParamArr;
                      colorsArrTab = newColorsArrTab;
                      
                      item.line = 0;
                      Context.paramToChart.forEach(function(item2, index, array) {
                        let item4 = table1.getItem(item2.row);
                        item4.line = index + 1;
                        table1.updateItem(item2.row, item4);
                      })
                    }
                  }
              else {
                      item.status = 0;
                  }
              table1.updateItem(row, item);
           },
            /* onAfterEditStop: function(state, editor, ignoreUpdate){
              //  console.log(editor.row);
              //  console.log("paramToCharts = " + paramToCharts);
               let table1 = this.getColumnConfig("line").collection;
               if ((editor.value) && (editor.value != "a")) table1.config.data[editor.value].disabled = false;
               if ((state.value) && (state.value != "a")) table1.config.data[state.value].disabled = true;
               let item = this.getItem(editor.row);
                // console.log(item);
                let showParam = {name: item.name, channel: item.channel, idParam: item.idParam};
                paramToCharts.push(showParam);
            }, */
            /* onCheck: function(row, column, state){
              console.log("onCheck");
              console.log("paramToCharts init");
              console.log(paramToCharts);
              console.log(row);
              console.log(column);
              console.log(state);
              let item = this.getItem(row);
              // console.log(item);
              let showParam = {name: item.name, channel: item.channel, idParam: item.idParam, row: row};
              let table1 = $$("showSelectChartWindowData");
              let item1 = table1.getItem(row);
              if (state == 1) {
                  paramToCharts.push(showParam);
                  console.log("paramToCharts Add");
                  console.log(paramToCharts);
                  item1.line = paramToCharts.length;
                  table1.updateItem(row, item1);
                  console.log("paramToCharts end add");
                  console.log(paramToCharts);
                  
              }
              if (state == 0) {
                  console.log("paramToCharts init remove");
                  console.log(paramToCharts);
                  item1.line = "a";
                  table1.updateItem(row, item1);
                  console.log(showParam);
                  let posId = paramToCharts.indexOf(showParam);
                  console.log("posId = " + posId);
                  paramToCharts.splice(posId, 1);
                  console.log(paramToCharts.length);
                  paramToCharts.forEach(function(item2, index, array) {
                    // console.log(item2.row);
                    let item4 = table1.getItem(item2.row);
                    // console.log(item4);
                    item4.line = index + 1;
                    table1.updateItem(item2.row, item4);
                  })
              }
              console.log("item1.line="+item1.line);
              // table1.updateItem(row, item1);
           }, */
          }
        },
        {
          height: 38,
          cols: [
            { "label": "Cancel", "view": "button", "height": 0, 
                click: function (id, event) {
                  // console.log("webixButton");
                  $$("showSelectChartWindow").hide();
                  clearDataTable();
                }
            },
            { "label": "Apply", "view": "button", "height": 0, 
                click: function (id, event) {
                    let chartToVisible = chartsArrHidden.shift();
                    let paramArr = [];
                    Context.paramToChart.forEach(function(item, index, array) {
                        paramArr.push(item.name);
                      });
                    Context.paramToCharts[chartToVisible] = Context.paramToChart;
                    console.log("Context.chartList");
                    console.log(Context.chartList);
                    Context.chartList[chartToVisible].setColorsArr(colorsArrTab);
                    Context.chartList[chartToVisible].setNamesArr(paramArr);
                    chartsArrVisible.push(chartToVisible);
                    console.log("Select chart click");
                    let chControl = ChartControls();
                    console.log(chControl[chartToVisible]);
                    // chControl[chartToVisible].setColorsArr(colorsArrTab);
                    // chControl[chartToVisible].setNamesArr(paramArr);
                    console.log("chartToVisible="+chartToVisible);
                    console.log(colorsArrTab);
                    console.log(paramArr);
                    document.documentElement.style.setProperty('--chartcount', chartsArrVisible.length);
                    // Alex
                    showElementChart(chartToVisible);
                  $$("showSelectChartWindow").hide();
                  clearDataTable();
                }
            }
          ]
        }
      ]
    }
  }
  return v;
}




export default class ViewDevicesSelectChart extends React.Component {
  constructor(props) {
    super(props);
    
  };


  render() {

       return (
        <WebixComponent ui={ViewDevicesSelectChartUI()}  data={[]} />
       )
  }

}