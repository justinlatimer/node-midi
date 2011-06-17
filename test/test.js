var midi = require("../build/default/node-midi.node");

var connection = new midi.connect();
midi.listen(function(data) {
  console.log(data);
});
