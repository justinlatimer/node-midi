var midi = require("../build/default/midi.node");

var output = new midi.output();
console.log(output.getPortCount());
console.log(output.getPortName(0));
output.openPort(0);
output.sendMessage([176,22,1]);
output.sendMessage([176,22,99]);
output.closePort();

