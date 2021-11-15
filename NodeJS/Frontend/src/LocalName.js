import React, { useState,useEffect } from "react";
import {Context} from './Context';

function LocalName() {
  // аналогично useState, но первым аргументом является ключ значения, хранящегося в локальном хранилище
  const [name, setName] = useLocalStorage("name", "Igor");

  return (
    <div>
      <input
        type="text"
        placeholder="Enter your name"
        value={name}
        onChange={(e) => setName(e.target.value)}
      />
    </div>
  );
}

// хук
function useLocalStorage(key, initialValue) {
  // состояние для хранения значения
  // передаем функцию инициализации useState для однократного выполнения
  const [storedValue, setStoredValue] = useState(() => {
    try {
      // получаем значение из локального хранилища по ключу
      const item = window.localStorage.getItem(key);
      // разбираем полученное значение или возвращаем initialValue
      return item ? JSON.parse(item) : initialValue;
    } catch (error) {
      // если возникла ошибка, также возвращаем начальное значение
      console.error(error);
      return initialValue;
    }
  });

  // возвращаем обернутую версию сеттера useState,
  // которая помещает новое значение в локальное хранилище
  const setValue = (value) => {
    try {
      // значение может быть функцией
      const valueToStore =
        value instanceof Function ? value(storedValue) : value;
      // сохраняем состояние
      setStoredValue(valueToStore);
      // помещаем его в локальное хранилище
      window.localStorage.setItem(key, JSON.stringify(valueToStore));
    } catch (error) {
      // более продвинутая реализация может предполагать обработку ошибок в зависимости от вида ошибки
      console.error(error);
    }
  };

  return [storedValue, setValue];
}
  export default LocalName;