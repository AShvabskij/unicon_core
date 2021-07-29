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
    view:"flexlayout",
      
      
      cols:[ // родительский лейаут для компонента 
            { //view:"button", value:"Device 1", minWidth: 120, align:"center" ,
              view:"toggle", label:"Device 1", minWidth: 120,
              click:function(id,event){
              }
            },
            { view:"button", value:"Device 2", minWidth: 120, align:"center" ,
              click:function(id,event){
              }
            }, // компонент будет перемещен в лейаут "devices"
            { view:"button", value:"Device 3", minWidth: 150, align:"center" ,
              click:function(id,event){
              }
            }, // компонент будет перемещен в лейаут "devices"
            { view:"button", value:"Device 4", minWidth: 150, align:"center" ,
              click:function(id,event){
              }
          }, // компонент будет перемещен в лейаут "devices"
          { view:"button", value:"Device 5", minWidth: 150, align:"center" ,
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

function getUI(select){
  return {
    view:"slider"
  };
}

function getUI1(props){
  const cells = [
    { header:"<span class='webix_icon mdi mdi-file-video'></span>List", body:{
      view:"list",
      template:"#rank#. #title# <div style='padding-left:18px'> Year:#year#, votes:#votes# </div>",
      type:{
        height:60
      },
      select:true
      
    }},
    { header:"<span class='webix_icon mdi mdi-comment'></span>Form", body:{
      template:"Place for the form control"
    }},
    { header:"<span class='webix_icon mdi mdi-help-circle'></span>About", body:{
      template:"About the app"
    }}
  ];

  const data = {
    cells:[
      {
        id:"listView",
        view:"list",
        template:"#rank#. #title# <div style='padding-left:18px'> Year:#year#, votes:#votes# </div>",
        type:{
          height:60
        },
        select:true,
       
      },
      {
        id:"formView",
        template:"Place for the form control"
      },
      {
        id:"aboutView",
        template:"About the app"
      }
    ]
  };
  
    return {

        "view": "tabbar",
        // "id" : "top:toolbar1",
        "options": [
          { value:"Parameters 1", id:"data3",  icon:"wxi-pencil" },
          { value:"Оscilloscope", id:"chart3",  icon:"wxi-pencil" },
          { value:"Control", id:"device_manage",  icon:"wxi-pencil" },
          { value:"Info", id:"data_m",  icon:"wxi-pencil" },
        ],
        cells:cells,
        on:{
          
          onChange: function(newValue, oldValue, config){
            // config is {yourProperty: "yourValue"}
            console.log("onChange ->>>>>>>>>>>");
            console.log(newValue);
            props.onClickMenu("3");
            //avp.updateDevice(newValue);
            // avp.baseTableUpdate();
          }
        }
  }
}

function getUI2(select){
  return {
    view:"datatable", scroll:false, width:0, autoheight:true, select:true, columns:[
      { id:"name", header:"name" },
      // { id:"email", fillspace:1 },
      // { id:"age", width: 50 }
    ],
    on:{
      onAfterSelect:function(id){
        console.log(" onAfterSelect ->>>>>>>>>>>");
        //select(id);
      }
    },
  };
}

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
    { header:"<span class='webix_icon mdi mdi-file-video'></span>Can",
    id:"can",

    body: 
      menuaccordeon
    
      
      
    },
    { header:"<span class='webix_icon mdi mdi-comment'></span>MBus", 
    id:"can2",
    body:{
      template:"Place for the form control"
    }},
    { header:"<span class='webix_icon mdi mdi-help-circle'></span>FO", 
    id:"can3",
    body:{
      template:"About the app",
      select:true
    }}
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
const LeftMenu = ({ data, select }) => (
  <Webix ui={getUI2(select)} data={data} />
)
const LeftMenu1 = ({ data }) => (
  <Webix ui={getUI1()} data={data} />
)



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