import React from "react";
import {inject, observer} from "mobx-react";
import View from './View';
import Button from './Button';
import ViewGraphicTrends from "./ViewGraphicTrends";
import ViewDevices from "./ViewDevices";

import ViewCPlotWeb from "./ViewCPlotWeb";

function ViewBase(props) {
  return (
   <div>
    <ViewGraphicTrends/>
    <ViewCPlotWeb/>
    <ViewDevices/>
   </div>
 );
}

export default  ViewBase;