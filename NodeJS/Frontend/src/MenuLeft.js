import 'webix/webix.css';
import WebixComponent from './WebixComponent';
import React from "react";
import {$$} from 'webix';


function showPage(pageID, parentID = "") {
  var sc = document.getElementById(pageID);
  sc.style.setProperty("visibility", "visible");
  // const m = document.getElementById(parentID);
  // if (sc.parentElement.id != parentID) {
  //   m.appendChild(sc)
  // }
  // console.log(sc.parentElement);

}

function hidePages(arrPages,visibeElements) {
  arrPages.forEach(element => {
    document.getElementById(element).style.display = "none";
  });
  visibeElements.forEach(element => {
    document.getElementById(element).style.display = "";
    document.getElementById(element).style.visibility = "visible";
  });
}

 function accordion() {
  return {
    view:"accordion",
    width: 0,
    id:"accmain",
    height:1200,
    multi : false,
    collapsed:true,
    select:true,
    rows:[],
    on:{
      onChange: function(newValue, oldValue, config){
          console.log(newValue)
      },
      onAfterExpand:function(id){
          // console.log("onAfterExpand")
          let pages = ["deviceView","ViewCPlotWeb","ViewPLC","ViewGraphicTrends"]
          console.log(id)
          // console.log($$(id)) 
          let changeId = $$(id);
          let newHeight = 42;
          switch(id)  {
            case "DeviceInit":
              newHeight = 500;
              hidePages(pages,["deviceView"]);
              break;

            case "cPlotWeb":
              hidePages(pages,["ViewCPlotWeb","chartCPlotWeb"]);
              break;

            case "PLC":
              hidePages(pages,["ViewPLC"]);
              break;

            case "GraphicTrends":
              hidePages(pages,["ViewGraphicTrends","chartTrends"]);
              break;
          }
          changeId.config.height = newHeight;
          changeId.resize();
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
