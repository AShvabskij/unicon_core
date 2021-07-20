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

		var oceanData = [
			{id: "1", title: "01. The Charm Offensive www", duration:"7:19"},
			{id: "2", title: "02. Heaven Alive", duration:"6:20"},
			{id: "3", title: "03. A Homage to Shame", duration:"5:52"},
			{id: "4", title: "04. Meredith", duration:"5:26"},
			{id: "5", title: "05. Music for a Nurse", duration:"8:16"},
			{id: "6", title: "06. New Pin", duration:"5:11"},
			{id: "7", title: "07. No Tomorrow", duration:"7:10"},
			{id: "8", title: "08. Mine Host", duration:"4:10"},
			{id: "9", title: "09. You Can’t Keep a Bad Man Down", duration:"7:36"},
			{id: "10", title: "10. Ornament. The Last Wrongs", duration:"9:21"}
		];

		var tree = { view:"tree",
		select: oceanData,
		editable:true,
		template:"{common.icon()} {common.checkbox()} {common.folder()} <span>#value#</span>",
			on:{ onAfterEditStop: function(state, editor, ignoreUpdate){
				console.log("onAfterEditStop");
				if(state.value != state.old){
					webix.message("Cell value was changed")
				}  
			},
	
	
		},
		data: [
			{id:"root", value:"Cars", open:true, data:[
				{ id:"1", open:true, value:"Toyota", data:[
					{ id:"1.1", value:"Avalon" },
					{ id:"1.2", value:"Corolla" },
					{ id:"1.3", value:"Camry" }
				]},
				{ id:"2", open:true, value:"Skoda", data:[
					{ id:"2.1", value:"Octavia" },
					{ id:"2.2", value:"Superb" }
				]}
			]}
		]};
		return tree;

	}
	init(view){
		
	}	

}