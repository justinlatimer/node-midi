var midi = require('bindings')('midi');
var Stream = require('stream');

// MIDI input inherits from EventEmitter
var EventEmitter = require('events').EventEmitter;
midi.Input.prototype.__proto__ = EventEmitter.prototype;

// Backwards compatibility.
midi.input = midi.Input;
midi.output = midi.Output;

module.exports = midi;

midi.createReadStream = function(input) {
  input = input || new midi.Input();
  var stream = new Stream();
  stream.readable = true;
  stream.paused = false;
  stream.queue = [];

  input.on('message', function(deltaTime, message) {

    var packet = new Buffer.from(message);

    if (!stream.paused) {
      stream.emit('data', packet);
    } else {
      stream.queue.push(packet);
    }
  });

  stream.pause = function() {
    stream.paused = true;
  };

  stream.resume = function() {
    stream.paused = false;
    while (stream.queue.length && stream.emit('data', stream.queue.shift())) {}
  };

  return stream;
};


midi.createWriteStream = function(output) {
  output = output || new midi.Output();
  var stream = new Stream();
  stream.writable = true;
  stream.paused = false;
  stream.queue = [];

  stream.write = function(d) {

    if (Buffer.isBuffer(d)) {
      d = Array.prototype.slice.call(d, 0);
    }

    output.sendMessage(d);

    return !this.paused;
  }

  stream.end = function(buf) {
    buf && stream.write(buf);
    stream.writable = false;
  };

  stream.destroy = function() {
    stream.writable = false;
  }

  return stream;
};
