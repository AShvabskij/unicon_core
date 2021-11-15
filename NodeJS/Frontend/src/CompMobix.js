import React from "react"
import ReactDOM from "react-dom"
import { makeAutoObservable } from "mobx"
import { observer } from "mobx-react"
import {Context} from './Context';

class Ctx {
    context = Context;
    constructor() {
        makeAutoObservable(Context)
    }
}
// Model the application state.
class Timer {
    secondsPassed = 0

    constructor() {
        makeAutoObservable(this)
    }

    increase() {
        this.secondsPassed += 1
    }

    reset() {
        this.secondsPassed = 0
    }
}

const myTimer = new Timer()
const ctx1 = new Ctx()
// Build a "user interface" that uses the observable state.
const TimerView1 = observer(({ timer }) => (
    <button onClick={() => timer.reset()}>{Context.title} Seconds passed: {timer.secondsPassed}</button>
));

const TimerView = observer(({ timer }) => (
    <button onClick={() => timer.reset()}>{Context.title} Seconds passed a: {timer.context.title}</button>
));

export default function TimerMobix() {
   return( <TimerView timer={ctx1} />)
};

// ReactDOM.render(<TimerView timer={myTimer} />, document.body)

// Update the 'Seconds passed: X' text every second.
