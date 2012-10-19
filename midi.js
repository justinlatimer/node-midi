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
    while (stream.queue.length && stream.write(queue.shift())) {}
  };

  return stream;
};


midi.createWriteStream = function(output) {
  output = output || new midi.output();
  var stream = new Stream();
  stream.writable = true;
  stream.paused = false;
  stream.queue = [];

  stream.write = function(d) {

    if (Buffer.isBuffer(d)) {
      d = [d[0], d[1], d[2]];
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
