var midi = require('../../midi.js');
var input = new midi.Input();
input.on('message', function(deltaTime, message) {
  console.log('m:' + message + ' d:' + deltaTime);
});

var newInput = function(port) {
  if (input) {
    input.closePort();
  }

  setTimeout(function() {
    console.log('new input', port);
    input.openPort(port);
  }, 100);
};

newInput(0);

setTimeout(function() {
  newInput(0);
}, 5000);

setTimeout(function() {
  input.closePort();
}, 10000);
