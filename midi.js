var EventEmitter = require('events').EventEmitter;
try {
    var midi = require('./build/Release/midi_addon');
} catch(e) {
    try {
        var midi = require('./build/default/midi_addon');
    } catch(e2) {
        console.log(e);
        throw(e2);
    }
}
midi.input.prototype.__proto__ = EventEmitter.prototype;

midi.getPortByName=function(iface,name) {
	var count=iface.getPortCount();
	for (var i=0; i<count; ++i) {
		if (iface.getPortName(i)==name)
			return i;
	}
	return -1;
}

module.exports = midi;
