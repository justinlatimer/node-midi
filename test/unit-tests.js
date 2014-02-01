var should = require('should');
var EventEmitter = require('events').EventEmitter;
var Midi = require('../midi');

describe('midi.input', function() {
  it('should raise when not called with new', function() {
    (function() {
      Midi.input();
    }).should.throw('Use the new operator to create instances of this object.');
  });

  it('should be an emitter', function() {
    var input = new Midi.input();
    input.should.be.an.instanceOf(EventEmitter);
  });

  describe('.getPortCount', function() {
    var input = new Midi.input();
    it('should return an integer', function() {
      // I feel like having more than 255 ports indicates a problem.
      input.getPortCount().should.be.within(0, 255);
    });
  });

  describe('.getPortName', function() {
    var input = new Midi.input();

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
});

describe('midi.output', function() {
  it('should raise when not called with new', function() {
    (function() {
      Midi.output();
    }).should.throw('Use the new operator to create instances of this object.');
  });

  it('should not be an emitter', function() {
    var output = new Midi.output();
    output.should.not.be.an.instanceOf(EventEmitter);
  });

  describe('.getPortCount', function() {
    var output = new Midi.output();
    it('should return an integer', function() {
      // I feel like having more than 255 ports indicates a problem.
      output.getPortCount().should.be.within(0, 255);
    });
  });

  describe('.getPortName', function() {
    var output = new Midi.output();

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
});
