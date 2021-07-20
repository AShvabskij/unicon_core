import {JetView} from "webix-jet";
import {device1} from "models/records";

export default class DataView extends JetView{
	config(){
		return { view:"datatable", 
			// width:0,
			// height:0,
			columns:[
				{id:"num", header:"Number"},
				{id:"name", header:"Name", width:"300"},
				{id:"value", header:"Value", width:"130"},
				{id:"dimension", header:"Dimension"},
				{id:"time", header:"Time"},
				{id:"chart", header:"Show on chart", width:"150"},
				{id:"numchart", header:"Number of chart", width:"200"}
			],
			autoConfig:true, css:"webix_shadow_medium" };
	}
	init(view){
		view.parse(device1);
	}
}