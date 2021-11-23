import React from "react";
import {inject, observer} from "mobx-react";
import View from './View';
import Button from './Button';
import {Context} from './Context';
import Chart from './Chart';
import ButtonChart from './ButtonChart';
import TimerMobix from './TimerMobix'

function ViewTest(props) {
  return (
   <div>
     <TimerMobix/>
    {Context.chartList.map((chart) => (
            <Chart id={chart.id} className="chart" title="&nbsp;"  />
          ))}
          {/* <Chart id="chart4" className="chart" title="&nbsp;"  />
          <Chart id="chart5" className="chart" title="&nbsp;"  />
          <Chart id="chart6" className="chart" title="&nbsp;"  /> */}
           <ButtonChart name="chart4" />
           <ButtonChart name="chart5" />
           <ButtonChart name="chart6" />
           <Button name="Кнопка 1"/>
           <Button name="Кнопка 2"/>
   </div>
 );
}

export default  ViewTest;