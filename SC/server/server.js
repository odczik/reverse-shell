const http = require('http');
const ws = require('ws');
const fs = require('fs')
require("dotenv").config()

let server = http.createServer((req, res) => {
    switch(req.url){
        case "/":
            fs.readFile("./index.html", (err, html) => {
                if(err) return console.log(err)
                res.writeHead(200, {"Content-Type": "text/html"})
                res.write(html)
                res.end()
            })
            break;
        case "/killswitch":
            fs.readFile("./killswitch", (err, html) => {
                if(err) return console.log(err)
                res.writeHead(200, {"Content-Type": "text/html"})
                res.write(html)
                res.end()
            })
            break;
    }
    
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
            case "ksw":
                let data = fs.readFileSync("./killswitch", "utf8")
                
                if(msg.check){
                    ws.send(JSON.stringify({type: "ksw", ksw: data}))
                } else {
                    if(data == 1){
                        fs.writeFile('./killswitch', '0', (err) => {
                            if(err) console.log(err)
                        });
                    } else {
                        fs.writeFileSync('./killswitch', '1', (err) => {
                            if(err) console.log(err)
                        });
                    }
                    fs.readFile("./killswitch", "utf8", (err, data) => {
                        if(err) console.log(err)
                        console.log(data)
                        ws.send(JSON.stringify({type: "ksw", ksw: data}))
                    })
                }
                break;
        }
        if(msg.type !== "connectedClients"){
            console.log(msg)
        }
    })
    ws.on("close", (socket) => {
        console.log("A connection has been closed.", socket, ws.id)
        delete clients[ws.id]
       if(clients["web"]) clients["web"].send(JSON.stringify({ type: "connectedClients", value: Object.keys(clients) }))
    })
});
