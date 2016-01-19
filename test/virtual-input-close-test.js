var midi = require('../midi.js');
var input;

[ 'a', 'b', 'c', 'd' ].forEach(function(portName) {
  if (input) {
    input.closePort();
  }

  input = new midi.input();
  input.openVirtualPort(portName);
});

var output = new midi.output();

console.log('ports: ' + output.getPortCount());

for (var i = 0; i < output.getPortCount(); ++i) {
  console.log('port [' + i + ']: ' + output.getPortName(i));
}

input.closePort();
