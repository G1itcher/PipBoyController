var http = require("http");

var onURL = 'http://192.168.1.217/on'

function turnOn(){
    http.get(onURL);
}

setInterval(turnOn, 4000);

turnOn();