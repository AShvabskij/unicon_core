import React, { useState } from "react";
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
import './App.css';

window.addEventListener('resize', function(event) {
  Context.resize();   
}, true);

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
          <div className="header"></div>
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
