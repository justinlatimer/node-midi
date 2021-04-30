var midi = require("../../midi.js");

var input = new midi.Input();
console.log('Input ports: ' + input.getPortCount());

for (var i = 0; i < input.getPortCount(); ++i) {
    console.log('Port ' + i + ' name: ' + input.getPortName(i));
}
