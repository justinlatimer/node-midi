var midi = require('../midi.js');
var input;

var newInput = function(port) {
  if (input) {
    input.closePort();
  }

  console.log('new input', port);

  input = new midi.input();

  input.on('message', function(deltaTime, message) {
    console.log('m:' + message + ' d:' + deltaTime);
  });

  input.openPort(port);
};

newInput(0);

setTimeout(function() {
  newInput(0);
}, 5000);

setTimeout(function() {
  input.closePort();
}, 10000);
