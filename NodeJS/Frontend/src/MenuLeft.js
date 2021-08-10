import 'webix/webix.css';
import WebixComponent from './WebixComponent';
import React from "react";


function devicesWebix(devicesArr) {
  // console.log("devicesWebix");
  //   console.log(devicesArr);
  let  devices = { margin:10, padding:0, type:"wide",
  view:"flexlayout",cols:[]};
  devicesArr.forEach(function(item, index, array) {
    // console.log(item, index);
    devices.cols.push( { view:"toggle", label:item, minWidth: 90,
      click:function(id,event){
      }
    })
  });
  return devices;
}

/* var devices = {
  // view:"layout", // необязательно
  // id:"devices", 
  // rows:[
  //     { height:10 },
  //     { responsive:"devices", 
      margin:10, padding:0, type:"wide",
      // height: 0,
    view:"flexlayout",
     
      cols:[ // родительский лейаут для компонента 
            { //view:"button", value:"Device 1", minWidth: 120, align:"center" ,
              view:"toggle", label:"Device 1", minWidth: 90,
              click:function(id,event){
              }
            },
            { view:"button", value:"Device 2", minWidth: 90, align:"center" ,
              click:function(id,event){
              }
            }, // компонент будет перемещен в лейаут "devices"
            { view:"button", value:"Device 3", minWidth: 90, align:"center" ,
              click:function(id,event){
              }
            }, // компонент будет перемещен в лейаут "devices"
            { view:"button", value:"Device 4", minWidth: 90, align:"center" ,
              click:function(id,event){
              }
          }, // компонент будет перемещен в лейаут "devices"
          { view:"button", value:"Device 5", minWidth: 90, align:"center" ,
            click:function(id,event){
            }
          } 
        ]
    //     },
    //     { height:10 }
    // ],

    // },
    // { width:20 }
  
}; */

// function menuaccordion(params) {
    
//   return {
//     view:"accordion",
//     // id:"top:accordion", 
//     // type:"wide",
//     // css:"app_menu",
//     width: 0,
//     // height: 0,
//     minHeight:400,
//     maxHeight:0,
//     multi : false,
//     collapsed:true,
//     select:true,
//     rows:[
//       { header:"Graphic trends", body: ""},
//       { header:"PLC", body: "" },
//       { header:"Logs", body: ""},
//       { header:"Devices", body: devicesWebix(params)},
//     // { header:"Devices", body: getUI2("") },
//       // { header:"Device1", body: menu_acc },
//       // { header:"Device 2", body: menu_acc2 },
//       // { header:"Device 3", body: ""}
//     ],
//     on:{
//       onMenuItemClick: function(id, e, node){
//         // config is {yourProperty: "yourValue"}
//         // console.log("onMenuItemClick");
//         // console.log(node);
//         // baseTableUpdate();
//       }
//     }
//   }
// }


// function getUI3(props){

//   const cells = [
//     { header:"<span class='webix_icon mdi mdi-file-video'></span>UNICON",
//     id:"unicon",
//     maxwidth: "25%",
//     body: 
//       menuaccordion(props)
//     }
//   ];


//   return {
//     view:"tabview",
//     height: 0,
//     // width: 0,
//     // tabbar:{ options:["A","B","C"]}, 
//     animate:false,
//     cells:cells
//   }

// }


 function accordion() {
  return {
    view:"accordion",
    width: 0,
    minHeight:400,
    maxHeight:0,
    multi : false,
    collapsed:true,
    select:true,
    rows:[],
    onAfterExpand:function(id,event){
      console.log("onAfterExpand");
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