import Webix from './Webix';
// import Chart from "./Chart2";
import React from "react";
// import * as webix from 'webix/webix.js';
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import { $$ } from 'webix';
import { Info } from './Context';
import moment from 'moment';


async function stopValues(param) {
  await param.closeValueStream();
}

async function getValue(param, rowId) {
  console.log("try to get value streams...");
// await param.closeValueStream();
// let value = await param.currentValue();
  let grid = $$("parametersGrid");
  let item = grid.getItem(rowId);

  let frequency = 2; // кол-во значений в секунду
  let resStream = await param.openValueStream(frequency);
  if (resStream === undefined || resStream === null) {
    return;
  }

  resStream.on('data', chunk => {
    let values = chunk;
    let value = values[values.length - 1];

    item.value = value.value.toFixed(3); 
    var dateStr = moment(value.valueTime).format('hh:mm:ss.SSS');
    item.time = dateStr;
    grid.updateItem(rowId, item);
  });
}

async function getValue2(param, rowId) {

  setInterval(async () => {

    let value = await param.lastValue();

    let grid = $$("parametersGrid");
    let item = grid.getItem(rowId);
    item.value = value.value;
    item.time = value.valueTime - Info.startTime;
    grid.updateItem(rowId, item);
  }, 500)
}

function getItems(tree, rowId) {
  let row = tree.getFirstChildId(rowId);
  let rowsList = [];
  if (row) {
    rowsList.push({ "id": row, "row": tree.getItem(row) });
    while (row = tree.getNextSiblingId(row)) {
      rowsList.push({ "id": row, "row": tree.getItem(row) });
    }
  }
  return rowsList;
}

function getUImainMenu(props) {

  return {
    view: "treetable", // "datatable"
    // width:0,
    // height:0,
    // id: "dataview",
    id: "parametersGrid",
    height: 353,
    columns: [
      {
        id: "name", header: "Name", fillspace: true,
        template: "{common.treetable()} #name#"
      },
      // {id:"name", header:"Name", fillspace:true}, //width:"250"
      { id: "value", header: "Value", width: "220" },
      { id: "dimension", header: "Dimension" },
      { id: "time", header: "Time" },
      {
        id: "chart", header: "Show on trend", width: "130",
        template: "{common.checkbox()}"
      },
      { id: "numchart", header: "Number of trend", width: "130" }
    ],
    on: {
      onAfterClose: function (id) {
        let tree = $$("parametersGrid");
        let rows = getItems(tree, id);
        rows.forEach(function (item, index, array) {
          item.row.param.closeValueStream();
        });
      },
      onAfterOpen: function (id) {
        let tree = $$("parametersGrid");
        let rows = getItems(tree, id);
        console.log("onAfterOpen=" + id);
        let arr = tree.getOpenItems();
        let index = arr.indexOf(id);
        arr.splice(index, 1);
        arr.forEach(function (item, index, array) {
          tree.close(item);
        });
        rows.forEach(function (item, index, array) {
          getValue(item.row.param, item.id);
        });
      }
    }
  }
}


function ParametersView(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);

  return (
    <div id="dataview">
      <WebixComponent ui={getUImainMenu(props)} data={props.data} />
    </div>
  );
}

export default ParametersView;