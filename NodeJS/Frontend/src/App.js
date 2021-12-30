import React, { useState, useEffect } from "react";
import logo from './logo.svg';
import LocalName from './LocalName';
import ViewBase from './ViewBase';
import ViewTest from './ViewTest';
import {Context} from './Context';
import {Setup} from './Setup';
import {Provider} from "mobx-react";
import CompMobix from "./CompMobix";
import BaseMenuLeft from "./MenuLeft";
import Tabbar from "./Tabbar";
import { observer } from "mobx-react";
import './App.css';
import WebixComponent, {scroll} from './WebixComponent';

window.addEventListener('resize', function(event) {
  Context.resize();   
}, true);


const CaptionObserver = observer(({  }) => {
  // let devArr = [{name:"dev1"},{name:"dev2"},{name:"dev3"}];
  Context.model.label = "";
  useEffect(() => {
    console.log("Render CaptionObserver");
    console.log(Context.states.indexDevice);
    // 
    // updateLeftMenuBase(Context.model.m_devices);
    let deviceItem = Context.model.device(Context.states.indexDevice);
    if (deviceItem ) {
      Context.model.label =  deviceItem.desc + ". Channel: " + deviceItem.interfaceName;
    }
    

  })
  return ( 
    <WebixComponent ui={{ "label": Context.model.label , "width":0, "view": "label", "css":"deviceLabel", "id":"descriptionDevice"}} />
  );
});


function App() {
  return (
    // <MenuLeft/>
    <div className="App">
      <div className="mainrow">
          <div className="left">
            <div className="headerlogo"></div>
            <BaseMenuLeft/>
          </div>
          <div className="main">
          <div className="header"><CaptionObserver/></div>
           <ViewBase/>
           {/* <ViewTest/> */}
          </div>
      </div>
      <div className="footer">footer</div>
      {/* <Tabbar/>
      
      <header className="App-header">
          
           <TimerMobix/>
          
           <Button name="Кнопка 1"/>
           <Button name="Кнопка 2"/>
           <ViewBase/>
       
       
       
      </header> */}
    </div>
  );
}

export default App;
