require("https").createServer().listen(process.env.PORT1);

const Net = require('net');
const port = process.env.PORT || 8080;
const readline = require("readline")

const rl = readline.createInterface({
  input: process.stdin, 
  output: process.stdout,
})
function ask(question, socket) {
    rl.question(question, (answer) => {
        socket.write(answer);
    })
}
const server = Net.createServer();
// The server listens to a socket for a client to make a connection request.
// Think of a socket as an end point.
server.listen(port, function() {
    console.log(`Server listening for connection requests on socket localhost:${port}`);
});

// When a client requests a connection with the server, the server creates a new
// socket dedicated to that client.
server.on('connection', function(socket) {
    var remoteAddress = socket.remoteAddress + ':' + socket.remotePort;  
    console.log('A new connection has been established.', remoteAddress);

    // Now that a TCP connection has been established, the server can send data to
    // the client by writing to its socket.
    socket.write('Hello, client.');

    // The server can also receive data from the client by reading from its socket.
    socket.on('data', function(data) {
        console.log('Client > ', data.toString());
        ask("Send > ", socket)
    });

    // When the client requests to end the TCP connection with the server, the server
    // ends the connection.
    socket.on('end', function() {
        console.log('Closing connection with the client');
    });

    // Don't forget to catch error, for your own sake.
    socket.on('error', function(err) {
        console.log(`Error: ${err}`);
    });
});