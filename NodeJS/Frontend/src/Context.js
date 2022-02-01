import {$$} from 'webix';

export const setStyleByID = (id, cssproperties, val) => { 
    console.log("setStyleByID, id = " + id);
    let element = document.getElementById(id);
    if (element) {
      element.style[cssproperties] = val;
    }
}

export const removeCssClass = (id,cssStyle) => { 
    let element = document.getElementById(id);
    if (element) {
      element.classList.remove(cssStyle);
        // element.classList.add(cssStyle);
    }
}

export const addCssClass = (id,cssStyle) => { 
    // console.log("addCssClass "+id+ " "+cssStyle);
    let element = document.getElementById(id);
    if (element) {
        console.log("addCssClass "+id+ " "+cssStyle + " ok");
        element.classList.add(cssStyle);
    }
}

export const showElementChart = (chartID, visibility = "visible") =>{
    setStyleByID(chartID,"visibility", visibility);
    if (visibility == "visible") {
        setStyleByID(chartID,"display","block");
    } else {
        setStyleByID(chartID,"display","none");
    }
};

export function setVisibleElements(visibleElementIDArr, hiddenElementsArr = []) {
    hiddenElementsArr.forEach(id => {
        setStyleByID(id,"display","none");
        setStyleByID(id,"visibility","hidden");
    });
    visibleElementIDArr.forEach(id => {
        setStyleByID(id,"display","block");
        setStyleByID(id,"visibility","visible");
    });
}

export const Context = {
    chartkey :"gu+2fCx0keJAMYee7rONF7fm5eVx4W4gKfRGACDXju3mwhDCLm+LwgbY1gVGz6YAro8vpyt/ymjhookolQkv7JfvaeVNI0OhH1hPYPSfX/wPC2uTamVa2LxChz6A/U9CkB7yAJanm7O9KfvymReEyoI0yle8ie01K+fqHH/a2Y7zCOH7Q0y4LncQbLMzYaPsN7OSJEgvpUIbjDXv/O05lLlnl8xYYEOBgANUu5NrJQw2JMUOtqz5/8xzrEZ2P4PRxox3bw6OKksnk1dtIo4bzS+4AWGkLCRsJH45YF1WJErLr+UcvlciVZcA3fEsVvi64Yg8ti4+Ul/wfMOBHY/0SLA8qF5xSjO64nj487lOgK9u2z2ojImMv7H3lX+z29yV8O/17JU/ClEtR/aH0oe5H0mh30A+sg7Ca3ujOAMmhW5el0nGTOuyU70CoE0t4MdvBMLim/08Jn0JDnP0FAU0/ehwefqqFzmIyXdBMtrM8Pg84RwiD//q6AlI0Tl08ahHH+b1wH8629sr3sBjH5V62MAvQcuSWBw0f4uyh7ZgCN8bpjaGz2hDy1u1r5JYSQjV+AJzVjmtrj0F+yL2Rq1WfzrUBvnvzmS01+g48BW+FRxmky38Bu0ixNo0ACyaIbH12eFou3zlw9eNTh3wUfInpmEFUHxp2w4g",
//  chartkey :"sfEa83FP+0EQ0l+ZJmuDoxvUMg7pFn7Dz8sL1p/A2/Qio7pYbQ9JSb3J9aFRSXe/v++/nx0clUpkAVbHRZw0t3G7Y74U0kaRPDWIT64SO8qoie6wRzdFUXVYWh6aPDZE+Bz/ye8kL0vwgwSWnQoIpvdvLKwwXmbk1h2Wsuc6MGaMfMvttPICImTQoUDIjwV0tndupRkJXwOjALmY+0CDY4v2pSoNcnkQIDv/kB7BG3SJ7luhe4WT7PkiAQkNq2e0fYLYzstm/2M7NqfE4LtsEj2v5/bBb763MeLMvxGb91LhpJMJUqCq/n4UORLgsiOrPhdB0OU+CP8alvSBuLoBiH2Pba3n/5gtv/sfE7+4BnFm3xQ/jmTKNtyXlFUk4qVcLG8BJPHhVBtRH0f62SV/XU9zKGF7Dy8jXQOxrb7Oje+MZPRf+LK3ln2bO9kB9T0aUR0ZaQsTGRERT8AIQZKmcI4mclROwHIRMzEoE33kRBliN1EOA7Jhkkb17ejUHneiLLMD9KIgMv4BKXC/ugX9JpvF6iwrFx4o7nHRWH92aAYY+Uof7ADA",
    model:{m_devices_hash:""},
    status:"Loading...",
    updateModel : 0,
    actions:{update:"updateLeftMenu",updateParameters:"updateParameters", updateOscilloscope: "updateOscilloscope"},
    count: 0,
    title: "t1",
    elements:{},
    devices :[{name:"dev1a"},{name:"dev2b"},{name:"dev3c"}],
    pages : ["ViewDevices","ViewCPlotWeb","ViewPLC","ViewGraphicTrends","devicesParametersTab"],
    states:{indexDevice:-100},
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
    
    deviceChartList: new Map(),
    /*[ // лист для создания списка графиков по устройствам
        {deviceId:0, charts:["chart0"]}
    ], */

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
        // Этот код будет полезен для стека графиков  
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