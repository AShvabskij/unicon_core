import {JetView} from "webix-jet";
import {device2} from "models/records";

export default class DataView extends JetView{
	config(){
		return { view:"datatable", autoConfig:true, css:"webix_shadow_medium" };
	}
	init(view){
		view.parse(device2);
	}
}