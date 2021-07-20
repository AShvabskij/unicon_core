export default {
	cols: [
		{template:"<iframe id='sci' src='http://localhost:8080/'></iframe>", css:"webix_shadow_medium app_start"},
        {	width: 430,
					padding:100,
                    margin:30,
					rows: [
						{view: "text", value: "100", label: "Timescale", labelWidth:100},
						{label: "Switch Monitoring", view: "switch", labelWidth:130},
						{view: "button", value: "Add chart"},
						{view: "button", value: "Remove chart"}
					]
				}

    ]
};
