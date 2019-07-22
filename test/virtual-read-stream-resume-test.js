var midi = require("../midi.js");
var virtualInput = new midi.Input();
var output = new midi.Output();
var assert = require('assert');
var payload = [144, 23, 81];
var called = false;

virtualInput.openVirtualPort("node-midi");

var reader = midi.createReadStream(virtualInput);
reader.pipe(process.stdout);
reader.on('data', function(buffer) {
  assert.deepEqual(buffer, new Buffer.from(payload));
  called = true;
});

for (var i = 0; i < output.getPortCount(); ++i) {
  if (output.getPortName(i) === 'node-midi') {
    output.openPort(i);
    break;
  }
}
reader.pause()
output.sendMessage(payload);

setTimeout(function() { reader.resume(); }, 10)

setTimeout(function() {
  assert(called);
  process.exit(0);
}, 100);
