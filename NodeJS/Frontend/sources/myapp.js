import "./styles/app.css";
import {JetApp, EmptyRouter, HashRouter } from "webix-jet";

export default class MyApp extends JetApp{
	constructor(config){
		const defaults = {
			id 		: APPNAME,
			version : VERSION,
			router 	: BUILD_AS_MODULE ? EmptyRouter : HashRouter,
			// debug 	: !PRODUCTION,
			debug 	: true,
			name 	: "UNICON",
			start 	: "/top/start/start1"
		};

		super({ ...defaults, ...config });
	}
}

if (!BUILD_AS_MODULE){
	webix.protoUI({
		name:"top:tree"
	}, webix.EditAbility, webix.ui.tree);
	webix.ready(() => new MyApp().render() );
}