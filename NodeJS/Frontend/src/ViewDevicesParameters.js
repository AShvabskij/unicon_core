import 'webix/webix.css';
import ReactDOM from 'react-dom';
import React,{useEffect} from "react";
import WebixComponent from './WebixComponent';
import { $$ } from 'webix';
import { Context } from './Context';
import moment from 'moment';
import {ValueFormatEnum} from './data_model/param.mjs';
import { observer } from "mobx-react";

async function getValue(param, rowId) {
  let value = await param.currentValue();
  updateRowValue(value, param, rowId);

  console.log("try to get value streams...");
  let frequency = 2; // кол-во значений в секунду
  let resStream = await param.openValueStream(frequency);
  if (resStream === undefined || resStream === null) {
    return;
  }

  resStream.on('data', chunk => {
    let values = chunk;
    let value = values[values.length - 1];
    updateRowValue(value, param, rowId);
});
}

let isInterval = false;
let currentCellParam = null;

async function getValue2(param, rowId) {
  let value = await param.lastValue();
  updateRowValue(value, param, rowId);

  if (param.id == 65 || param.id == 66 || param.id == 67 && !isInterval) {
    setInterval(() => {

      isInterval = true;
      value.value += 1;
      updateRowValue(value, param, rowId);
    }, 300)
  }
}

