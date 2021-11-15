import React, { useRef, createRef, useEffect } from 'react';

import * as webix from 'webix/webix.js';
import 'webix/webix.css';
//https://github.com/boypanjaitan16/webix-react-hooks/blob/cefe81ec8e53e83bce4b1a86de83e4a5ccace0be/src/components/WebixComponent.js


export function scroll(view, id = "") {
  let v = {view: "scrollview",
  scroll: "y",
  css:"demoscroll",
  body: view
  };
  if (id != "") {
    v["id"] = id;
  }
  return v
}

function UpdateItems(current,dataList) {
  current.reconstruct();
  // let n = current._cells.length;
   dataList.forEach(element => {
    current.addView(
                { view:"accordionitem",
                  header: element.header, 
                  body: element.body, 
                  id:element.id,
                  onAfterExpand:function(id,event){
                    console.log("onAfterExpand");
                  } 
              }
            );
   });
  let n = current._cells.length;
   for (var i = 0; i < n-1 ; i++) {
      current._cells[i].collapse();
   }
  //   console.log("accordionItem._headlabel.innerText");
  //   console.log(element.header);
  //   let flagAdd = true;
  //   for (var i = 0; i < n ; i++) {
  //     if (current._cells[i]._headlabel.innerText == element.header) {
  //       current._cells[i].body
  //       current.removeView(current._cells[i].config.id);
  //       flagAdd = false;
  //       current.addView(
  //           { view:"accordionitem",
  //             header: element.header, 
  //             body: element.body, 
  //             // id:element.id 
  //         }
  //       );
  //     }
  //   }

  //   if (accordionItem._headlabel.innerText == element.header) {
  //       current.removeView(accordionItem.config.id);
  //       flagAdd = false;
  //       current.addView(
  //           { view:"accordionitem",
  //             header: element.header, 
  //             body: element.body, 
  //             // id:element.id 
  //         }
  //       );
  //     }
  //   if (flagAdd) {
  //     current.addView(
  //       { view:"accordionitem",
  //         header: element.header, 
  //         body: element.body, 
  //         // id:element.id 
  //     }
  //   );

  //   }
  //   }
  // );
  // current.resize();
}


function WebixComponent({data, ui}){
  const webixRef  = createRef()
  const uiState   = useRef()     

  const setWebixData= (dataToUpdate) => {
    console.log(uiState.current);
    if (uiState.current["setValues"])
      uiState.current.setValues(dataToUpdate);
    else if (uiState.current.parse)
      uiState.current.parse(dataToUpdate)
    else if (uiState.current.setValue)
      uiState.current.setValue(dataToUpdate)
    else if (uiState.current.addView) {
    //   uiState.current.rows = [{ header:"Graphic trends 12", body: ""}]; 
      console.log("uiState.current.rows");
      console.log(uiState.current.index);
      let n = uiState.current._cells.length;
      //UpdateItems(uiState.current,dataToUpdate);
      
    }

    if (uiState.current["config"]["view"] == "scrollview"){
      console.log("scrollview");
      if (uiState.current["_body_cell"]["config"]["view"] == "accordion") {
        console.log("accordion");
        UpdateItems(uiState.current["_body_cell"],dataToUpdate);
      }
    }
    if (uiState.current["config"]["view"] == "accordion"){
      UpdateItems(uiState.current,dataToUpdate);
    }
  }

  useEffect(() => {
    uiState.current = webix.ui(
  	  ui, 
  	  webixRef.current
    );
  
    return () => {
      ui.current  = null;
    }
  }, [])

  useEffect(() => {
      if (data){
        console.log("data");
        console.log(data);
        setWebixData(data);
      }
  }, [data])

  
  return (
    <div className="webixitem" ref={webixRef}></div>
  );
}

export default WebixComponent;