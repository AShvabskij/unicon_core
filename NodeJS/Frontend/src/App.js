import React from "react";
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
// import Chart from './ChartInteract';
// import Chart2 from './Chart2';
// import MenuTest from './MenuTest';
// import MenuInfo from './MenuInfo';
// import MenuDemo from './MenuDemo';

/* const {Model, SysInterfacesEnum}  = require("./data_model/model.js");
let model = new Model();

const deviceId = 12;
const paramId = 65;

async function loaderDataModel() {
  
  let param = model.device(deviceId).param(paramId);
  let resStream = await param.openValueStream();

  resStream.on('data', chunk => {
    let stringifiedRes = chunk.toString();
    console.log(`Received from stream: ${stringifiedRes}`);

    // if (frontWebSocket) {
    //   frontWebSocket.send(stringifiedRes);
    // }
  });
}  */

function App() {
  // React.useEffect(() => {
  //   console.log("on load");
  //   //initSciChart();
  // }, []);
  //loaderDataModel();
  window.chartEvents = [];
 return (
    
    <div className="App">
      <div className="c1">
          <MenuLeft />
      </div>

      <div className="c2">
          <MenuTop />
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
