import Webix from './Webix';
// import Chart from "./Chart2";
import React from "react";
// import * as webix from 'webix/webix.js';
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';

function getUImainMenu(props) {
	return { view:"datatable", 
			// width:0,
			// height:0,
			id: "dataview",
			height:353,
			columns:[
				{id:"num", header:"Number"},
				{id:"name", header:"Name", width:"300"},
				{id:"value", header:"Value", width:"130"},
				{id:"dimension", header:"Dimension"},
				{id:"time", header:"Time"},
				{id:"chart", header:"Show on chart", width:"150"},
				{id:"numchart", header:"Number of chart", width:"200"}
			],
			autoConfig:true, 
      // css:"webix_shadow_medium" 
			};
	// this.ui.$$("tree").parse(data.tree());
}

function getTable(params) {
	return {
    view:"datatable", 
    id:"film_list",
    scroll:"y",
    select:true,
    height:300,
    hover:"myhover",
    columns:[
        { id:"rank", header:"", width:50, css:"rank"},
        { id:"title", header:"Film title", fillspace:true},
        { id:"year",  header:"Released", width:100},
        { id:"votes", header:"Votes", width:100},
        { id:"rating", header:"Rating", width:100}
    ]
}
	
}

function getTable2(params) {
	return {
    "view": "tabbar",
    "options": [
      { value:"Parameters", id:"dataview",  icon:"wxi-pencil" },
      { value:"Оscilloscope ", id:"scichart-root",  icon:"wxi-pencil" },
      { value:"Control", id:"device_manage",  icon:"wxi-pencil" },
      { value:"Info", id:"data_m",  icon:"wxi-pencil" },
    ],
	}
}
// export default class DataView extends JetView{
// 	config(){
// 		return { view:"datatable", 
// 			// width:0,
// 			// height:0,
// 			id: "dataview",
// 			columns:[
// 				{id:"num", header:"Number"},
// 				{id:"name", header:"Name", width:"300"},
// 				{id:"value", header:"Value", width:"130"},
// 				{id:"dimension", header:"Dimension"},
// 				{id:"time", header:"Time"},
// 				{id:"chart", header:"Show on chart", width:"150"},
// 				{id:"numchart", header:"Number of chart", width:"200"}
// 			],
// 			autoConfig:true, css:"webix_shadow_medium" };
// 	}
// 	init(view){
// 		view.parse(device1);
// 	}
// }


// export default class DataView extends React.Component {
//   constructor(props) { 
//     super(props);
//     this.title = "first title"
//     this.state = { title: "state title" };
// 	this.data = [
// 	{ id:9, num: "1", name:"Parameter 1 (2110)", value:"1.008", dimension:"W", time:"11:56",chart:"+",numchart:1},
// 	{ id:10, num: "2", name:"Parameter 2 (2120)", value:"2.7896", dimension:"A", time:"11:56",chart:"+",numchart:1},
// 	{ id:11, num: "3", name:"Parameter 3 (2130)", value:"8", dimension:"kHz", time:"11:00",chart:"+",numchart:"2"},
// 	{ id:7, num: "4", name:"Parameter 4 (2140)", value:"356", dimension:"NO/NC", time:"11:20",chart:"–",numchart:""}
// ];
//   };
//   render() {
	
//     return(
//         <div id="dataview">
//         <Webix  ui={getUImainMenu(this.props)} data={this.data} />
//         </div>
//     )
//   }
// }

function DataView(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);

  return ( 
      // <Webix ui={getUI3(props.devtitle)} data={props.devtitle}/>
      <div id="dataview">
        <WebixComponent ui={getUImainMenu(props)} data={props.data} />
      </div> 
  );
}

export default DataView;