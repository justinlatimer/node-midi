var should = require('should');
var EventEmitter = require('events').EventEmitter;
var Midi = require('../../midi');
describe('midi.Input', function() {

  var input;
  beforeEach(()=>{
    input = new Midi.Input();;
  })

  afterEach(()=>{
    input.closePort();
  })

  it('should raise when not called with new', function() {
    (function() {
      Midi.Input();
    }).should.throw('Use the new operator to create instances of this object.');
  });
  
  it('should be an emitter', function() {
    input.should.be.an.instanceOf(EventEmitter);
  });


  describe('.getPortCount', function() {
    it('.getPortCoun should return an integer', function() {
      // I feel like having more than 255 ports indicates a problem.
      input.getPortCount().should.be.within(0, 255);
    });
  });

  describe('.getPortName', function() {
    it('requires an argument', function() {
      (function() {
        input.getPortName();
      }).should.throw('First argument must be an integer');
    });

    it('requires an integer', function() {
      (function() {
        input.getPortName('asdf');
      }).should.throw('First argument must be an integer');
    });

    it('returns an empty string for unknown port', function() {
      input.getPortName(999).should.eql('');
    });
  });


  describe('.openPort', function() {
    it('requires an argument', function() {
      (function() {
        input.openPort();
      }).should.throw('First argument must be an integer');
    });

    it('requires an integer', function() {
      (function() {
        input.openPort('asdf');
      }).should.throw('First argument must be an integer');
    });

    it('requires a valid port', function() {
      (function() {
        input.openPort(999);
      }).should.throw('Invalid MIDI port number');
    });
  });


  describe('.openVirtualPort', function() {
    it('requires an argument', function() {
      (function() {
        input.openVirtualPort();
      }).should.throw('First argument must be a string');
    });

    it('requires a string', function() {
      (function() {
        input.openVirtualPort(999);
      }).should.throw('First argument must be a string');
    });
  });

});
