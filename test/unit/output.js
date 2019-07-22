var should = require('should');
var EventEmitter = require('events').EventEmitter;
var Midi = require('../../midi');

describe('midi.Output', function() {
  var output;

  beforeEach(()=>{
    output = new Midi.Output();;
  });

  afterEach(()=>{
     output.closePort();;
  });

  it('should raise when not called with new', function() {
    (function() {
      Midi.Output();
    }).should.throw('Use the new operator to create instances of this object.');
  });

  it('should not be an emitter', function() {
    var output = new Midi.Output();
    output.should.not.be.an.instanceOf(EventEmitter);
  });

  describe('.getPortCount', function() {
    it('should return an integer', function() {
      // I feel like having more than 255 ports indicates a problem.
      output.getPortCount().should.be.within(0, 255);
    });
  });

  describe('.getPortName', function() {
    it('requires an argument', function() {
      (function() {
        output.getPortName();
      }).should.throw('First argument must be an integer');
    });

    it('requires an integer', function() {
      (function() {
        output.getPortName('asdf');
      }).should.throw('First argument must be an integer');
    });

    it('returns an empty string for unknown port', function() {
      output.getPortName(999).should.eql('');
    });
  });

  describe('.openPort', function() {
    it('requires an argument', function() {
      (function() {
        output.openPort();
      }).should.throw('First argument must be an integer');
    });

    it('requires an integer', function() {
      (function() {
        output.openPort('asdf');
      }).should.throw('First argument must be an integer');
    });

    it('requires a valid port', function() {
      (function() {
        output.openPort(999);
      }).should.throw('Invalid MIDI port number');
    });
  });

  describe('.openVirtualPort', function() {
    it('requires an argument', function() {
      (function() {
        output.openVirtualPort();
      }).should.throw('First argument must be a string');
    });

    it('requires a string', function() {
      (function() {
        output.openVirtualPort(999);
      }).should.throw('First argument must be a string');
    });
  });

  describe('.closePort', function() {
    var output = new Midi.Output();

    it('allows you to close a port that was not opened', function() {
      output.closePort();
    });
  });
});
