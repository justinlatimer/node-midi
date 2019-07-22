var midi = require("../midi.js");

var outputName = "node-midi Virtual Output";
var inputName = "node-midi Virtual Input";

var virtualOutput = new midi.Output();
var virtualInput = new midi.Input();

virtualOutput.openVirtualPort(outputName);
virtualInput.on('message', function(deltaTime, message) {
  console.log('Virtual input recieved m:' + message + ' d:' + deltaTime);
  virtualOutput.sendMessage([
    message[0],
    message[1] + 10,
    message[2]
  ]);
});
virtualInput.openVirtualPort(inputName);

setTimeout(function() {
  var output = new midi.Output();
  var input = new midi.Input();

  input.on('message', function(deltaTime, message) {
    console.log('Input recieved m:' + message + ' d:' + deltaTime);
  });

  console.log('Enumerating inputs');
  for (var i = 0; i < input.getPortCount(); ++i) {
    console.log('Input found: ' + input.getPortName(i));
    if (input.getPortName(i) == outputName) {
      console.log('Opening ' + input.getPortName(i));
      input.openPort(i);
    }
  }

  console.log('Enumerating outputs');
  for (var i = 0; i < output.getPortCount(); ++i) {
    console.log('Output found: ' + input.getPortName(i));
    if (output.getPortName(i) == inputName) {
      console.log('Opening ' + output.getPortName(i));
      output.openPort(i);
    }
  }

  var id = setInterval(function() {
    console.log('Sending message');
    output.sendMessage([144, 23, 81]);
  }, 1000);

  setTimeout(function() {
    clearInterval(id);

    setTimeout(function() {
      input.closePort();
      output.closePort();
      virtualInput.closePort();
      virtualOutput.closePort();
    }, 100);
  }, 10000);
}, 500);
