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
        setStyleByID(id,"display","block");
        setStyleByID(id,"visibility","visible");
    });
}

export const Context = {
    chartkey :"zbybzSGphR4PphTdLsWJv+f3fKz4TX98buu1h2Yy8PkiSlA0xM9cPbXMVLX3bXlnAqhOIP3xqqrFQLWf7cWC1yMNIhYIyBeRQZ79HXbQoJwzL61/h8oa5brqQob0mI+MS4Baw3kJE278PURCagu6UUOuDvyw4KwuvkjVweAHDQ1V2LA5F5N+JAiHrhHKMrhdWWCtBqwWfZEzV8zpKiD37cpXJw2is2YH1Mc/tG4PqPrv7agZu94uTXM6RLO5d0FM4Tsv7CNG3EL7mYp7vyWfvfS5M37ld0PqrHn2FNAiXgYTyInk4WzYSLwY+k4PBdXRect9r//WkjuTVsYOBm8NDBffqsR6FpaBGTZS3ryKW10qu28Sb175P3+7mftznyaYMxrLs8b6/a+Ph/BXiiXbS4BUrDSAt6OsfQliBoyoAT7NYDhHjqKGWBmwItN0YvVvH74pjpaMVRqnn3FXz/jAEjfI5uvZqIuER/33E3Npu+/HBi9XAB5y79FkIW7q/798/o3stv+ls913R78W6XIeSP/cjPNvy88ItOc6wlS1kTL8LdRuCyLBsA==",
    model:{m_devices_hash:""},
    updateModel : 0,
    actions:{update:"updateLeftMenu",updateParameters:"updateParameters"},
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