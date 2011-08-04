var midi = require("../build/default/midi.node");

var output = new midi.output();
output.openVirtualPort("node-midi Virtual Output");
output.sendMessage([176,22,1]);
output.sendMessage([176,22,99]);
setTimeout(function() {
  output.closePort();
}, 10000);
