import React from "react";
import {inject, observer} from "mobx-react";
import ViewGraphicTrends from "./ViewGraphicTrends";
import ViewDevices from "./ViewDevices";

import ViewCPlotWeb from "./ViewCPlotWeb";

function ViewBase(props) {
  return (
   <div>
    <ViewDevices/>
    <ViewGraphicTrends/>
    <ViewCPlotWeb/>
   </div>
 );
}

export default  ViewBase;