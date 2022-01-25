import React, { useState } from "react";
import {Context} from './Context';
import WebixComponent from './WebixComponent';

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

function Tabbar(props) {
  return (
      <WebixComponent ui={getControl(props)} />
    
  );
}

export default Tabbar;