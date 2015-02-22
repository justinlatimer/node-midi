♪ ♫ ♩ ♬

# node-midi

A node.js wrapper for the RtMidi C++ library that provides realtime MIDI I/O.
RtMidi supports Linux (ALSA & Jack), Macintosh OS X (CoreMidi), and Windows
(Multimedia).

[![Build Status](https://travis-ci.org/justinlatimer/node-midi.svg)](https://travis-ci.org/justinlatimer/node-midi)

## Prerequisites

### OSX

* Some version of Xcode (or Command Line Tools)
* Python (for node-gyp)

### Windows

* Microsoft Visual C++ (the Express edition works fine)
* Python (for node-gyp)

### Linux

* A C++ compiler
* You must have installed and configured ALSA. Without it this module will **NOT** build.
* Install the libasound2-dev package.
* Python (for node-gyp)

## Installation

Installation uses node-gyp and requires Python 2.7.2 or higher.

From npm:

    $ npm install midi

From source:

    $ git clone https://github.com/justinlatimer/node-midi.git
    $ cd node-midi/
    $ npm install

## Usage

### Input

```js
var midi = require('midi');

// Set up a new input.
var input = new midi.input();

// Count the available input ports.
input.getPortCount();

// Get the name of a specified input port.
input.getPortName(0);

// Configure a callback.
input.on('message', function(deltaTime, message) {
  console.log('m:' + message + ' d:' + deltaTime);
});

// Open the first available input port.
input.openPort(0);

// Sysex, timing, and active sensing messages are ignored
// by default. To enable these message types, pass false for
// the appropriate type in the function below.
// Order: (Sysex, Timing, Active Sensing)
// For example if you want to receive only MIDI Clock beats
// you should use 
// input.ignoreTypes(true, false, true)
input.ignoreTypes(false, false, false);

// ... receive MIDI messages ...

// Close the port when done.
input.closePort();
```

### Output

```js
var midi = require('midi');

// Set up a new output.
var output = new midi.output();

// Count the available output ports.
output.getPortCount();

// Get the name of a specified output port.
output.getPortName(0);

// Open the first available output port.
output.openPort(0);

// Send a MIDI message.
output.sendMessage([176,22,1]);

// Close the port when done.
output.closePort();
```

### Virtual Ports

Instead of opening a connection to an existing MIDI device, on Mac OS X and
Linux with ALSA you can create a virtual device that other software may
connect to. This can be done simply by calling openVirtualPort(portName) instead
of openPort(portNumber).

```js
var midi = require('midi');

// Set up a new input.
var input = new midi.input();

// Configure a callback.
input.on('message', function(deltaTime, message) {
    console.log('m:' + message + ' d:' + deltaTime);
});

// Create a virtual input port.
input.openVirtualPort("Test Input");

// A midi device "Test Input" is now available for other
// software to send messages to.

// ... receive MIDI messages ...

// Close the port when done.
input.closePort();
```

The same can be done with output ports.

### Streams

You can also use this library with streams! Here are the interfaces

#### Readable Stream

```js
// create a readable stream
var stream1 = midi.createReadStream();

// createReadStream also accepts an optional `input` param
var input = new midi.input();
input.openVirtualPort('hello world');

var stream2 = midi.createReadStream(input)

stream2.pipe(require('fs').createWriteStream('something.bin'));
```

#### Writable Stream

```js
// create a writable stream
var stream1 = midi.createWriteStream();

// createWriteStream also accepts an optional `output` param
var output = new midi.output();
output.openVirtualPort('hello again');

var stream2 = midi.createWriteStream(output);

require('fs').createReadStream('something.bin').pipe(stream2);
```

## References

  * http://music.mcgill.ca/~gary/rtmidi/
  * http://syskall.com/how-to-write-your-own-native-nodejs-extension

## Maintainers

  * Justin Latimer - [@justinlatimer](https://github.com/justinlatimer)
  * Elijah Insua - [@tmpvar](https://github.com/tmpvar)
  * Andrew Morton - [@drewish](https://github.com/drewish)

## Contributors

  * Luc Deschenaux - [@luxigo](https://github.com/luxigo)
  * Michael Alyn Miller - [@malyn](https://github.com/malyn)
  * Hugo Hromic - [@hhromic](https://github.com/hhromic)

## License

Copyright (C) 2011 by Justin Latimer.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

A different license may apply to other software included in this package,
including RtMidi. Please consult their respective license files for the
terms of their individual licenses.
