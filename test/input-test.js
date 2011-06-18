var midi = require("../build/default/midi.node");

var input = new midi.input();
console.log(input.getPortCount());
console.log(input.getPortName(0));
input.on('message', function(deltaTime, message) {
  console.log('m:' + message + ' d:' + deltaTime);
});
input.openPort(0);
setTimeout(function() {
  input.closePort();
}, 10000);