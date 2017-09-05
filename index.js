var http = require("http");
var PythonShell = require("python-shell");
var proc = require("process");

var onURL = 'http://192.168.1.217/on'
var offURL = 'http://192.168.1.217/off'

var options = {
    mode: 'text',
    scriptPath: __dirname + "/PipboyServer/"
  };

var pipboyServer = new PythonShell("pipboy.py", options);
var greenPipboy = __dirname + "/PipboyServer/GreenData";
var yellowPipboy = __dirname + "/PipboyServer/YellowData";
var redPipboy = __dirname + "/PipboyServer/RedData";

var pipboyState = greenPipboy;

function startPipboyServer(){
    pipboyServer.send("start");
    setPipboyState(pipboyState);
}

function setPipboyState(state)
{
    pipboyState = state;
    console.log("setState: "+state);
    pipboyServer.send("load "+state);
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

setInterval(turnOn, 4000);

turnOn();

var read = process.openStdin();
read.addListener("data", function(d){
    console.log("Entered: "+d);
    d = d.toString().trim();
    switch(d)
    {
        case "Green":
            setPipboyState(greenPipboy);
            break;
        case "Yello":
            setPipboyState(yellowPipboy);
            break;
        case "Red":
            setPipboyState(redPipboy);
            break;
    }
})