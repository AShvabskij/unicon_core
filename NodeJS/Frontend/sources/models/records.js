export const data = new webix.DataCollection({ data:[
	{ id:1, title:"Divise 1", text:"Request timeout for icmp_seq 4", state:"ON"},
	{ id:2, title:"Divise 2", text:"0 packets received", state:"OFF"},
	{ id:3, title:"Divise 1", text:"Request timeout for icmp_seq 5", state:"ON"},
	{ id:4, title:"Divise 3", text:"Request timeout for icmp_seq 400", state:"ON"},
	{ id:5, title:"Divise 1", text:"Request timeout for icmp_seq 6", state:"ON"},
	{ id:6, title:"Divise 1 / UPS", text:"220 W", state:"ON"}
]});

export const device1 = new webix.DataCollection({data:[
	{ id:9, num: "1", name:"Parameter 1 (2110)", value:"1.008", dimension:"W", time:"11:56",chart:"+",numchart:1},
	{ id:10, num: "2", name:"Parameter 2 (2120)", value:"2.7896", dimension:"A", time:"11:56",chart:"+",numchart:1},
	{ id:11, num: "3", name:"Parameter 3 (2130)", value:"8", dimension:"kHz", time:"11:00",chart:"+",numchart:"2"},
	{ id:7, num: "4", name:"Parameter 4 (2140)", value:"356", dimension:"NO/NC", time:"11:20",chart:"–",numchart:""}
]});

export const device2 = new webix.DataCollection({data:[
	{ id:1, title:"The Shawshank Redemption 123", year:1994, votes:678790, rating:9.2, rank:1},
	{ id:5, title:"My Fair Lady", year:1964, votes:533848, rating:8.9, rank:5},
	{ id:8, title:"Victory", year:1957, votes:1645560, rating:8.9, rank:6}
]});

export const data_devise2 = new webix.DataCollection({data:[
	{ id:1, title:"Схема устройства 2", year:1994, votes:678790, rating:9.2, rank:1}
]});


// export const data1 = new webix.DataCollection({
// 	url:"http://127.0.0.1/settings/data.json"
// });