import 'webix/webix.css';
import WebixComponent, {scroll} from './WebixComponent';
import { $$, template } from 'webix';
import * as webix from 'webix/webix.js';
import React,{useEffect} from "react";
// import Chart from './ChartInteract';
// import Chart2 from './Chart2';
// import { ChartControls } from './Chart2';
// import ParametersView from './ParametersView';
// import ControlView from './ControlView';
import ViewDevicesInfo from './ViewDevicesInfo';
import ViewDevicesControl from "./ViewDevicesControl";
import ViewDevicesParameters from "./ViewDevicesParameters";
import ViewDevicesOscilloscope from "./ViewDevicesOscilloscope"
import Config from './.config.js';
import { Model } from "./data_model/fr_model.mjs";
import { observer } from "mobx-react";
import { Context, setVisibleElements } from "./Context";


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

function tabViewControl() {
  let cellsArr = ["devicesParametersTab","devicesОscilloscopeTab","devicesControlTab","devicesInfoTab"];
  console.log("tabViewControl");
  // console.log(props.updateDevices);
  return {
    view: "tabview",
    id: "tabViewControl",
    cells: [
      {
        // id: "parameters",
        header: "Parameters",
        body: {
          id: "devicesParametersTab",
          select:true,
        }
      },
      {
        // id: "oscilloscope",
        header: "Оscilloscope",
        body: {
          id: "devicesОscilloscopeTab",
        }
      },
      {
        // id: "control",
        header: "Control",
        body: {
          id: "devicesControlTab",
        }
      },
      {
        // id: "info",
        header: "Info",
        body: {
          id: "devicesInfoTab",
        }
      }
    ],
    tabbar: {
      on: {

        onAfterTabClick: function (id, ev) {
          console.log("onAfterTabClick="+id);
          console.log(ev);
          setVisibleElements([id],cellsArr);
          if (id == "devicesInfoTab") {
            //setVisibleElements([id],cellsArr);
          }
          if (id == "oscilloscopeContent") {
            //showChart("chart2", "memo1");
            // showElementChart("chart3");
            // showElementChart("chart4")
            // showElementChart("chart5")
          }
          if (id == "controlContent") {
            let cv = document.getElementById("controlview");
           // cv.style.visibility = "visible";
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

const TabViewObserver = observer(({  }) => {
  // let devArr = [{name:"dev1"},{name:"dev2"},{name:"dev3"}];
  useEffect(() => {
    console.log("Render TabViewObserver");
    console.log(Context.states.indexDevice);
  })
  return ( 
      <WebixComponent ui={tabViewControl()} data={[]} />
  );
});


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
        <TabViewObserver/>
        {/* Context.states.indexDevice */}
        <ViewDevicesInfo id="devicesInfoTab" data={Context.states.indexDevice} />
        <ViewDevicesControl id="devicesControlTab" />
        <ViewDevicesParameters id="devicesParametersTab"/>
        <ViewDevicesOscilloscope id="devicesОscilloscopeTab" />
             {/* <ViewDevicesInfo id="asd" data={""} /> */}
      </div>
    )
  }
};


// export default MenuCenter;
// export { removeButtonClick };