import React, { useState } from "react";
import { Grid, Row, Col } from 'react-flexbox-grid';
import logo from "./logo.svg";
import "./App.css";
import { HashRouter as Router, Route, NavLink} from 'react-router-dom';
import Home from './Home';
// import FilmsView from './FilmsView';
// import Chart from './Chart';
import MenuTop from './MenuTop';
import MenuLeft from './MenuLeft';
import DataView from './DataView';
import { accordionInit } from './data/config.js';
// import Chart from './ChartInteract';
// import Chart2 from './Chart2';


function App() {
  // React.useEffect(() => {
  //   console.log("on load");
  //   //initSciChart();
  // }, []);
  //loaderDataModel();
  window.chartEvents = [];

  // const [name, setName] = useState("");
  const [devicesD, setDevicesName] = useState(accordionInit);

 return (
  <div className="App">
      <div className="c1">
          <div><span class='webix_icon mdi mdi-file-video'></span>UNICON</div>
          <MenuLeft devtitle={devicesD}/>
      </div>

      <div className="c2">
          <MenuTop updateDevices={setDevicesName}/>
      </div>
      {/* <div className="c3">

          123
      </div>
       */}
      
     {/*  <Row >
              <Col className = "c1" xs={2}  > 
                  <MenuLeft />
              </Col>
              <Col className = "c2" xs={10}   > 
              <MenuTop />
              </Col>
              
        </Row>
 */}
      
    </div>
  );
}

export default App;
