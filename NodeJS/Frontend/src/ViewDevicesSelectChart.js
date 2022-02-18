import React from "react";
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import { $$, template } from 'webix';
import { Context, addCssClass } from './Context';

let colorsArrTab = [];
let selectedParams = [];

function mark_votes(value, config){
  if (value > 0)
      // return { "background":colorsArrTab[value-1], "color":colorsTitleArr[value-1] };
      return { "background":colorsArrTab[value-1], "color":colorsArrTab[value-1] };
  else 
      return { "background":colorsArrTab[12], "color":"white" };
};

const clearDataTable = () => {
  let table1 = $$("showSelectChartWindowData");
  selectedParams.forEach(function(item, index, array) {
    let item2 = table1.getItem(item.row);
    item2.line = 0;
  });
  if (table1) {
    table1.refresh();
  } else {
    console.log("Error ViewDevicesSelectChart not exist element #showSelectChartWindowData");
  }
  selectedParams = [];
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
          columns:[
            { id:"channel", header:"Channel", width:120, css:"center"},
            { id:"name", header:["Name", {content:"textFilter" }], fillspace:1,  },
            { id:"status", header:["Show", {content:"masterCheckbox"}], width:80, css:"center", 
                  template:"{common.checkbox()}"},
            { id:"line",	//editor:"combo", 
             cssFormat:mark_votes,
              header:"Num Line", width:300},
          ],
          data: [],
          on: {
            onCheck: function(row, column, state){
              // console.log("onCheck");
              let table1 = $$("showSelectChartWindowData");
              let item = table1.getItem(row);
              if (item.name !== "—") {
                  let showParam = {name: item.name, channel: item.channel, idParam: item.idParam, row: row, color: item.color};
                  
                  if (state == 1) {
                      selectedParams.push(showParam);
                      //console.log(colorsArrTab);
                      colorsArrTab.push(item.color);
                      item.line = selectedParams.length;
                  }
                  if (state == 0) {
                      let newParamArr = [];
                      let newColorsArrTab = [];
                      selectedParams.forEach(function(item3, index, array) {
                          //console.log(item3);  
                          if(item3.channel == item.channel) {
                          }
                          else {
                            newParamArr.push(item3);
                            newColorsArrTab.push(colorsArrTab[index]);
                          }
                      });
                      selectedParams = newParamArr;
                      colorsArrTab = newColorsArrTab;
                      item.line = 0;
                      selectedParams.forEach(function(item2, index, array) {
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

                  let visibleCharts = Context.deviceCharts(Context.states.indexDevice);
                  let difference = Context.oscilloscopeChartList.filter(x => !visibleCharts.includes(x));
                  let chartToVisible = difference.shift();
                  if (chartToVisible) {

                    let paramArr = [];
                    selectedParams.forEach(function(item, index, array) {
                        paramArr.push(item.name);
                    });
                    let chartParams = new Map();
                    if (Context.paramToCharts.has(Context.states.indexDevice)) {
                      chartParams = Context.paramToCharts.get(Context.states.indexDevice);
                    }
                    chartParams.set(chartToVisible, selectedParams);

                    Context.paramToCharts.set(Context.states.indexDevice, chartParams);
                    let params = Context.paramToCharts.get(Context.states.indexDevice);

                    Context.chartList[chartToVisible].setColorsArr(colorsArrTab);
                    Context.chartList[chartToVisible].setNamesArr(paramArr);

                    Context.addChart(Context.states.indexDevice, chartToVisible);
                  }
              
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