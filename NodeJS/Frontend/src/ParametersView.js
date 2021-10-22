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
//let item = grid.getItem(rowId);

  let frequency = 2; // кол-во значений в секунду
  let resStream = await param.openValueStream(frequency);
  if (resStream === undefined || resStream === null) {
    return;
  }

  resStream.on('data', chunk => {
    let values = chunk;
    let value = values[values.length - 1];

    var dateStr = moment(value.valueTime).format('hh:mm:ss.SSS');
//  item.value = param.displayValue(value.value);
//  item.time = dateStr;
    
    grid.updateItem(rowId, {"value": param.displayValue(value.value), "time": dateStr});
});
}

let isInterval = false;
async function getValue2(param, rowId) {
  let value = await param.lastValue();
  let grid = $$("parametersGrid");
  let item = grid.getItem(rowId);

  item.value = value.value;
  item.time = value.valueTime - Info.startTime;

  grid.updateItem(rowId, item);

  if (param.id == 65 || param.id == 66 || param.id == 67 && !isInterval) {
    setInterval(() => {

      isInterval = true;
      item.value = item.value + 1;
      item.time = value.valueTime - Info.startTime;
//    grid.render(rowId, item)
      grid.updateItem(rowId, {"value": item.value});
    }, 300)
  }
  
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

function mark_items_edit(value, config){
  let res = "no_editable";
  if (config.rw == "W") res =  "editable";
  return res;
};

function getUImainMenu(props) {

  return {
    view: "treetable", // "datatable"
    // width:0,
    // height:0,
    // id: "dataview",
    id: "parametersGrid",
    height: 600,
    /* rules:{
      value:function(obj){ 
        console.log(obj);
        return true; 
      }
   }, */
    columns: [
      { id: "name", header: "Name", width: "280",
      template: "{common.treetable()} #name#"
      },
      { 
        id: "desc", header: "Description", fillspace: true
      },
      { id: "value", header: "Value", width: "70", cssFormat:mark_items_edit, 
          editor:"",
          // template:"<input type='text' value='#value#' style='width:155px;'>"
        },
      { id: "dimension", header: "Dimension", width: "90" },
      { id: "time", header: "Time" },
      {
        id: "chart", header: "Show", width: "60",
        template: "{common.checkbox()}"
      },
      { id: "numchart", header: "Trend", width: "60" },
      // { id:"votes", header:"Votes", template:function(obj){
      //   if (obj.votes > 350000)
      //     return "<span style='color:green;'>"+obj.votes+"</span>";
      //   else
      //     return "<span style='color:red;'>"+obj.votes+"</span>";
      // },
      //  width:100}
    ],
    autoheight:true,
    scrollX:false,
    select:"cell",
    navigation:true,
    editable:true,
		editaction:"custom",
    // checkboxRefresh:true,
    on: {
      onItemClick:function(id){
				console.log("onItemClick");
				console.log(id);
        console.log(Info.model.devices()[Info.states.indexDevice].params);
        let paramCell = Info.model.devices()[Info.states.indexDevice].params[id.row.substr(1,1)-1];
        // console.log(paramCell.rw);
				if (paramCell.rw == "W") {
			  		this.editRow(id);
				}
			},
			onBeforeEditStart:function(id){
				console.log("onBeforeEditStart");
        // console.log(id);
        let paramCell = Info.model.devices()[Info.states.indexDevice].params[id.row.substr(1,1)-1];
        // console.log(paramCell.rw);
				
        if((id.column === "value") && (id.row != "m3")){
				  var currentEd = this.getColumnConfig(id.column).editor;
				  if (paramCell.rw == "W") {
			  		this.getColumnConfig(id.column).editor = "text"; // "inline-text";
			  	}
          
          // if (id.row === 3) {
				  // 	this.getColumnConfig(id.column).editor = "text";	
				  // } else {
					// this.getColumnConfig(id.column).editor = "popup"
				  // }
			  }
        if ((id.column === "value") && (id.row == "m3")) {
          let column = this.getColumnConfig(id.column);
          column.collection = [
            {id:"a", value: "a"},
            {id:"b", value: "b"},
            {id:"c", value: "c"}
          ];
          column.editor = "richselect";
        }
			},
      onAfterClose: function (id) {
        let tree = $$("parametersGrid");
        let rows = getItems(tree, id);
        rows.forEach(function (item, index, array) {
          item.row.param.closeValueStream();
          if (item.row.rw == "W") {
            let value = item.row.param.value.value + 1
            item.row.param.setValue(value)
          }
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
          // let item4 = tree.getItem(item.row.id);
          // let editItem = false;
          // if (item4.rw == 'W') {
          //   console.log("play RW-W");
          //   // tree.updateItem(item.row.id, item4.id);
          //   console.log(item4);
          //   // tree.validateEditor();
          //   editItem = true;
          //   // tree.editCell(item.row.id, item4.id, true, true);
          //   // tree.getColumnConfig("value").editable = false;
          //   // tree.refreshColumns();
          //   tree.edit({
          //     row:item4.id,
          //     column:"value",
          //     editable:true,
          //     // editor:"popup",
          //   });
          //   tree.refresh();
          // }
          // if (editItem) tree.editCell(item.row.id, item4.id, false);
        });
      }
    }
  }
}


function ParametersView(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);
  console.log("!!!!!");
  console.log(props);

  return (
    <div id="dataview">
      <WebixComponent ui={getUImainMenu(props)} data={props.data} />
    </div>
  );
}

export default ParametersView;