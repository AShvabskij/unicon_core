import React, { useState,useEffect } from "react";
import {Context} from './Context';
import {model} from './Setup'

function demo() {
    console.log("demo");
    console.log(this);
}

function View( {name}) {
    // Объявление переменной состояния, которую мы назовём "count"
    const [count, setCount] = useState(0);
    const [isOnline, setIsOnline] = useState(null);
    const [isLoading, setIsLoading] = useState(null);
    // [Context.count, Context.actions.update] = useState(0);
    //Context.actions.update = setCount;
    
    const increment = () => {
        Context.count = Context.count+1;
        Context.title = "aaa";
        Context.devices = [{name:"qwe"}];
        console.log("Context.count="+Context.count);
        setCount(count + 1)
      }

    useEffect(() => {
        document.title = Context.title+"," + count;
    }, [Context.title]);

    useEffect(() => {
        function handleStatusChange(status) {
            setIsOnline(status.isOnline);
        }

        // ChatAPI.subscribeToFriendStatus(friendID, handleStatusChange);
        return () => {
        // ChatAPI.unsubscribeFromFriendStatus(friendID, handleStatusChange);
        };
    });

    useEffect(() => {
      load()
    }, []);

    function load() {
      setIsLoading(true)

      model.load().then(result => {
        console.log("loadDataModel result:");
        console.log(model.devices());
        setIsLoading(false);
      }, error => {
        console.log("loadDataModel error");
        console.log(error);
      });
    }

    return (
      <div>
        <p>Вы кликнули {count}  - {Context.title} раз</p>
        {/* <button onClick={() => Context.actions.update(count + 5)}> */}
        <button onClick={increment}>
          View Нажми на меня {name} demo
        </button>
      </div>
    );
  }
  
  export default View;