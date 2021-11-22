import React, { useState } from "react";
import {Context} from './Context';

function updateTitle(name) {
  // Context.title = name+"-qwe12";
  console.log("ButtonChart show "+name);
  // Context.showChart(devicename);
}

function ButtonChart(props) {
     return (
      <div>
        <button onClick={() => updateTitle(props.name)}>
          Показать чарт {props.name}
        </button>
      </div>
    );
  }
  
  export default ButtonChart;