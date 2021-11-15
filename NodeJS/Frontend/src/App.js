import React, { useState } from "react";
import logo from './logo.svg';
import View from './View';
import Button from './Button';
import LocalName from './LocalName';
import ViewBase from './ViewBase';
import TimerMobix from './TimerMobix'
import {Context} from './Context';
import {Provider} from "mobx-react";
import CompMobix from "./CompMobix";
import MenuLeft from "./MenuLeft";
import Tabbar from "./Tabbar";
import Chart from './Chart';
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
          <MenuLeft/>
          </div>
          <div className="main">
          <TimerMobix/>
          <Chart id="chart4" title="&nbsp;"  />
           <Button name="Кнопка 1"/>
           <Button name="Кнопка 2"/>
           <ViewBase/>
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
