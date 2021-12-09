import 'webix/webix.css';
import WebixComponent, {scroll} from './WebixComponent';
import { $$, template } from 'webix';
import * as webix from 'webix/webix.js';
import React from "react";
// import Chart from './ChartInteract';
// import Chart2 from './Chart2';
// import { ChartControls } from './Chart2';
// import ParametersView from './ParametersView';
// import ControlView from './ControlView';
import ViewDevicesInfo from './ViewDevicesInfo';
import Config from './.config.js';
import { Model } from "./data_model/fr_model.mjs";
import { observer } from "mobx-react";
import { Context } from "./Context";


function tabview1(props) {
  return {
    view: "tabview",
    // id: "tabview1",
    height: 800,
    css:"tabParam",
    pading: "0",
    cells: [
      {
        id: "parameters",
        header: "Parameters",
        body: {
          id: "parametersContent",
          view: "htmlform",
          content: "dataview",
        }
      },
      {
        id: "oscilloscope",
        header: "Оscilloscope",
        body: {
          id: "oscilloscopeContent",
          view: "htmlform",
          content: "memo1"
        }
      },
      {
        id: "control",
        header: "Control",
        body: {
          id: "controlContent",
          view: "htmlform",
          content: "controlview"
        }
      },
      {
        id: "info",
        header: "Info",
        body: {
          id: "infoContent",
          view: "htmlform",
          content: "infoview"
        }
      }
    ],
    
    
  }
}

function getControl(props) {
  return {
    "cols": [
      {
        "options": [
          "One",
          "Two",
          "Three"
        ],
        "view": "tabbar",
        "height": 0
      }
    ]
  }
}

function getControl1(props) {
  return {
    view: "tabview",
    cells: [
      {
        id: "parameters",
        header: "Parameters",
        body: {
        }
      },
      {
        id: "oscilloscope",
        header: "Оscilloscope",
        body: {
        }
      },
      {
        id: "control",
        header: "Control",
        body: {
        }
      },
      {
        id: "info",
        header: "Info",
        body: {
          id: "infoContent",
          view: "htmlform",
          content: "viewdevicesinfo"
        }
      }
    ],
    tabbar: {
      on: {

        onAfterTabClick: function (id, ev) {
          if (id == "oscilloscopeContent") {
            //showChart("chart2", "memo1");
            // showElementChart("chart3");
            // showElementChart("chart4")
            // showElementChart("chart5")
          }
          if (id == "controlContent") {
            let cv = document.getElementById("controlview");
            cv.style.visibility = "visible";
          }

          // if (id == "controlContent") {
          //   showChart("chart1", "memo2")
          // }
          Context.resize();
        }
      }
    }
  }
}

export default class ViewDevices extends React.Component {
  constructor(props) {
    super(props);
    this.title = "first title"
    this.state = { title: "state title", dt: [] };
    this.updateDevices = props.updateDevices;

  };


  render() {

    let component = this;
    // Info.elements = { ...Info.elements, menuTop: this };

    return (
      <div id="ViewDevices" className="page">
        <WebixComponent ui={getControl1(this.props)} data={[]} />
        
        {/* Context.states.indexDevice */}
        hi
        <ViewDevicesInfo id="viewdevicesinfo" />
             {/* <ViewDevicesInfo id="asd" data={""} /> */}
      </div>
    )
  }
};


// export default MenuCenter;
// export { removeButtonClick };