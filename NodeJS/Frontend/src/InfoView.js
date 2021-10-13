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


function getInfo(props) {
  return {
    "cols": [
      {
        "rows": [
          { "label": "Label", "view": "label" },
          {
            "height": 0,
            "cols": [
              { "view": "template", "template": "You can place any widget here..", "role": "placeholder" },
              { "label": "Label", "view": "label", "height": 0 }
            ]
          }
        ]
      }
    ]
  };
   
  
  // return {
  //     view: "template",
  //     // "id": 1633671692548,
  //     "rows": [
  //       { "label": "Label", "view": "label", "height": 99 },
  //       {
  //         "height": 0,
  //         "cols": [
  //           { "icon": "wxi-user", "view": "icon", "width": 448, "height": 0 },
  //           { "label": "Label", "view": "label", "height": 0, "width": 0 }
  //         ]
  //       }
  //     ]
  // }
}


function InfoView(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);

  return (
    <div id="infoview">
      <table>  
        <tr>
          <td><div class="infoPics"> </div></td>
          <td>By using thyristors (SCRs) in a phase angle control mode, reduced voltage control can be achieved. Phase control makes it possible to gradually increase the motor terminal voltage from an initial set point up to the system supply voltage level. The related starting current and the starting torque can be optimally adjusted to the motor/load conditions.</td>
        </tr>
      </table>
      
      {/* <Webix ui={{ "label": "Label123456", "view": "label" }} /> */}
    </div>
  );
}

export default InfoView;