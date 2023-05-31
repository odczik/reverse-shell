const https = require('https');
const ws = require('ws');

let server = https.createServer({}, (req, res) => {
  res.writeHead(200);
  res.end(`hello world\n`);
});
server.addListener('upgrade', (req, res, head) => console.log('UPGRADE:', req.url));
server.on('error', (err) => console.error(err));
server.listen(8080, () => console.log('Https running on port 8080'));

const wss = new ws.Server({server, path: '/echo'});
wss.on('connection', function connection(ws) {
    ws.send('Hello');   
    ws.on('message', (data) => ws.send('Receive: ' + data));
});
