import 'webix/webix.css';
import WebixComponent, {scroll} from './WebixComponent';
import React,{useEffect,useState} from "react";
import {Context} from './Context';
import { observer } from "mobx-react"

function label(caption, css,width) {
  return { "label": caption, "view": "label", css:css, width: width}

}

const GetCurrentTime = () => {
  let d = new Date;
  return d.getHours().toString().padStart(2,0)+":"+d.getMinutes().toString().padStart(2,0)+":"+d.getSeconds().toString().padStart(2,0);
}

const Footer = observer(({  }) => {
  const [ timer, setMinutes ] = useState(GetCurrentTime());
  useEffect(()=>{
    let myInterval = setInterval(() => {
           setMinutes(GetCurrentTime());
        }, 1000)
        return ()=> {
            clearInterval(myInterval);
          };
    });


  return ( 
    <div className="footer">
      <WebixComponent ui={label("Load","footer_label",0)} data={Context.status} />
      <WebixComponent ui={label("Data","footer_label",0)} data={"Data"} />
      <WebixComponent ui={label("state","footer_label",0)} data={"state"} />
      <WebixComponent ui={label(timer,"footer_label_right",0)} data={timer} />
     </div> 
  );
});

export default Footer;
