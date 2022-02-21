import React from "react";
// import * as webix from 'webix/webix.js';
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';
import { $$ } from 'webix';



function getControl(props) {
  let data = [
    { "id": "1", "value": "One" },
    { "id": "2", "value": "Two" },
    { "id": "3", "value": "Three" }
  ];
  return {
    "height": 850,
    "cols": [
      { "label": "", "view": "label", "width": 128},
      {
        "width": 350,
        // "height": 950,
        "rows": [
          { "label": "DCDC  ch3", "view": "label" },
          { "value": "1", "options": data, "view": "combo" },
          { "label": "", "view": "label", "height": 18},
          {
            "cols": [
              { "label": "Udc", "view": "label", "height": 20, "align": "center" },
              { "label": "Udc", "view": "label", "height": 20, "align": "center" },
            ]
          },
          {
            "cols": [
              { "label": "800V", "view": "label", "height": 20, "valign": "top" },
              { "label": "120A", "view": "label", "height": 20, "valign": "top" },
            ]
          },
          { "label": "", "view": "label", "height": 95},
          { "label": "Power 60kW", "view": "label", "height": 18, "align": "center" },
          { "title": "#value#", "value": 50, "view": "slider" },
          { "label": "", "view": "label", "height": 28},
          {
            "cols": [
              { "label": "POWER ON", "view": "button", "height": 38 },
              { "label": "POWER OFF", "view": "button", "height": 38 }
            ]
          },
          { "label": "TRIP RESET", "view": "button","height": 38  }
          
        ]
      },
      { "label": "", "view": "label", "width": 128},
      {
        "width": 350,
        
        "rows": [
          { "label": "ACDC ch1", "view": "label" },
          { "value": "1", "options": data, "view": "combo" },
          { "label": "", "view": "label", "height": 18},
          {
            "cols": [
              { "label": "Uab", "view": "label", "height": 20, "align": "center" },
              { "label": "Uab", "view": "label", "height": 20, "align": "center" },
              { "label": "Uab", "view": "label", "height": 20, "align": "center" },
            ]
          },
          {
            "cols": [
              { "label": "400V", "view": "label", "height": 20, "valign": "top" },
              { "label": "400V", "view": "label", "height": 20, "valign": "top" },
              { "label": "400V", "view": "label", "height": 20, "valign": "top" },
            ]
          },
          { "label": "", "view": "label", "height": 18},
          {
            "cols": [
              { "label": "Ia", "view": "label", "height": 20, "valign": "top" },
              { "label": "Ia", "view": "label", "height": 20, "valign": "top" },
              { "label": "Ia", "view": "label", "height": 20, "valign": "top" },
            ]
          },
          {
            "cols": [
              { "label": "61 A", "view": "label", "height": 20, "valign": "top" },
              { "label": "61 A", "view": "label", "height": 20, "valign": "top" },
              { "label": "61 A", "view": "label", "height": 20, "valign": "top" },
            ]
          },
          {"label": "", "view": "label", "height": 18},

          { "label": "Power 60kW", "view": "label", "height": 38, "align": "center" },
          { "title": "#value#", "value": 50, "view": "slider" },
          { "label": "", "view": "label", "height": 28},
          {
            "cols": [
              { "label": "POWER ON", "view": "button", "height": 38 },
              { "label": "POWER OFF", "view": "button", "height": 38 }
            ]
          },
          { "label": "TRIP RESET", "view": "button", "height": 38  }
          
        ]
      },
      { "label": "", "view": "label", "width": 128}
    ]
  }
}


function ViewDevicesControl(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);

  return (
    <div id={props.id} className="pages" >
      <WebixComponent  ui={getControl(props)} />
    </div>
    
  );
}

export default ViewDevicesControl;