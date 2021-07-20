// example of custom component with Webix UI inside
// this one is a static view, not linked to the React data store

import React, { Component } from 'react';
import ReactDOM from 'react-dom';
import * as data from './data/data.js';

import * as webix from 'webix/webix.js';
import 'webix/webix.css';

class FilmsView extends Component {
  render() {
    return (
      <div id="data3"  ref="root" style={{height:"100%"}}></div>
    );
  }

  componentDidMount(){
    var comp = this;
    var tree = {
      view:"tree", id:"tree", gravity: 0.6,
      width: 0,
      height: 600,
      select:true,
      on:{ onSelectChange: function(){
        //selected = $$("myTree").getSelectedId()
        // alert("item has just been clicked qw");
        console.log("------------ onSelectChange")
        console.log(this);
        var d = data.grid();
        d.push({ id:9, title:"12 Angry Men 9", year:1957, votes:164558 });
        comp.ui.$$("grid").parse(d);
        }
      }
    };

    var grid = {
      view:"datatable", id:"grid", autoConfig:true,
      scroll:false,
      select:true
    };

    this.ui = webix.ui({
      cols:[
        tree, 
        { view:"resizer" },
        grid
      ],
      isolate:true,

      container:ReactDOM.findDOMNode(this.refs.root)
    });

    this.ui.$$("tree").parse(data.tree());
    // this.ui.$$("grid").parse(data.grid());
  }

  componentWillUnmount(){
    this.ui.destructor();
    this.ui = null;
  }

  shouldComponentUpdate(){
  	// as component is not linked to the external data, there is no need in updates
    return false;
  }

}

export default FilmsView;
