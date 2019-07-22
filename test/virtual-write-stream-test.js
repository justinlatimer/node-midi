var midi = require("../midi.js");
var virtualInput = new midi.Input();
var output = new midi.Output();
var assert = require('assert');
var fs = require('fs');
var expect = [144, 23, 81];
var called = false;
var writer = midi.createWriteStream(output);

virtualInput.openVirtualPort("node-midi");
virtualInput.on('message', function(deltaTime, buffer) {
  assert.deepEqual(buffer, expect);
  called = true;
});


for (var i = 0; i < output.getPortCount(); ++i) {
  if (output.getPortName(i) === 'node-midi') {
    output.openPort(i);
    break;
  }
}

fs.createReadStream(__dirname + '/fixture/144-23-81.bin').pipe(writer);

setTimeout(function() {
  assert(called);
  output.closePort();
  virtualInput.closePort();
  process.exit(0);
}, 100);
