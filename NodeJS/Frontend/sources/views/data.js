import {JetView} from "webix-jet";
import {data} from "models/records";

export default class DataView extends JetView{
	

	config(){
		// return { view:"datatable", autoConfig:true, autowidth:true,css:"webix_shadow_medium", editable:true ,columns: [
		// 	{ id:"title", header:"Title",editor:"text" ,width:"100"},{ id:"id", header:"id" },
		// 	{ id:"year", header:"Year" }
		//   ]};
		// return { view:"datatable", autoConfig:true, css:"webix_shadow_medium", editable:true };

		// this.editors = {
		// 	"myeditor": {
		// 		focus: function () {/* ... */},
		// 		getValue: function () {/* ... */},
		// 		setValue: function (val) {/* ... */},
		// 		render: function () {/* ... */}
		// 	}
		// };

		function custom_checkbox(obj, common, value){
			console.log("custom_checkbox");
			// console.log(obj.old.value);
			// webix.message("Cell value was changed")
			if (value)
			  return "<div class='webix_table_checkbox checked'> YES </div>";
			else
			  return "<div class='webix_table_checkbox notchecked'> NO </div>";
		  };

		return { view:"datatable",  css:"webix_shadow_medium", editable:true ,columns: [{ id:"title", header:"Title", editor: "text"  ,fillspace:true},
		{ id:"id", header:"id" },{ id:"year", header:"Year 1905" }, {id:"www", header:"URL"}, { id: "button", template: "{common.checkbox()}" },
		{ id: "action", template: "custom_checkbox" }],
		editable:true,
		checkboxRefresh:true,
		type:{
			toggle:function(obj, common, value, column, index){
				console.log("toggle");
			  var html = "<div class='webix_secondary'>";
			  html += "<div class='webix_button webix_table_checkbox custom_toggle " + (value ? "shadow" : "") + "'>";
			  html += (value ? "Enabled"+value : "Disabled"+index);
			  html += "</div></div>";
			  return html;
			}
		  },
		//   on:{ onAfterEditStop: function(state, editor, ignoreUpdate){
		// 	console.log("onAfterEditStop");
		// 		if(state.value != state.old){
		// 			webix.message("Cell value was changed")
		// 		}
		// 	}  
		// },
		on:{
			"onItemClick":function(id, e, trg){
			  //id.column - column id
			  //id.row - row id
			  webix.message("Click on row: " + id.row+", column: " + id.column);
			//   id.column.value = "123";
			//innerHtml
			// trg.innerHtml = "123"
			console.log("data")
			  console.log(data.data.pull[1])
			  data.data.pull[1].title = "123"
			}
		  }
	}
		
	}
	init(view){
		webix.protoUI({ name:"datatable"}, webix.ui.datatable, webix.ActiveContent );
		view.parse(data);
	}
}