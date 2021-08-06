import 'webix/webix.css';
import Webix from './Webix';
import React from "react";


var devices = {
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
  
};


var menu_acc = {
  view:"menu", 
  id:"top:menu_acc", 
  css:"app_menu",
  width:0, layout:"y", 
  select:true,
  template:"<span class='webix_icon #icon#'>123</span> #value# ",
  // data: avp.getDeviceFromJSON(mainDataJsonString),
  on:{
    onMenuItemClick: function(id, e, node){
      // config is {yourProperty: "yourValue"}
      console.log("onMenuItemClick");
      console.log(node);
      // baseTableUpdate();
    }
    }
};

var menu_acc2 = {
  view:"menu", 
  id:"top:menu_acc2", 
  css:"app_menu",
  
  select:true,
  template:"<span class='webix_icon #icon#'></span> #value# ",
  //data: avp.getDeviceFromJSON(mainDataJsonString),
  on:{
    onMenuItemClick: function(id, e, node){
      // config is {yourProperty: "yourValue"}
      console.log("onMenuItemClick");
      console.log(node);
      // baseTableUpdate();
    }
    }
};

var menuaccordeon ={
  view:"accordion",
 // id:"top:accordion", 
  // type:"wide",
  width: 0,
  // height: 0,
  minHeight:400,
  maxHeight:0,
  multi : false,
  collapsed:true,
  rows:[
    { header:"Graphic trends", body: ""},
    { header:"PLC", body: "" },
    { header:"Logs", body: ""},
    { header:"Devices", body: devices},
   // { header:"Devices", body: getUI2("") },
    // { header:"Device1", body: menu_acc },
    // { header:"Device 2", body: menu_acc2 },
    // { header:"Device 3", body: ""}
    
  ]
}



function getUI3(props){

  const cells = [
    { header:"<span class='webix_icon mdi mdi-file-video'></span>UNICON",
    id:"unicon",
    maxwidth: "25%",
    body: 
      menuaccordeon
    }
  ];


  return {
    view:"tabview",
    height: 0,
    width: 0,
    // tabbar:{ options:["A","B","C"]}, 
    animate:false,
    cells:cells
  }

}


export default class MenuLeft extends React.Component {
   constructor(props) { 
     super(props);
     this.title = "first title"
     this.state = { title: "state title" };
   };
 
   componentDidMount = () => {
     /**
       * Обязательная регистрация компонента с параметрами вызова
     */
     //this.props.ButtonStore.registration(this.props);
   };
 
   componentWillUnmount = () => {
    // this.props.ButtonStore.unmount(this.props.name);
   };

  //  handleClick() {
  //    this.title = "click";
  //    console.log("handleClick "+this.title )
  //    this.setState({ title: "handleClick state title" });
  //  }

  
 
   render() {
    //  const {
    //    ButtonStore,
    //    disabled,
    //    name,
    //    text,
      
    //  } = this.props;

     console.log("render CustomButton");
     return (
           <Webix ui={getUI3(this.props)} />
       
     );
   }
 }