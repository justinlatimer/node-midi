var midi = require("../../midi.js");

var input = new midi.Input();

console.log('Is open ', input.isPortOpen());
input.openPort(0);
console.log('Is open ', input.isPortOpen());

input.closePort();
