export function baseTableUpdate() {
	console.log("update");
    // let data_url = "http://127.0.0.1/settings/data.json";
	// $$("tableinfo").load(data_url);
}


var mainDataJsonString = JSON.stringify({
	devices : [
		{device: "dev1", name:"Device 1", interface : {can:true, mbus:false, fo:true} , groupparams : ["m1","acp"]},
		{device: "dev2", name:"Device 2", interface : {can:false, mbus:false, fo:true}, groupparams : ["m1","acp","relay"]},
	],
	groupparams : [
		{id : "m1", goupname : "Module 1 (1800)" , interface : {can:true, mbus:false, fo:true}},
		{id : "m2",goupname : "Module 2 (3100)" , interface : {can:true, mbus:false, fo:true}},
		{id : "acp",goupname : "АЦП (1700)" , interface : {can:true, mbus:true, fo:true}},
		{id : "relay",goupname : "Реле (2100)" , interface : {can:false, mbus:false, fo:true}}
	]
});

export function getDeviceFromJSON(json) {
	var dataFromServer = JSON.parse(json);
	var data = [
		{ value:"Device 12", id:"start", icon:"wxi-columns" },
		{ value:"Device 23", id:"start1", icon:"wxi-columns" },
		{ value:"Data",		 id:"data",  icon:"wxi-pencil" }
	];
	console.log("getDeviceFromJSON");
	return data;
}

export function getGroupParamFromJSON(json,protocol) {
	var dataFromServer = JSON.parse(json);
	// console.log(dataFromServer.groupparams);
	var data = [];
	dataFromServer.groupparams.forEach( function (element, i) {
		// console.log(element.goupname);
		if (element.interface[protocol]) {
			data.push({value:element.goupname,id: "groupparams"+i })
		}
	});
	// var data = [
	// 	{ value:"Device 12", id:"start", icon:"wxi-columns" },
	// 	{ value:"Device 23", id:"start1", icon:"wxi-columns" },
	// 	{ value:"Data",		 id:"data",  icon:"wxi-pencil" }
	// ];
	// console.log("getDeviceFromJSON");
	return data;
}



export function updateDevice(protocol) {
	// var dataFromServer = JSON.parse(json);
	var data = [
		{ value:"Device 121", id:"start", icon:"wxi-columns" },
		{ value:"Device 231", id:"start1", icon:"wxi-columns" },
		{ value:"Data",		 id:"data",  icon:"wxi-pencil" }
	];
	data = getGroupParamFromJSON(mainDataJsonString,protocol);
	$$("top:menu_acc").clearAll();
	$$("top:menu_acc").define("data",data);
	// $$("top:menu_acc").data.sync(data);
	$$("top:menu_acc").render();
	console.log("getDeviceFromJSON");
	return data;
}
