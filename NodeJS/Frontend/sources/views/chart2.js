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
            valChart2 = Math.round(Math.sin(i/10)*50, 2)+55;
            item = { id: i, sales: valChart, sales2: valChart2, year: i};
            dataset.push(item);
            
        }
        
        var multiple_dataset = [
            { sales:"20", sales2:"35", sales3:"55", year:"02" },
            { sales:"40", sales2:"24", sales3:"40", year:"03" },
            { sales:"44", sales2:"20", sales3:"27", year:"04" },
            { sales:"23", sales2:"50", sales3:"43", year:"05" },
            { sales:"21", sales2:"36", sales3:"31", year:"06" },
            { sales:"50", sales2:"40", sales3:"56", year:"07" },
            { sales:"30", sales2:"65", sales3:"75", year:"08" },
            { sales:"90", sales2:"62", sales3:"55", year:"09" },
            { sales:"55", sales2:"40", sales3:"60", year:"10" },
            { sales:"72", sales2:"45", sales3:"54", year:"11" }
        ];
        

		var tree = {
            view:"chart",
            //   width:600,
            //   height:250,
              type:"line",
              
              xAxis:{
                template:"'#year#"
              },
              yAxis:{
                start:0,
                step:10,
                end: 100,
                // template:function(obj){
                //     return (obj%20?"":obj)
                //   }
              },
              legend:{
                values:[{text:"Company A",color:"#1293f8"},{text:"Company B",color:"#66cc00"}],
                align:"right",
                valign:"middle",
                layout:"y",
                width: 100,
                margin: 8
              },
             series:[
                {
                  value:"#sales#",
                  item:{
                    borderColor: "#1293f8",
                    color: "#ffffff",
                    radius:0
                  },
                  line:{
                    color:"#1293f8",
                    width:3
                  },
                  tooltip:{
                    template:"#sales#"
                  }
                },
                {
                  value:"#sales2#",
                  item:{
                    borderColor: "#66cc00",
                    color: "#ffffff",
                    radius:0
              
                  },
                  line:{
                    color:"#66cc00",
                    width:3
                  },
                  tooltip:{
                    template:"#sales2#"
                  }
                }
              ],
              data:  multiple_dataset
            //   data:  dataset
            };
         
		return tree;

	}
	init(view){
		
	}	

}