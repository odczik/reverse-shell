const http = require('http');
const ws = require('ws');

let server = http.createServer((req, res) => {
  res.writeHead(200);
  res.end(`hello world\n`);
});
server.addListener('upgrade', (req, res, head) => console.log('UPGRADE:', req.url));
server.on('error', (err) => console.error(err));
server.listen(8080, () => console.log('Https running on port 8080'));

const wss = new ws.Server({server, path: '/'});
wss.on('connection', function connection(ws) { 
    console.log('A new connection has been established.', ws.url);

    ws.send('Hello');   
    ws.on('message', (data) => ws.send('Receive: ' + data));
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
