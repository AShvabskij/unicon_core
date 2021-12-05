import {$$} from 'webix';

function setStyleByID(id,cssproperties,val){ 
    let element = document.getElementById(id);
    if (element) {
      element.style[cssproperties] = val;
    }
}

export const Context = {
    chartkey :"K6uorSFNu5My0BzJDCrEb8JHNOcyJfdEMCzpBrttKe7rnLKqhW5IAWf9YJw8R8r0dyhzopvSRffGUAvkZf7AO2tgwnEb7BIbzH0Qap+cEskjV2wjSQ2PppbU+oqQJ2uqkyTyrCvCav8Xzb7L8AE9NfhUoGytYCj/6fhzYYF6v1XSLVtNZkLqvPlqvMnL5ZVaFGjkS+gYxWhCMBHGMO8Pklus+6zjoP0d0dmjgliL+1sfDDMfe9IcAlHfEG9IJk0XxGe90D2ibvspAc/nwD3MattGGkxgCOyely0GT+xf1fevo4l0d6/mIm4o+pVSv78U9OrZ7BDQl1g3COvbAIW/P8ZQQsrA7hcO0j2cs0XObZ2KcV4Tx/wXesUenJnAGGXNCBrQIZ3g8pv1kIFQT25anzerw+vhLhGFVIGPP4rnqEgNgekXsXNbWPvZgXG+oqP0jWQ/+YO/+kogq9yZYUOswL6B9jkI6c3RvNBJ4HUeoV4bwUuUUJJvFub2ID8MUQcolz19m3DUNYokmuQ0aoefIbU7zvB9rf2j0vFrNARtSJDVMFnd",
    model:{m_devices_hash:""},
    updateModel : 0,
    actions:{update:"updateLeftMenu",updateParameters:"updateParameters"},
    count: 0,
    title: "t1",
    elements:{},
    devices :[{name:"dev1a"},{name:"dev2b"},{name:"dev3c"}],
    pages : ["deviceView","ViewCPlotWeb","ViewPLC","ViewGraphicTrends"],
    states:{indexDevice:1},
    chartList: [
        {id:"chart1",dev:"",index:0},
        {id:"chart2",dev:"",index:1},
        // {id:"chart3",dev:""},
        // {id:"chart4",dev:""}
    ],
    resize :function(event) {
        console.log("resize");
        //let elements = ["$scrollview1"];
        let elements = ["accmain","scrollacc"];
        
        elements.forEach(function(item, index, array) {
            //if(item == "controlview") $$(item).resize();
            if ($$(item)) {
                if ($$(item)["adjust"]) {
                   $$(item).adjust();
                }
            }
           
        });
    },
    showPages :function(visibeElements) {
        this.pages.forEach(id => {
            setStyleByID(id,"display","none");
          });
          visibeElements.forEach(id => {
            document.documentElement.style.setProperty('--chartcount', 1);  
            setStyleByID(id,"display","");
            setStyleByID(id,"visibility","visible");
          });
    }

}