var midi = require('bindings')('midi');
var Stream = require('stream');

// MIDI input inherits from EventEmitter
var EventEmitter = require('events').EventEmitter;
midi.input.prototype.__proto__ = EventEmitter.prototype;

module.exports = midi;

midi.createReadStream = function(input) {
  input = input || new midi.input();
  var stream = new Stream();
  stream.readable = true;
  stream.paused = false;
  stream.queue = [];

  input.on('message', function(deltaTime, message) {

    var packet = new Buffer(message);

    if (!stream.paused) {
      var res = stream.emit('data', packet);
    } else {
      stream.queue.push(packet);
    }

    if (!res) {
      stream.paused = true;
      stream.once('drain', function() {
        while (stream.queue.length && stream.write(queue.shift())) {}
      });
    }
  });

  return stream;
};


midi.createWriteStream = function() {

};
