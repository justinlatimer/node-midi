// Node version 0.4.0 and 0.6.0 compatibility
try {
  var midi = require('./build/Release/midi_addon');
} catch(e) { try {
  var midi = require('./build/default/midi_addon');
} catch(e) {
  throw e;
}}

// MIDI input inherits from EventEmitter
var EventEmitter = require('events').EventEmitter;
midi.input.prototype.__proto__ = EventEmitter.prototype;

module.exports = midi;
