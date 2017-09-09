var http = require("http");
var PythonShell = require("python-shell");
var proc = require("process");

var onURL = 'http://192.168.1.217/on'
var offURL = 'http://192.168.1.217/off'

var onInterval;

var options = {
    mode: 'text',
    scriptPath: __dirname + "/PipboyServer/"
  };

var pipboyServer = new PythonShell("pipboy.py", options);
var greenPipboy = __dirname + "/PipboyServer/GreenData";
var yellowPipboy = __dirname + "/PipboyServer/YellowData";
var redPipboy = __dirname + "/PipboyServer/RedData";

var colours = {
    Red: redPipboy,
    Green: greenPipboy,
    Yellow: yellowPipboy
}

var pipboyState = greenPipboy;

function startPipboyServer(){
    pipboyServer.send("start");
    setPipboyState(pipboyState);
}

function setPipboyColour(colour)
{
    var colourFile = colours[colour]
    if(colourFile){
        console.log("setState: "+colourFile);
        pipboyServer.send("load "+colourFile);
    }
}

startPipboyServer();

function turnOn(){
    try{
        http.get(onURL);
    }
    finally{
        // Nothing to do.
    }
}

function stayOn(){
    onInterval = setInterval(turnOn, 4000);
}

function turnOff(){
    try{
        http.get(offURL);
    }
    finally{
        // Nothing to do.
    }
}

function stayOff(){
    clearInterval(onInterval);
    turnOff();
}

setInterval(turnOn, 4000);

turnOn();

var coloursMap = {};

for(var colour in colours){
    coloursMap[colour] = colour;
}

module.exports = {
    start: function(){
        stayOn();
    },
    stop = function(){
        stayOff();
    },
    on: function(){
        turnOn();
    },
    off: function(){
        turnOff();
    },
    setColour: function(colour){
        setPipboyColour(colour);
    },
    Colours: coloursMap 
}