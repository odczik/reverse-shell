const http = require('http');
const ws = require('ws');
const fs = require('fs')
require("dotenv").config()

let server = http.createServer((req, res) => {
    fs.readFile("./index.html", (err, html) => {
        if(err) return console.log(err)
        res.writeHead(200, {"Content-Type": "text/html"})
        res.write(html)
        res.end()
    })
});

server.addListener('upgrade', (req) => console.log('UPGRADE:', req.url, "FROM:", `${req.socket.remoteAddress}:${req.socket.remotePort}`));
server.on('error', (err) => console.error(err));
server.listen(process.env.PORT, () => console.log('Https running on port', process.env.PORT));

let clients = {}
const wss = new ws.Server({server, path: '/'});
wss.on('connection', function connection(ws) { 
    console.log('A new connection has been established.');
    ws.send(JSON.stringify({ type: "msg", value: "Message from server" }));

    ws.addEventListener("message", (msg) => {
      msg = JSON.parse(msg.data)
      switch(msg.type){
        case "id":
          if(msg.accessCode !== process.env.ACCESSCODE) return ws.send(JSON.stringify({ type: "accessDenied" }))
          ws.send(JSON.stringify({ type: "accessGranted" }))
          ws.id = msg.value
          clients[ws.id] = ws
          if(clients["web"]) clients["web"].send(JSON.stringify({ type: "connectedClients", value: Object.keys(clients) }))
          break;
        case "connectedClients":
          ws.send(JSON.stringify({ type: "connectedClients", value: Object.keys(clients) }))
          break;
        case "cmd":
          if(clients[msg.to]) clients[msg.to].send(JSON.stringify({ type: "exec", value: msg.value }))
          break;
      }
      if(msg.type !== "connectedClients"){
        console.log(msg)
      }
    })
    
    /*ws.on("ping", (data) => {
      console.log(data.toString())
    });*/
    ws.on("close", (socket) => {
      console.log("A connection has been closed.", socket, ws.id)
      delete clients[ws.id]
       if(clients["web"]) clients["web"].send(JSON.stringify({ type: "connectedClients", value: Object.keys(clients) }))
    })
});

/*const ws = Net.createServer();
ws.listen(8080, function() {
    console.log(`Server listening for connection requests on socket localhost:${port}`);
});

ws.on('connection', function(socket) {
    var remoteAddress = socket.remoteAddress + ':' + socket.remotePort;  
    console.log('A new connection has been established.', remoteAddress);

    socket.write('Hello, client.');

    socket.on('data', function(data) {
        console.log('Client > ', data.toString());
        ask("Send > ", socket)
    });

    socket.on('end', function() {
        console.log('Closing connection with the client');
    });

    socket.on('error', function(err) {
        console.log(`Error: ${err}`);
    });
});*/
