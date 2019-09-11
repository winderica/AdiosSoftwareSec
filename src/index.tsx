import React from 'react';
import { render } from 'react-dom';
import App from './App';
import main from './cpp/build/Debug/main.node';

// expose main for test
window.main = main as Main;

render(
    <App />,
    document.getElementById('root'),
);
