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
```bash
$ npm install midi
```

From source:
```bash
$ git clone https://github.com/justinlatimer/node-midi.git
$ cd node-midi/
$ npm install
```

## Usage

### MIDI Messages

This library deals with MIDI messages as JS Arrays for both input and output. For example, `[144,69,127]` is MIDI message with status code 144 which means "Note on" on "Channel 1".

For list of midi status codes, see http://www.midi.org/techspecs/midimessages.php

### Input

```js
const midi = require('midi');

// Set up a new input.
const input = new midi.Input();

// Count the available input ports.
input.getPortCount();

// Get the name of a specified input port.
input.getPortName(0);

// Configure a callback.
input.on('message', (deltaTime, message) => {
  // The message is an array of numbers corresponding to the MIDI bytes:
  //   [status, data1, data2]
  // https://www.cs.cf.ac.uk/Dave/Multimedia/node158.html has some helpful
  // information interpreting the messages.
  console.log(`m: ${message} d: ${deltaTime}`);
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
setTimeout(function() {
  input.closePort();
}, 100000);
```

### Output

```js
const midi = require('midi');

// Set up a new output.
const output = new midi.Output();

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
const midi = require('midi');

// Set up a new input.
const input = new midi.Input();

// Configure a callback.
input.on('message', (deltaTime, message) => {
    console.log(`m: ${message} d: ${deltaTime}`);
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
const stream1 = midi.createReadStream();

// createReadStream also accepts an optional `input` param
const input = new midi.Input();
input.openVirtualPort('hello world');

const stream2 = midi.createReadStream(input)

stream2.pipe(require('fs').createWriteStream('something.bin'));
```

#### Writable Stream

```js
// create a writable stream
const stream1 = midi.createWriteStream();

// createWriteStream also accepts an optional `output` param
const output = new midi.Output();
output.openVirtualPort('hello again');

const stream2 = midi.createWriteStream(output);

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
