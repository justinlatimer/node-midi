# node-midi Changelog

## Version 0.7.0

* Add readable/writable stream support. ()

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
