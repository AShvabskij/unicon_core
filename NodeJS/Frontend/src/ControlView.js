import Webix from './Webix';
// import Chart from "./Chart2";
import React from "react";
// import * as webix from 'webix/webix.js';
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import { $$ } from 'webix';
import { Info } from './Context';
import moment from 'moment';


function getControl(props) {

  return {
    
  }
}


function ControlView(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);

  return (
    <div id="controlview">
      <WebixComponent ui={getControl(props)} data={props.data} />
    </div>
  );
}

export default ControlView;