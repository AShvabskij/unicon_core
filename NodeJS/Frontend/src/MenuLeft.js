import 'webix/webix.css';
import WebixComponent from './WebixComponent';
import React from "react";
import {$$} from 'webix';


 function accordion() {
  return {
    view:"accordion",
    width: 0,
    id:"accmain",
    multi : false,
    collapsed:true,
    select:true,
    rows:[],
    on:{
      onChange: function(newValue, oldValue, config){
          console.log(newValue)
      },
      onAfterExpand:function(id){
          console.log("onAfterExpand")
          console.log(id)
          console.log($$(id))
          
      }
    }
  }
}


 function MenuLeft(props) {
  // console.log("MenuLeft ");
  // console.log(props.devtitle);
  
  return ( 
      // <Webix ui={getUI3(props.devtitle)} data={props.devtitle}/>
      <WebixComponent ui={accordion()} data={props.devtitle} />
  );
}

export default MenuLeft;