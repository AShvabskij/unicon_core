import {JetView} from "webix-jet";
import {device2} from "models/records";

export default class DataView extends JetView{
	config(){
		//return { view: "switch", value: 1, label:"Light" };
        return { 
			height: 300,
			cols: [
				{
					"padding":30,
						rows: [
							{ "label": "Switch ON/OFF", "view": "switch", "height": 0, "labelWidth":"150"},
							{ "label": "Switch Monitoring", "view": "switch", "height": 0, "labelWidth":"150"},
							{ "label": "Show Indexes", "view": "switch", "height": 0, "labelWidth":"150"}
						]
				},
				{	width: 400,
					padding:100,
					rows: [
						{view: "text", value: "123", label: "Parameter 1", labelWidth:100},
						{view: "text", value: "123", label: "Parameter 2", labelWidth:100},
						{view: "text", value: "123", label: "Parameter 3", labelWidth:100},
						{view: "text", value: "123", label: "Parameter 4", labelWidth:100}
					]
				}
			]
		};
    }
	
    init(view){
		//view.parse(device2);
	}
}