import React, { useState } from "react";
import { Grid, Row, Col } from 'react-flexbox-grid';
import logo from "./logo.svg";
import "./App.css";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';
// import { HashRouter as Router, Route, NavLink} from 'react-router-dom';
import { Link, BrowserRouter as Router, Route } from 'react-router-dom';
import {useHistory} from 'react-router'
import Home from './Home';
// import FilmsView from './FilmsView';
// import Chart from './Chart';
import MenuTop from './MenuTop';
import MenuLeft from './MenuLeft';
import DataView from './DataView';
import { accordionInit } from './data/config.js';
import {AddCounter,Info} from "./Context"
// import Chart from './ChartInteract';
// import Chart2 from './Chart2';

const DevicesPage = () => {
    return (
        <div className="c2">
        {/* <MenuTop updateDevices={Info.actions.setDevicesName}/> */}
        <MenuTop/>
    </div>
    );
  }; 

const CPLotWebPage = () => {
    return (
        <div className="c2">
            CPLotWebPage
         </div>
    );
  }; 


  const Button = () => {
    const hist = useHistory();
    return (
        <button onClick={() => updateLeftMenu1()}>Show CPLotWebPage</button>
    );
  }; 

  const updateLeftMenu1= () => {
    let grid = $$("parametersGrid");
    let item = grid.getItem("m3");
    item.value = "qwe";
    // grid.refresh();
    grid.updateItem("m3",item);


    // v.attachEvent("onAfterExpand", function(id){
    //     console.log("Expand for section "+id);
    //   })
    // v.expand();
    // v.setValue("123");
    // let d = [
	// 	{ header:"Graphic trends 1", body: ""},
    //     { header:"PLC ", body: "" },
    //     { header:"CPLotWeb 1", body: ""},
    //     { header:"Devices 1", id:"DeviceInit" ,body: "" }
    // ];
    // Info.actions.setDevicesName(d)
    // console.log(v)
    let v= $$("DeviceInit");
    v.define("header","new header");
    // v.expand();

  }

  const updateLeftMenu = () => {
    // let v= $$("DeviceInit");
    // v.attachEvent("onAfterExpand", function(id){
    //     console.log("Expand for section "+id);
    //   })
    // v.expand();
    // v.setValue("123");
    // let d = [
	// 	{ header:"Graphic trends 1", body: ""},
    //     { header:"PLC ", body: "" },
    //     { header:"CPLotWeb 1", body: ""},
    //     { header:"Devices 1", id:"DeviceInit" ,body: "" }
    // ];
    // Info.actions.setDevicesName(d)
    // console.log(v)
    let v= $$("DeviceInit");
    console.log(v)
    // v.define("header","new header");
    // v.body = "new header 1";
    // v.define("value","new header 1");
    // v.define({body:{view: "button", value: "Add Chart 2",  align: "left"}});
    // v.refresh();
    webix.ui({
        view: "button", value: "Add Chart 2",  align: "left"}
      , $$("DeviceInit"), 0);
    let comp = {view:"segmented", height:100, multiview:true, value:1, options:[
        { id:"1", value:"Section A <br>sdf sdff 12 " }, // the initially selected segment
        { id:"2", value:"Section B <br>fsdf f sdf f s" }, 
        { id:"3", value:"Section C <br>fsdf fsdf  fsdf" }
    
        ],
        click:function(id,event){
            // console.log(id)
        },
        on:{
            onChange: function(newValue, oldValue, config){
                console.log(newValue)
            },
            onItemClick:function(id,ev){
                console.log(id)
                console.log(ev)
            }
          }
    }
      webix.ui({ view: "toolbar",
            cols: [ {view: "button", value: "Add Chart 2",  align: "left",animate:{ type:"flip", subtype:"vertical" }},
            {view: "button", value: "Add Chart 3",  align: "left",}
    ],animate:{ type:"flip", subtype:"vertical" }},$$("DeviceInit"), 0);

    webix.ui(comp,$$("DeviceInit"), 0);

    v.refresh();
    // v.setValue("123");
  }


  const Button1 = () => {
    // const hist = useHistory();
    return (
        <button onClick={() => updateLeftMenu()}>new header</button>
    );
  }; 


function App() {
  // React.useEffect(() => {
  //   console.log("on load");
  //   //initSciChart();
  // }, []);
  //loaderDataModel();
  window.chartEvents = [];

  // const [name, setName] = useState("");
  const [devicesD, setDevicesName] = useState(accordionInit);
  Info.actions = {...Info.actions,setDevicesName:setDevicesName};
  

 return (
  <div className="App">
      <Router>
      <div className="c1">
          <div><span class='webix_icon mdi mdi-file-video'></span>UNICON</div>
        <Button/>
        <Button1/>
          <MenuLeft devtitle={devicesD}/>
      </div>
        <Route exact path="/" component={DevicesPage} />   
        <Route exact path="/CPLotWebPage" component={CPLotWebPage} /> 
      </Router>
      
    </div>
  );
}

export default App;
