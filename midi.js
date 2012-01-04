var EventEmitter = require('events').EventEmitter;
var midi = require('./build/Release/midi_addon');
midi.input.prototype.__proto__ = EventEmitter.prototype;
module.exports = midi;
