var midi = require("../../midi.js");

var output = new midi.output();

console.log('Is open ', output.isPortOpen());
output.openPort(0);
console.log('Is open ', output.isPortOpen());

output.closePort();
