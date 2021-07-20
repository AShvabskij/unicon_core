import 'webix/webix.css';
import Webix from './Webix';
import FilmsView from "./FilmsView1";
import Chart from "./Chart";
import React,{ useState,useReducer } from "react";
import * as webix from 'webix/webix.js';

function tabbar() {
  return {
    type:"space", padding:1, width:1500, height:450,
    rows:[
      {
        // type:"clean",
        rows:[
          {
            borderless:true, 
            view:"tabbar", 
            id:"tabbar", 
            value:"listView", 
            multiview:true, 
            options:[
              { value:'List 1', id:'ac123'},
              { value:'Form', id:'memo2'},
              { value:'Empty', id:'memo3'},
              { value:'Empty1', id:'memo4'}
            ]
          },
          {
            cells:[
              {
                id:"ac123",
                view:"htmlform",
                content:"memo1"
              },
              {
              id:"memo2",
              view:"htmlform",
              content:"memo2"
              },
              {
                id:"memo3",
                view:"htmlform",
                content:"memo3"
                },
                {
                  id:"memo4",
                  view:"htmlform",
                  content:"memo4"
                  },
              // {
              //   id:"emptyView",
              //   view:"htmlform",
              //   content:"scichart-root2"
              //   },
             
              
            ]
          }
        ]
      }
    ]
  }
}



function tabview() {
  return {
           view:"tabview", 
           height:500,
            cells:[
              {
                id:"ac123",
                view:"htmlform",
                content:"memo1"
              },
              {
              id:"memo23",
              view:"htmlform",
              content:"memo2"
              },
              {
                id:"memo3",
                view:"htmlform",
                content:"memo3"
                },
                {
                  id:"memo4",
                  view:"htmlform",
                  content:"memo4"
                  },
              // {
              //   id:"emptyView",
              //   view:"htmlform",
              //   content:"scichart-root2"
              //   },
             
              
            ],
            on:{
              onItemClick: function(id){
                console.log("You have clicked on an item with id="+id); 
                  // alert("You have clicked on an item with id="+id); 
              }
          }
          }
        
}

function tabview1() {
  return {
    view:"tabview",
    height: 800,
    cells:[     
      {
        id: "tv1",
        header:"Form",
        body:{
          id:"w3",  
          template:"<div id='f1'>Form Content<div>"     
        }      
      },
      { 
        id: "tv2",
        header:"Empty", 
        body:{
          id:"w2",  
          view:"htmlform",
          content: "memo3" 
        } 
      },
      { 
        id: "tv2",
        header:"Empty", 
        body:{
          id:"w1",  
          view:"htmlform",
          content: "memo1" 
        } 
      }
    ],
    tabbar:{
      on:{
        
        onAfterTabClick:function(id,ev){
          webix.message("tab was clicked 12345"+this.getValue());
          // console.log("----->>>>>>111111");
          // console.log("onAfterTabClick 123"); 
          // console.log(id)
          // console.log("onAfterTabClick 5"); 
          if (id=="w2") {
            var sc =  document.getElementById("chart2");
            sc.style.setProperty("visibility","visible");
            const m =  document.getElementById("memo3");
            if (sc.parentElement.id != "memo3") {
              m.appendChild(sc)
            }
            console.log(sc.parentElement);
          }

          if (id=="w1") {
            var sc =  document.getElementById("chart1");
            sc.style.setProperty("visibility","visible");
            const m =  document.getElementById("memo1");
            if (sc.parentElement.id != "memo1") {
              m.appendChild(sc)
            }
            console.log(sc.parentElement);
          }

        },
        onChange: function(newValue, oldValue, config){
          // config is {yourProperty: "yourValue"}
          console.log(this);
          console.log("----->>>>>>"+this.getValue());
          // var chart1 = document.getElementById("wwwqqq");
          // console.log(chart1);
          //avp.updateDevice();
        }
      }
    },
    on:{
      onChange: function(newValue, oldValue, config){
        // config is {yourProperty: "yourValue"}
       //console.log(this);
        console.log("!!!!!----->>>>>>"+newValue);
        console.log("!!!!!----->>>>>>"+oldValue);
        // var chart1 = document.getElementById("wwwqqq");
        // console.log(chart1);
        //avp.updateDevice();
    }
  }
}
}


function Counter() {
  
  return (
    <>
      <h1>count: Demo</h1>
      <Webix ui={tabview1()} />
    </>
  );
}



export default Counter;