import React, { useState } from "react";
import { Grid, Row, Col } from 'react-flexbox-grid';
import logo from "./logo.svg";
import "./App.css";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';
// import { HashRouter as Router, Route, NavLink} from 'react-router-dom';
import { Link, BrowserRouter as Router, Route } from 'react-router-dom';
import {useHistory} from 'react-router'
import DevicesView from './DevicesView';
import MenuLeft from './MenuLeft';
import { accordionInit } from './data/config.js';
import {AddCounter,Info} from "./Context"

const DevicesPage = () => {
    return (
        <div className="c2">
        {/* <DevicesView updateDevices={Info.actions.setDevicesName}/> */}
        <DevicesView/>
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
    let v= $$("DeviceInit");
    v.define("header","new header");
    // v.expand();
  }

  const updateLeftMenu = () => {
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

  // const [name, setName] = useState("");
  const [devicesD, setDevicesName] = useState(accordionInit);
  Info.actions = {...Info.actions,setDevicesName:setDevicesName};
  

 return (
  <div className="App">
      <Router>
      <div className="c1">
          <div><span class='webix_icon mdi mdi-file-video'></span>UNICON</div>
       
          <MenuLeft devtitle={devicesD}/>
      </div>
        <DevicesPage/>
        {/* <Route exact path="/" component={DevicesPage} />   
        <Route exact path="/CPLotWebPage" component={CPLotWebPage} />  */}
      </Router>
      
    </div>
  );
}

export default App;
