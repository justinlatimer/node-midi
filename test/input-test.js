var midi = require("../build/default/midi.node");

var input = new midi.input();
console.log(input.getPortCount());
console.log(input.getPortName(0));
input.openPort(0);
input.closePort();
