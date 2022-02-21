import React, { useRef, createRef, useEffect } from 'react';

import * as webix from 'webix/webix.js';
import 'webix/webix.css';
//https://github.com/boypanjaitan16/webix-react-hooks/blob/cefe81ec8e53e83bce4b1a86de83e4a5ccace0be/src/components/WebixComponent.js

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
    // console.log(uiState.current.addView);
    if (uiState.current.setValues)
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
      UpdateItems(uiState.current,dataToUpdate);
      for (var i = 0; i < n ; i++) {
        // console.log(uiState.current._cells[i]._settings.id );
        // console.log(uiState.current._cells[i]);
        // uiState.current._cells[i].config.body.template = "qwe";
        // console.log(uiState.current._cells[i]._headlabel.innerText);
        // console.log(uiState.current._cells[i].config.id);
        // UpdateItems(uiState.current._cells[i],dataToUpdate,uiState.current);
        // uiState.current.removeView(uiState.current._cells[i].config.id);
        // if (uiState.current._cells[i]._headlabel.innerText == )
        
      }
      // dataToUpdate.forEach(element => {
      //   uiState.current.addView(
      //       { view:"accordionitem",
      //         header:element.header, 
      //         body: element.body, 
      //         // id:element.id 
      //     }
      //   );
      //   }
      // );
    //  uiState.current.removeView("mytemplate2");
    //   uiState.current.addView({ view:"accordionitem",
    //   header:dataToUpdate, body: "", id:"mytemplate2" }
    //     );

        //uiState.current.reconstruct();

    //   console.log("setWebixData");
    //   console.log(uiState.current.rows);
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
    <div ref={webixRef}></div>
  );
}

export default WebixComponent;