import React, { useState } from "react";
import {Context} from './Context';

function updateTitle(name) {
  Context.title = name+"-qwe12 ";
  Context.model1.m_devices_hash = name;
  // Context.model.m_devices = [];
  console.log(Context.model1.m_devices_hash);
  Context.model.m_devices.push("");
}

function Button(props) {
     return (
      <div>
        <button onClick={() => updateTitle(props.name)}>
          Нажми на меня {Context.title}
        </button>
      </div>
    );
  }
  
  export default Button;