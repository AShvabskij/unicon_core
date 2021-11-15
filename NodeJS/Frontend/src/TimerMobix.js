import React,{useEffect} from "react"
import ReactDOM from "react-dom"
import { makeAutoObservable } from "mobx"
import { observer } from "mobx-react"
import {Context} from './Context';

const TimerView = observer(({  }) => {
    useEffect(() => {
        console.log("Render TimerView");
    })
    return (
    <button >{Context.title} Seconds passed abc: {Context.title}</button>
    )
});

// @observer
// function TimerView1() {
//    return  <button >{Context.title} Seconds passed abc: {Context.title}</button>
// }

export default function TimerMobix() {
    useEffect(() => {
        console.log("Render TimerMobix");
    })
   return( <TimerView  />)
};



// ReactDOM.render(<TimerView timer={myTimer} />, document.body)

// Update the 'Seconds passed: X' text every second.
// setInterval(() => {
//     myTimer.increase()
// }, 1000)