function updateRowValue(value, param, rowId) {
  var dateStr = moment(value.valueTime).format('hh:mm:ss.SSS');
  let grid = $$("parametersGrid");

//setTimeout(() => {
//  grid.updateItem(rowId, {"value": param.displayValue(value.value), "time": dateStr});
//}, 1000);
  
  grid.updateItem(rowId, {"value": param.displayValue(value.value), "time": dateStr});

//  let item = grid.getItem(rowId);
//  item.value = item.value + 1;
//  item.time = value.valueTime - Info.startTime;

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
    height: 400,
    /* rules:{
      value:function(obj){ 
        console.log(obj);
        return true; 
      }
   }, */
    columns: [
      { id: "name", header: "Name", width: "300",
      template: "{common.treetable()} #name#",
      header:[
        "Name",
        { content:"textFilter" }
      ]      
      },
      { id: "value", header: "Value", width: "170", cssFormat:mark_items_edit, 
          editor:"",
          // template:"<input type='text' value='#value#' style='width:155px;'>"
        },
      { id: "dimension", header: "Dimension", width: "80" },
      { id: "time", header: "Time" },
      {
        id: "chart", header: "Show", width: "60",
        template: "{common.checkbox()}"
      },
//    { id: "numchart", header: "Trend", width: "40" },
      { 
        id: "desc", header: "Description", fillspace: true
      },

      // { id:"votes", header:"Votes", template:function(obj){
      //   if (obj.votes > 350000)
      //     return "<span style='color:green;'>"+obj.votes+"</span>";
      //   else
      //     return "<span style='color:red;'>"+obj.votes+"</span>";
      // },
      //  width:100}
    ],
    // autoheight:true,
    // scrollX:false,
    // scroll:"auto",
    select:"cell",
    navigation:true,
    editable:true,
		editaction:"custom",
    // checkboxRefresh:true,
    on: {
      onItemClick:function(id){
				console.log("onItemClick");
				// console.log(id);
        console.log(Context.model.device(Context.states.indexDevice).params);
        let paramCell = Context.model.device(Context.states.indexDevice).params[id.row.substr(1)-1];
        console.log(paramCell);
        // console.log(id);
        if (paramCell != undefined) {
            if (paramCell["rw"] == "W") {
                this.editRow(id);
            }
        }
			},
			onBeforeEditStart:function(id){
				console.log("onBeforeEditStart");
        // console.log(id);
        let cellParam = Context.model.device(Context.states.indexDevice).params[id.row.substr(1)-1];
        console.log(cellParam.rw);
				
        if((id.column === "value") && (cellParam.rw == "W")){
				  // let currentEd = this.getColumnConfig(id.column).editor;
				  if (cellParam.valueFormat == ValueFormatEnum.Text) {
			  		let column = this.getColumnConfig(id.column);
              column.collection = [];
              let val;
              for(let key in cellParam.valueTexts) {
                val = cellParam.valueTexts[key];
                column.collection.push({id:val, value: val});
              }
            column.editor = "richselect";
			  	}
          else this.getColumnConfig(id.column).editor = "text"; // "inline-text";
			  }
			},
      onAfterEditStart:function(id){
        currentCellParam = Context.model.device(Context.states.indexDevice).params[id.row.substr(1)-1];
      },
      onAfterEditStop: function (state, editor, ignoreUpdate) {
      if (state.value === state.old) {
        return;
      }
      
      let value = null;
      switch(currentCellParam.valueFormat) {
        case ValueFormatEnum.Text: {
          for(let key in currentCellParam.valueTexts) {
            if (currentCellParam.valueTexts[key] === state.value) {
              value = Number(key);
              break;
            }
          }
        } break;
        default: value = Number(state.value);
      }

      currentCellParam.setValue(value);
    },
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

const updateParameters = (indexDevice) => {
  ClearTableParams();
  console.log("updateParameters indexDevice = " + indexDevice);
  let deviceItem = Context.model.device(indexDevice);
  console.log("updateParameters");
  console.log(deviceItem);
  console.log(Context.model.devices());
  let dt1 = [];
  if (deviceItem) {
    // При переключении девайсов сбрасывать графики
    if (Context.states.indexDevice != indexDevice) {
          for (let i = 0; i < 3; i++) {
              // removeButtonClick();
          }
      }

      
      let dtt = [];
      let i = 1;
      let j = 1;
      let infoCurrentDivice = deviceItem.desc + ". Channel: " + deviceItem.interfaceName;
      Context.states.indexDevice = indexDevice;
      let desc = $$("descriptionDevice");
      // desc.setValue(infoCurrentDivice);
      deviceItem.modules.forEach(function(item, index, array) {
          dtt = [];
          item.params.forEach(function(itemP, indexP, array) {
              infoCurrentDivice = infoCurrentDivice + " [" + Number(itemP.deviceId).toString(16) + "]" + "</br>Chanal: " + item.interfaceName;
              dtt.push({id: "m"+i, // name: itemP.moduleId,  //"[" + itemP.deviceId + "] " + item.name + " [" + itemP.moduleId + "]", 
              name:itemP.name + " ["+ Number(itemP.moduleId).toString(16) + "." + Number(itemP.id).toString(16) + "]", 
              value:" ", dimension:itemP.valueUnit, time:" ", 
              desc:itemP.desc,
              chart:0, 
              numchart:"",
              param:itemP,
              rw: itemP.rw,
          })
              i++;
          });
          dt1.push({"id":"modul"+j, "value":"", "name": item.name + " [" + Number(item.id).toString(16) + "]",
          "open":false, "rw": "R", "data":dtt 
          });
          j++;
      });
      
      // Context.elements.menuTop.setState((state, props) => ({
      //         // dt: [{"id":"can","programmInt":"Can", "open":"false", "data":dt1}]
      //         dt: {"id":"can","data":dt1}
      //  }));

   };
   console.log(dt1);
   return dt1;
}

function ParametersView(props) {
  return ( 
    <div id={props.id} className="pages" >
      <ParametersViewObserver/>
    </div>
  );
}

const ClearTableParams = () =>{
  let tree = $$("parametersGrid");
  if (tree) {
      let arr = tree.getOpenItems();
        arr.forEach(function (item) {
                tree.close(item);
        });
      tree.clearAll();
  }

}

const ParametersViewObserver = observer(({  }) => {
  // let devArr = [{name:"dev1"},{name:"dev2"},{name:"dev3"}];
  useEffect(() => {
    console.log("Render ParametersView");
    console.log(Context.model.devices());
    console.log(Context.states.indexDevice);
    // 
    // updateLeftMenuBase(Context.model.m_devices);
  })
  return ( 
      <WebixComponent ui={getUImainMenu()} data={updateParameters(Context.states.indexDevice)} updateModel={ Context.states.indexDevice } />
  );
});

// function ParametersView(props) {
//   // console.log("MenuLeft ");
//   // console.log(props.devtitle);
//   console.log("!!!!!");
//   console.log(props);

//   return (
//     <div id={props.id} className="pages" >
//       <WebixComponent ui={getUImainMenu(props)} data={props.data} />
//     </div>
//   );
// }

export default ParametersView;