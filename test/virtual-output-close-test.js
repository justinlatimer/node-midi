var midi = require('../midi.js');
var output;

[ 'a', 'b', 'c', 'd' ].forEach(function(portName) {
  if (output) {
    output.closePort();
  }

  output = new midi.output();
  output.openVirtualPort(portName);
});

var input = new midi.input();

console.log('ports: ' + input.getPortCount());

for (var i = 0; i < input.getPortCount(); ++i) {
  console.log('port [' + i + ']: ' + input.getPortName(i));
}
