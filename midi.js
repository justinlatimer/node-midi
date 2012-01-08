var midi = require('./build/Release/midi');

// MIDI input inherits from EventEmitter
var EventEmitter = require('events').EventEmitter;
midi.input.prototype.__proto__ = EventEmitter.prototype;

module.exports = midi;
