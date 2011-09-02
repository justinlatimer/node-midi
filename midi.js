var EventEmitter = require('events').EventEmitter;
var midi = require('./build/default/midi_addon');
midi.input.prototype.__proto__ = EventEmitter.prototype;
module.exports = midi;
