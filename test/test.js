var midi = require("../build/default/midi.node");

var output = new midi.output();
console.log(output.getPortCount());
console.log(output.getPortName(0));
