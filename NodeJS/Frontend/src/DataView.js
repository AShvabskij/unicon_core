import Webix from './Webix';
// import Chart from "./Chart2";
import React from "react";
// import * as webix from 'webix/webix.js';
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import {$$} from 'webix';
import { Info } from './Context';


async function getValue(param,rowId) {
  console.log("try to get param value...");
  await param.closeValueStream();
  let resStream = await param.openValueStream();
  resStream.on("data", chunk => {
    let stringifiedRes = chunk.toString();
    // console.log(`Received from stream: ${stringifiedRes}`);
    let value = JSON.parse(stringifiedRes);
    // let xValue = (value.valueTime & 0xFFFF) * 0.05;
    let xValue = value.valueTime - Info.startTime;
    let yValue = value.value;
    // console.log (xValue +“,”+ yValue);
    if (yValue != -1) {
      let grid = $$("parametersGrid");
      let item = grid.getItem(rowId);
      item.value = yValue;
      item.time = xValue;
      // grid.refresh();
      grid.updateItem(rowId,item);
    }
  });
}

async function getValue1(deviceId = 1, paramId = 65) {
  console.log("try to get value...");
  let startDate = new Date();
  Info.model.device(deviceId).param(paramId).lastValue().then(result => {
    var paramValue = result;
    var message = `param id = ${paramValue.paramId}, value = ${paramValue.value}, value format = ${paramValue.valueFormat}`;
    console.log(message);
  });
  let param = Info.model.device(deviceId).param(paramId);
  await param.closeValueStream();
  let resStream = await param.openValueStream();
  resStream.on("data", chunk => {
    let stringifiedRes = chunk.toString();
    // console.log(`Received from stream: ${stringifiedRes}`);
    let value = JSON.parse(stringifiedRes);
    // let xValue = (value.valueTime & 0xFFFF) * 0.05;
    let xValue = value.valueTime - Info.startTime;
    let yValue = value.value;
    // console.log (xValue +“,”+ yValue);
    if (yValue != -1) {
      console.log(yValue);
    }
  });
}


function getItems(tree,rowId) {
  let row = tree.getFirstChildId(rowId);
  let rowsList = [];
  if (row) {
    rowsList.push({"id":row,"row":tree.getItem(row)});
    while ( row = tree.getNextSiblingId(row)) {
      rowsList.push({"id":row,"row":tree.getItem(row)});
    }
  }
  return rowsList;
}

function getUImainMenu(props) {
  
	return { view:"treetable", // "datatable"
			// width:0,
			// height:0,
			// id: "dataview",
      id: "parametersGrid",
			height:353,
			columns:[
				{id:"modul", header:"Modul", width:"250",
          template:"{common.treetable()} #modul#"},
        {id:"name", header:"Name", fillspace:true}, //width:"250"
				{id:"value", header:"Value", width:"90"},
				{id:"dimension", header:"Dimension"},
				{id:"time", header:"Time"},
				{id:"chart", header:"Show on trend", width:"130"},
				{id:"numchart", header:"Number of trend", width:"130"}
			],
      on : {
        onAfterClose :function(id){
          let tree = $$("parametersGrid");
          let rows = getItems(tree,id);
          rows.forEach(function(item, index, array) {
               item.row.param.closeValueStream();
          });
        },
        onAfterOpen :function(id){
          let tree = $$("parametersGrid");
          let rows = getItems(tree,id);
          console.log("onAfterOpen="+id);
          let arr = tree.getOpenItems();
          let index = arr.indexOf(id);
          arr.splice(index,1);
          arr.forEach(function(item, index, array) {
            tree.close(item);
          });
          rows.forEach(function(item, index, array) {
            // getValue(item.row.param,item.id);
          });
        } 
      }
		}
}


function DataView(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);

  return ( 
      <div id="dataview">
        <WebixComponent ui={getUImainMenu(props)} data={props.data} />
      </div> 
  );
}

export default DataView;