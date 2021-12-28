import {$$} from 'webix';

function setStyleByID(id,cssproperties,val){ 
    let element = document.getElementById(id);
    if (element) {
      element.style[cssproperties] = val;
    }
}

export const showElementChart = (chartID, visibility = "visible") =>{
    setStyleByID(chartID,"visibility", visibility);
};

export function setVisibleElements(visibleElementIDArr, hiddenElementsArr = []){
    hiddenElementsArr.forEach(id => {
        setStyleByID(id,"display","none");
    });
    visibleElementIDArr.forEach(id => {
        setStyleByID(id,"display","");
        setStyleByID(id,"visibility","visible");
    });
}

export const Context = {
    chartkey :"gsJRQxZxl8gBN/ERCJCijRtf12mSQZzsxYICNJYVcZML0wRQFOxkmmSqS8AEwrg52+sJH8YZtvuIxig/QZIzdQ84lNFeM8pVRk7Do0WKQov2HhPsufVZrYNZez2hRE/M41EUHNuggTkTxlyY0kyiv7JbY0sqx970+lFqZHhsw0YeoJik0QoMyaTGZNRBC3yi306UxqXU0ONYbbH7SRPIWLx0+TU9B5oG6Lyfbqx0l7NkSiLLYIHO3qVXqsHPwXAErIb8mVdzxyUdwejLlC++8oN5WtU24RcZ1WlHffXKNGhw+BIX41SI3qgMYe1R3h2Xc7oZI6Va2ek+BUhghnOUZ1HfgoRu2UsMu1tK+kZSmaQS4sR0v+s1SVGSZId1XgCyQUojLaPXTAvSe+SFs2dkmrRxvdVHHwLp+cJeTKmV/aKI8Sb5sqc14lPPzZMW+H0Nlz+gTH3mGDPNRYf8f4AjOX78ajcUbMIDUhgyMgJ/OYDeOBcyS1AKUfEX0zpmSBDM9PD1wOfE5SUsRmW9tFuq1jCdVRWd2JVwnx6Qzi19oUamuBfArKCw",
    model:{m_devices_hash:""},
    updateModel : 0,
    actions:{update:"updateLeftMenu",updateParameters:"updateParameters"},
    count: 0,
    title: "t1",
    elements:{},
    devices :[{name:"dev1a"},{name:"dev2b"},{name:"dev3c"}],
    pages : ["ViewDevices","ViewCPlotWeb","ViewPLC","ViewGraphicTrends"],
    states:{indexDevice:0},
    //Хранит набор параметров для отображения на Chart
    oscilloscopeChartList: ["chart3","chart4","chart5"],
    paramToChart:[],
    paramToCharts:{"chart3":{}, "chart4":{}, "chart5":{}},
    chartList:{"chart3":{setNamesArr: () => {}, setColorsArr: () => {}}, 
            "chart4":{setNamesArr: () => {}, setColorsArr: () => {}}, 
            "chart5":{setNamesArr: () => {}, setColorsArr: () => {}},
            "chartTrends":{setNamesArr: () => {}, setColorsArr: () => {}},
            "chartCPlotWeb":{setNamesArr: () => {}, setColorsArr: () => {}},
        },
    // Временный лист для создания общео списка графиков
    chartListTemporary: [
        {id:"chart1",dev:"",index:0},
        {id:"chart2",dev:"",index:1},
        // {id:"chart3",dev:""},
        // {id:"chart4",dev:""}
    ],
    addNamesColor :function(chartID,setNamesArr,setColorsArr) {
        if (! this.chartList[chartID]) {
            this.chartList[chartID] = {};
        }
        this.chartList[chartID]["setNamesArr"] = setNamesArr;
        this.chartList[chartID]["setColorsArr"] = setColorsArr
    },
    resize :function(event) {
        console.log("resize");
        //let elements = ["$scrollview1"];
        let elements = ["accmain","scrollacc","tabViewControl"];
        
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
        setVisibleElements(visibeElements,this.pages);
        document.documentElement.style.setProperty('--chartcount', 1);  
        // this.pages.forEach(id => {
        //     setStyleByID(id,"display","none");
        //   });
        //   visibeElements.forEach(id => {
        //     document.documentElement.style.setProperty('--chartcount', 1);  
        //     setStyleByID(id,"display","");
        //     setStyleByID(id,"visibility","visible");
        //   });
    }

}