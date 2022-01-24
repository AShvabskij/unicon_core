import React, { useState, useEffect } from "react";
import logo from './logo.svg';
import { observer } from "mobx-react";
import LocalName from './LocalName';
import ViewBase from './ViewBase';
import ViewTest from './ViewTest';
import {Context} from './Context';
import {Setup} from './Setup';
import {Provider} from "mobx-react";
import BaseMenuLeft from "./MenuLeft";
import Tabbar from "./Tabbar";
import { makeAutoObservable } from "mobx"

import WebixComponent from './WebixComponent';
import Footer from './Footer';
import './App.css';

window.addEventListener('resize', function(event) {
  Context.resize();   
}, true);


makeAutoObservable(Context);

const CaptionObserver = observer(({  }) => {
  Context.model.label = "";
  useEffect(() => {
    // console.log("Render CaptionObserver");
    // console.log(Context.states.indexDevice);
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
      
      <Footer/>
      
    </div>
  );
}

export default App;
