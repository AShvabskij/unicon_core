import 'webix/webix.css';
import WebixComponent, {scroll} from './WebixComponent';
import { $$, template } from 'webix';
import * as webix from 'webix/webix.js';
import React,{useEffect} from "react";

import ViewDevicesInfo from './ViewDevicesInfo';
import ViewDevicesControl from "./ViewDevicesControl";
import ViewDevicesParameters from "./ViewDevicesParameters";
import ViewDevicesOscilloscope from "./ViewDevicesOscilloscope"
import { observer } from "mobx-react";
import { Context, setVisibleElements } from "./Context";

function tabViewControl() {
  let cellsArr = ["devicesParametersTab","devicesОscilloscopeTab","devicesControlTab","devicesInfoTab"];

  return {
    view: "tabview",
    id: "tabViewControl",
    type:{
      height:"auto"
    },
    cells: [
      {
        id: "parameters",
        header: "Parameters",
        body: {
          id: "devicesParametersTab",
          select:true,
        },
        
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
      id: "tabbarParam",
      on: {
        onAfterTabClick: function (id, ev) {
          // console.log("onAfterTabClick="+id);
          // console.log(ev);
          setVisibleElements([id],cellsArr);
          Context.resize();
        }
      }
    }
  }
}

const TabViewObserver = observer(({  }) => {
  useEffect(() => {
    // console.log("Render TabViewObserver");
    // console.log(Context.states.indexDevice);
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
    return (
      <div id="ViewDevices" className="page">
        <TabViewObserver/>
        <ViewDevicesInfo id="devicesInfoTab" />
        <ViewDevicesControl id="devicesControlTab" />
        <ViewDevicesParameters id="devicesParametersTab"/>
        <ViewDevicesOscilloscope id="devicesОscilloscopeTab" />
      </div>
    )
  }
};