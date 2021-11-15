import React, { useState } from "react";
import {Context} from './Context';

function updateTitle(name) {
  Context.title = name+"-qwe12";
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