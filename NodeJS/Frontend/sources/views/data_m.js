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
        
		// return { view:"datatable",  css:"webix_shadow_medium", editable:true ,columns: [{ id:"title", header:"Title", editor: "text"  ,fillspace:true},{ id:"id", header:"id" },{ id:"year", header:"Year 1" }]};
        return {
            view:"calendar", 
    		width:0, 
    		height:0
          };
    }
	init(view){
		
	}
}