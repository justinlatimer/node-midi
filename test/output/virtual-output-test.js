var midi = require("../../midi.js");

var output = new midi.Output();
output.openVirtualPort("node-midi Virtual Output");
setTimeout(function() {
  output.sendMessage([144,64,90]);
}, 10000);
setTimeout(function() {
  output.sendMessage([128,64,40]);
}, 15000);
setTimeout(function() {
  output.closePort();
}, 20000);
