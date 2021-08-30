import React, {useState,useEffect} from "react";
import {$$} from 'webix';
import * as webix from 'webix/webix.js';

export const Context = React.createContext();
let startDate = new Date();
const interfaceName = {1: 'Can', 2: 'MBus', 3: 'FO'};

const updateLeftMenu = (devices) => {
    let options = [];
    devices.forEach(function(item, index, array) {
        // console.log(item, index);
        let label = item.name + "</br>Channal: " + interfaceName[item.interface]
        options.push( { id:index, value:label, css:"devices"})
 
        }); 
    let comp = {view:"segmented", css:"devices", height:100, multiview:true, value:1, options:options,
        click:function(id,event){
            // console.log(id)
        },
        on:{
            onChange: function(newValue, oldValue, config){
                console.log(newValue)
            },
            onItemClick:function(id,ev){
                console.log(id)
                console.log(ev)
            }
        }
    }
   
    webix.ui(comp,$$("DeviceInit"), 0);
} 

export const Info = {startTime:startDate.getTime(),model:{},actions:{updateLeftMenu:updateLeftMenu},elements:{}}

export const AddCounter = () => {

};

