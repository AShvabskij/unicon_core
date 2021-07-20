import {JetView, plugins} from "webix-jet";
// import ToolbarView from "views/toolbar";
import {data} from "models/records";
// import baseTableUpdate from "js/avp";
import * as avp from "js/avp";

export default class TopView extends JetView{


	config(){

		var mainDataJsonString = JSON.stringify({
			devices : [
				{device: "dev1", name:"Device 1", interface : {can:true, mbus:false, fo:true} , groupparams : ["m1","acp"]},
				{device: "dev2", name:"Device 2", interface : {can:false, mbus:false, fo:true}, groupparams : ["m1","acp","relay"]},
			],
			groupparams : [
				{id : "m1", goupname : "Module 1 (1800)" , interface : {can:true, mbus:false, fo:true}},
				{id : "m2",goupname : "Module 2 (3100)" , interface : {can:true, mbus:false, fo:true}},
				{id : "acp",goupname : "АЦП (1700)" , interface : {can:true, mbus:false, fo:true}},
				{id : "relay",goupname : "Реле (2100)" , interface : {can:true, mbus:false, fo:true}}
			]
		});

		// ---------------------------------------
		var header = {
			type:"header", template:this.app.config.name, css:"webix_header app_header"
		};
		var header1 = {
			type:"header", template:"Пар устройства", css:"webix_header app_header"
		};
		console.log("TopView config");
		var menu = {
			view:"menu", id:"top:menu", 
			css:"app_menu",
			width:180, layout:"y", select:true,
			template:"<span class='webix_icon #icon#'></span> #value# ",
			// data:[
			// 	{ value:"Device 1", id:"start", icon:"wxi-columns" },
			// 	{ value:"Device 2", id:"start1", icon:"wxi-columns" },
			// 	{ value:"Data",		 id:"data",  icon:"wxi-pencil" }
			// ]
			data: avp.getDeviceFromJSON(mainDataJsonString)
		};
		// console.log(avp.getDeviceFromJSON(""));
		// console.log(menu["data"]);
		var menu_acc = {
			view:"menu", 
			id:"top:menu_acc", 
			css:"app_menu",
			width:180, layout:"y", select:true,
			template:"<span class='webix_icon #icon#'></span> #value# ",
			// data: avp.getDeviceFromJSON(mainDataJsonString),
			on:{
				onMenuItemClick: function(id, e, node){
					// config is {yourProperty: "yourValue"}
					console.log("onMenuItemClick");
					console.log(node);
					// baseTableUpdate();
				}
			  }
		};

		var menu_acc2 = {
			view:"menu", 
			id:"top:menu_acc2", 
			css:"app_menu",
			width:180, layout:"y", select:true,
			template:"<span class='webix_icon #icon#'></span> #value# ",
			data: avp.getDeviceFromJSON(mainDataJsonString),
			on:{
				onMenuItemClick: function(id, e, node){
					// config is {yourProperty: "yourValue"}
					console.log("onMenuItemClick");
					console.log(node);
					// baseTableUpdate();
				}
			  }
		};

		var menuaccordeon ={
			view:"accordion",
			id:"top:accordion", 
			// type:"wide",
			multi : false,
			collapsed:true,
			rows:[
				{ header:"Device 1", body: menu_acc },
				{ header:"Device 2", body: menu_acc2 },
				{ header:"Device 3", body: ""},
				{ header:"Graphic trends", body: ""},
				{ header:"PLC", body: "" }
			]
		}

		var row1 = {
			view:"menu", id:"top:menu1", 
			css:"app_menu",
			width:180, layout:"y", select:true,
			template:"<span class='webix_icon #icon#'></span> #value# ",
			data:[
				{ value:"Схема устройства", id:"start", icon:"wxi-columns" },
				{ value:"График", id:"start1", icon:"wxi-columns" },
				{ value:"Параметры",		 id:"data_m",  icon:"wxi-pencil" }
			]
		};

		var tree = { view:"tree",
		id:"top:tree",
		// select: true,
		editable:true,
		editor:"text",
		editValue:"value",
		autoConfig:true, 
		// template:"{common.icon()} {common.checkbox()} {common.folder()} <span>#value#</span>",
		on:{ onSelectChange: function(){
			//selected = $$("myTree").getSelectedId()
			// alert("item has just been clicked qw");
			console.log("------------ onSelectChange")
			console.log(this)
			}
		}, 
		data: [
			{id:"root", value:"Udc", open:true, data:[
				{ id:"1", open:true, value:"Udc1_meas", data:[
					{ id:"1.1", value:"Udc2_meas" },
					{ id:"1.2", value:"Udc3_meas" },
					{ id:"1.3", value:"Udc4_meas" }
				]},
				{ id:"2", open:true, value:"Udc Gain", data:[
					{ id:"2.1", value:"Udc1 Gain" },
					{ id:"2.2", value:"Udc2 Gain" }
				]}
			]}
		]};

 
		var ui1 = {
			type:"clean", paddingX:5, css:"app_layout", cols:[
				{  paddingX:5, paddingY:10, rows: [ {css:"webix_shadow_medium", rows:[header, menu,header1,tree]} ]},
				{ type:"wide", paddingY:10, paddingX:5, rows:[
					{ $subview:true } 
				]}
			]
		};

		var tabProtocol = {
			"view": "tabbar",
			"id" : "top:protocol",
			"width": "300",
			"options": [
				{ value:"Can", id:"can"},
				{ value:"MBus", id:"mbus" },
				{ value:"FO", id:"fo" }
			],
		
			on:{
				onChange: function(newValue, oldValue, config){
					// config is {yourProperty: "yourValue"}
					console.log(this);
					console.log(newValue);
					avp.updateDevice(newValue);
					// avp.baseTableUpdate();
				}
			  }

		};

		var ui = {
			type:"clean", paddingX:5, css:"app_layout", rows: [ {cols:[
				{rows: [header, tabProtocol ,menuaccordeon]} ,
				{rows: [
					{
						"view": "tabbar",
						"id" : "top:toolbar",
						"options": [
							{ value:"Parameters", id:"data3",  icon:"wxi-pencil" },
							{ value:"Оscilloscope", id:"chart3",  icon:"wxi-pencil" },
							{ value:"Control", id:"device_manage",  icon:"wxi-pencil" },
							{ value:"Info", id:"data_m",  icon:"wxi-pencil" },
						],
						
						on:{
							onChange: function(newValue, oldValue, config){
								// config is {yourProperty: "yourValue"}
								console.log(this);
								console.log(newValue);
								//avp.updateDevice();
							}
						  }

					},
					{ $subview:true },
					{"padding":10, rows: [ {"label": "Monitoring", "view": "label", "height": 38, "margin": 20}]},
			{
				"data": data,
				"columns": [
					{ "id": "title", "header": "Status", "fillspace": true, "sort": "string" },
					{ "id": "text", "header": "Text", "width":"400","sort": "string" },
					{ "id": "state", "header": "State", "width":"250","sort": "check" },
				],
				"view": "datatable",
				id:"tableinfo",
				// table.clearAll()
				// table.load(data_url);
				"height": 150
			}
				]}
				
			]
			},
			
		]
		};

		return ui;
	}
	init(){
		// console.log("init");
		// this.use(plugins.Menu, "top:menu" );
		this.use(plugins.Menu, "top:toolbar" );
		//this.use(plugins.Menu, "top:menu_acc" );
		
		// this.use(plugins.Tree, "top:tree");
	}
}