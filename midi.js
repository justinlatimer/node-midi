const midi = require("pkg-prebuilds/bindings")(
  __dirname,
  require("./binding-options")
);
const Stream = require('stream');

// MIDI input inherits from EventEmitter
const { EventEmitter } = require('events');

class Input extends EventEmitter {
  constructor() {
    super()
    
    this.input = new midi.Input((deltaTime, message) => {
      this.emit('message', deltaTime, Array.from(message.values()))
    })
  }

  closePort() {
    return this.input.closePort()
  }
  getPortCount() {
    return this.input.getPortCount()
  }
  getPortName(port) {
    return this.input.getPortName(port)
  }
  isPortOpen() {
    return this.input.isPortOpen()
  }
  ignoreTypes(sysex, timing, activeSensing) {
    return this.input.ignoreTypes(sysex, timing, activeSensing)
  }
  openPort(port) {
    return this.input.openPort(port)
  }
  openVirtualPort(port) {
    return this.input.openVirtualPort(port)
  }
}

class Output {
  constructor() {
    this.output = new midi.Output()
  }

  closePort() {
    return this.output.closePort()
  }
  getPortCount() {
    return this.output.getPortCount()
  }
  getPortName(port) {
    return this.output.getPortName(port)
  }
  isPortOpen() {
    return this.output.isPortOpen()
  }
  openPort(port) {
    return this.output.openPort(port)
  }
  openVirtualPort(port) {
    return this.output.openVirtualPort(port)
  }
  send(message) {
    return this.sendMessage(message)
  }
  sendMessage(message) {
    if (Array.isArray(message)) {
      message = Buffer.from(message)
    }
    if (!Buffer.isBuffer(message)) {
      throw new Error('First argument must be an array or Buffer')
    }

    return this.output.sendMessage(message)
  }
}


function createReadStream(input) {
  input = input || new Input();
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

function createWriteStream(output) {
  output = output || new Output();
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

module.exports = {
  Input,
  Output,

  createReadStream,
  createWriteStream,
  
  // Backwards compatibility.
  input: Input,
  output: Output,
};
