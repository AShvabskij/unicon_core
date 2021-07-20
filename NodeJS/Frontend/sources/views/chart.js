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

		var  dataset = [
            // { id:1, sales:20, year:"02"},
            // { id:2, sales:55, year:"03"},
            // { id:3, sales:40, year:"04"},
            // { id:4, sales:78, year:"05"},
            // { id:5, sales:61, year:"06"},
            // { id:6, sales:35, year:"07"},
            // { id:7, sales:80, year:"08"},
            // { id:8, sales:50, year:"09"},
            // { id:9, sales:65, year:"10"},
            // { id:10, sales:59, year:"11"}
        ];
        let item = {};
        let valChart =  0;
        for (let i = 0; i < 300; i++) {
            valChart = Math.round(Math.sin(i/10)*50, 2)+50;
            item = { id: i, sales: valChart, year: i};
            dataset.push(item);
            
        }

		var tree = {
            view:"chart",
            // width:600,
            // height:250,
            type:"line",
            value:"#sales#",
            
            legend:{
              values:[{text:"Parameter A",color:"#1293f8"},{text:"Parameter B",color:"#66cc00"}],
              align:"right",
              valign:"middle",
              layout:"y",
              width: 100,
              margin: 8
            },
            item:{
              borderColor: "#1293f8",
              color: "#ffffff",
              radius:0
            },
            line:{
              color:"#1293f8",
              width:3
            },
            xAxis:{
              template:"'#year#"
            },
            offset:0,
            yAxis:{
              start:0,
              end:100,
              step:10,
              template:function(obj){
                return (obj%20?"":obj)
              }
            },
            data: dataset
          };
		return tree;

	}
	init(view){
		
	}	

}