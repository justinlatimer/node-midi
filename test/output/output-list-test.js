var midi = require("../../midi.js");

var output = new midi.Output();
console.log('Output ports: ' + output.getPortCount());

for (var i = 0; i < output.getPortCount(); ++i) {
    console.log('Port ' + i + ' name: ' + output.getPortName(i));
}
