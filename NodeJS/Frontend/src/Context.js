import {$$} from 'webix';
import { makeObservable, observable, action } from "mobx"

export const setStyleByID = (id, cssproperties, val) => { 
    let element = document.getElementById(id);
    if (element) {
      element.style[cssproperties] = val;
    }
}
export const getStyleByID = (id, cssproperties) => { 
    let element = document.getElementById(id);
    if (element) {
      return element.style[cssproperties];
    }

    return '';
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
//  chartkey :"gJx71maeV36Rp3a4YNg4n/MHqoSMB4AXXuu10wUT4kCUv2QEci+3D6WbWhilM/rWekgkEsJN3nT7AW5d1YUXmc+7q+1bpW1AIK4Rizbqg0Xn2zCODHB0aEaDpJqOxmb8t5pQLxEilkGI7pz1olyZML1t1XXeQngxcRnpefk1ZYBaIX+m06UeGhiSXlPAZWKcdepearGZIGajn8bn5c/smlnPZCoxCu02q+X5xbFUAhfTgz5lX/J6i6vTvMGZLK2mxRqvzrhUy3NnhDNTYjLfUSszJYswHX3L635QU9lIjIWIBF7Z9akMuiJ++qeVqCbg+TH4YD0hIRsThah+nMxikyeYLyqxUjqCgwBeCM0fqY7x6tBScMU9O2eFcBdZi9/NYwQLL/e4NY7JkBEIJRJX9uEvU5otyCC7eToaH3Zloq9BcPf5doGqxjgFyxJkOMTtqUXOmzfudfmt3HsG4oi/Ojwrt9qbX1sYlln9xlsO1fk1Rz4aNvWpk/KjrR9efK5pz5YETVrZMT4xmJPUQlhBxPkoX/oq1k65jnO1WiVmjLB+oY/iLRc1dksVnsQdbJvb",
    chartkey :"sfEa83FP+0EQ0l+ZJmuDoxvUMg7pFn7Dz8sL1p/A2/Qio7pYbQ9JSb3J9aFRSXe/v++/nx0clUpkAVbHRZw0t3G7Y74U0kaRPDWIT64SO8qoie6wRzdFUXVYWh6aPDZE+Bz/ye8kL0vwgwSWnQoIpvdvLKwwXmbk1h2Wsuc6MGaMfMvttPICImTQoUDIjwV0tndupRkJXwOjALmY+0CDY4v2pSoNcnkQIDv/kB7BG3SJ7luhe4WT7PkiAQkNq2e0fYLYzstm/2M7NqfE4LtsEj2v5/bBb763MeLMvxGb91LhpJMJUqCq/n4UORLgsiOrPhdB0OU+CP8alvSBuLoBiH2Pba3n/5gtv/sfE7+4BnFm3xQ/jmTKNtyXlFUk4qVcLG8BJPHhVBtRH0f62SV/XU9zKGF7Dy8jXQOxrb7Oje+MZPRf+LK3ln2bO9kB9T0aUR0ZaQsTGRERT8AIQZKmcI4mclROwHIRMzEoE33kRBliN1EOA7Jhkkb17ejUHneiLLMD9KIgMv4BKXC/ugX9JpvF6iwrFx4o7nHRWH92aAYY+Uof7ADA",
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
    chartControls: [], // ["chart3" : [addVarPointRange, clearChart, scale, setVisibility, ...]]
    oscilloscopeChartList: ["chart3","chart4","chart5"], //Хранит набор параметров для отображения на Chart
    paramToCharts: new Map(), // { deviceId: {"chart3":{}, "chart4":{}, "chart5":{}} }
    chartList:{"chart3":{setNamesArr: () => {}, setColorsArr: () => {}}, 
            "chart4":{setNamesArr: () => {}, setColorsArr: () => {}}, 
            "chart5":{setNamesArr: () => {}, setColorsArr: () => {}},
            "chartTrends":{setNamesArr: () => {}, setColorsArr: () => {}},
            "chartCPlotWeb":{setNamesArr: () => {}, setColorsArr: () => {}},
        },
    
    deviceChartList: new Map(),
    chartsLength: 0,
    /*[ // лист для создания списка графиков по устройствам
        {deviceId:0, charts:["chart0"]}
    ], */

    deviceCharts: function(indexDevice) {
        let res = [];
        let ind = indexDevice !== undefined ? indexDevice : Context.states.indexDevice;
        
        if (Context.deviceChartList.has(ind)) {
          res = Context.deviceChartList.get(ind);
        }
      
        return res;
    },

    addChart: function(deviceIndex, chartId) {
        if (!Context.deviceChartList.has(deviceIndex)) {
            Context.deviceChartList.set(deviceIndex, []);
        }

        let charts = Context.deviceChartList.get(deviceIndex);
        if (chartId) {
          charts.push(chartId);
          this.chartsLength = charts.length
        }
    },

    popChart: function(deviceIndex) {
        let charts = [];
        if (Context.deviceChartList.has(deviceIndex)) {
            charts = Context.deviceChartList.get(deviceIndex);
        }
  
        if (charts.length === 0) return;
      
        charts.pop();
        this.chartsLength = charts.length; 
    },

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