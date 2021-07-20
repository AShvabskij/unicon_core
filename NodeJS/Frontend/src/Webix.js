// example of custom component with Webix UI inside
// this one is a static view, not linked to the React data store

import React, { Component } from 'react';
import ReactDOM from 'react-dom';

import * as webix from 'webix/webix.js';
import 'webix/webix.css';

class Webix extends Component {
  render() {
    return (
      <div ref="root"></div>
    );
  }

  setWebixData(data){
    const ui = this.ui;
    if (ui.setValues)
      ui.setValues(data);
    else if (ui.parse) {
      console.log("parse");
      console.log(data);
      ui.parse(data);
    } else if (ui.setValue)
      ui.setValue(data); 
  }

  componentWillUnmount(){
    this.ui.destructor();
    this.ui = null;
  }

  componentWillUpdate(props){
    const ui = this.ui;
    if (props.data)
      this.setWebixData(props.data);
    if (props.select)
      this.select(props.select);
    if (props.click) {
      // console.log("props.click");
      // console.log(props.click)
      // const ui = this.ui;
      ui.onclickMessage = props.click;
    }
      
  }

  componentDidMount(){
  	this.ui = webix.ui(
  	  this.props.ui, 
  	  ReactDOM.findDOMNode(this.refs.root)
	  );

    this.componentWillUpdate(this.props);
  }
  
}

export default Webix;