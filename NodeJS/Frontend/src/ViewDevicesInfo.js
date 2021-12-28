import React from "react";
import 'webix/webix.css';
import ReactDOM from 'react-dom';
import WebixComponent from './WebixComponent';



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
   
  }


function ViewDevicesInfo(props) {
  console.log("InfoView");
  console.log(props.data);
  let className= "infoPic"+props.data;
  let textArr = [];
  textArr.push("By using thyristors (SCRs) in a phase angle control mode, reduced voltage control can be achieved. Phase control makes it possible to gradually increase the motor terminal voltage from an initial set point up to the system supply voltage level. The related starting current and the starting torque can be optimally adjusted to the motor/load conditions.");
  textArr.push("Control Module- MVCP is the “brain” of the soft starter. It consists of the mBoard that includes: • Main CPU PCB. • HMI board: can be either placed in the Control Module box or at the cabinet door. • Fireboard PCB. • Powersupply. • Input/outputinterfaceterminals. • Optional PCBs (when ordered). </br>The Control Module for HRVS-DN-PowerStart is identical for all ratings and suitable for mounting in the L.V. compartment of the cabinet which should be fully segregated from the M.V. compartment. </br>Interposing relays should be connected to all HRVS-DN-PowerStart auxiliary contacts, three relays must be incorporated: Immediate, End of Acceleration and Fault.");
  textArr.push("Motor will start only if SOFT STOP (terminal 21) and STOP (terminal 22) terminals are connected to Control Input voltage.");
  textArr.push("Control Input voltage (START, SOFT STOP, STOP, terminal inputs 20,21,22) can be the same as Control Supply (terminals 41, 42) or voltage from a different source.");
  textArr.push("Text 5");
  textArr.push("Text 6");
  let text= textArr[props.data];
  return (
    
    <div id={props.id} className="pages">
      <table>  
        <tr>
          <td><div class={className}> </div></td>
          <td>{text}</td>
        </tr>
      </table>
     </div>
  );
}

export default ViewDevicesInfo;