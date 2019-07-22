# node-midi Changelog

## Version 1.0.0

* Added isPortOpen (nroadley)
* Improve examples in README (Simon Egersand)
* Updated examples to es6 (Amila Welihinda)
* Update mocha (The Repo Nanny)
* Update rtmidi to 4.0.0 (Tim Susa)
* Add license to package.json.
* Use NAN to handle additional differences in modern nodejs versions.
* Change supported nodejs version to 6, 8, 10, 12.
* Better handling of Buffer for stream (jhorology)
* Fixing read stream resume bug (justinjmoses)
* Fix clean up of inputs.
* Exception catching to prevent RtMidi errors crashing the node process (Jeremy Bernstein)
* Split classes into different files.
* Fix capitalisation on the classes.
* Add some documentation about MIDI message formats.

## Version 0.9.5

* Updated RtMidi to most recent version (Szymon Kaliski)
* Updating NAN to the latest version. This allows node 6.2.0 to be used. (Michael Lawrence)

## Version 0.9.4

* Upgrade to nan v2.0 (Julián Duque)
* Call cancelCallback when closing port (Szymon Kaliski)

## Version 0.9.3

* Update NAN version for iojs 2.x support. (Ilkka Myller)

## Version 0.9.2

* More NAN use for broader support (nw.js, iojs). (Andrew Morton)

## Version 0.9.1

* Use NAN to support node 0.8-0.12. (Andrew Morton)

## Version 0.9.0

* Avoid fatal error closing unopened port. (Andrew Morton)
* Upgraded RtMidi to 2.1.0. (Hugo Hromic)
* Fixed compile warnings on Windows. (Hugo Hromic)

## Version 0.8.1

* Fixing crash when `new` is omitted. (Andrew Morton)

## Version 0.8.0

* Update RtMidi to latest upstream. (Andrew Morton)
* Added missing MIDI Clock event case. (Hugo Hromic)
* Upgraded RtMidi library to version 2.0.1. (Hugo Hromic)

## Version 0.7.1

* Remove unmatched uv_unref() causing segfault. (Andrew Morton)

## Version 0.7.0

* Add readable/writable stream support. (Elijah Insua)

## Version 0.6.0

* Upgrade build system to node-gyp bringing Windows support. (Michael Alyn Miller)
* Fix an overzealous delete.

## Version 0.5.0

* Switch from using libev to libuv. (Luc Deschenaux)
* Check a port number is valid before trying to open it. (Luc Deschenaux)
* Remove support for node versions < 0.6.0.
* Code and build system improvements with new supported node versions.
* Update documentation.


## Version 0.4.0

* Upgrade RtMidi to 1.0.15. (Luc Deschenaux)
* Refactor the EventEmitter inheritance to support node > 0.5.2. (Luc Deschenaux)
* Add support for ignore type settings (Sysex, Timing, Active Sensing) on the input. (Luc Deschenaux)
* List supported node versions in the package.json.


## Version 0.3.0

* Add support for virtual input and output ports.


## Version 0.2.0

* Add Linux support to the build script.


## Version 0.1.0

* Initial release.
