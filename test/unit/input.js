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


  describe(".on('message')", function() {
    it('allows promises to resolve', async function() {
      const portName = 'node-midi Virtual Loopback';

      // Create a promise we can use to pass/fail the whole test.
      let resolveTestPromise, rejectTestPromise;
      const testPromise = new Promise((resolve, reject) => {
        resolveTestPromise = resolve;
        rejectTestPromise = reject;
      });

      // Create a promise to resolve in the on('message') callback.
      let resolvePendingPromise, rejectPendingPromise;
      const promise = new Promise((resolve, reject) => {
        resolvePendingPromise = resolve;
        rejectPendingPromise = reject;
      });

      // Create a virtual loopback MIDI port.
      var input = new Midi.Input();
      input.on('message', function(deltaTime, message) {
        // Resolve the promise now we have received a MIDI message.
        resolvePendingPromise(message);
      });
      input.openVirtualPort(portName);

      let awaitComplete = false;
      setTimeout(function() {
        input.closePort();
        if (!awaitComplete) {
          // If the `await` statement below has not yet returned, then this is
          // a failure.
          rejectTestPromise(new Error('Await did not return in time'));
        }
      }, 1500); // Must be under 2000 or Mocha fails us for being too slow.

      // Find the other end of the virtual MIDI port we created above.
      const output = new Midi.Output();
      for (var i = 0; i < output.getPortCount(); ++i) {
        if (output.getPortName(i).includes(portName)) {
          output.openPort(i);
        }
      }
      if (!output.isPortOpen()) {
        throw Error('Unable to find virtual loopback port');
      }

      // Send a message, which should end up calling the on('message') handler.
      output.sendMessage([176, 22, 1]);
      output.closePort();

      // Wait for the promise resolved by on('message').  If everything is
      // working, this promise has already been resolved and will complete
      // immediately.  However if the event loop is starved, it will block
      // until the next event arrives, resulting in a timeout.
      const result = await promise;
      awaitComplete = true;

      // Resolve the promise for this test, which will have no effect if the
      // promise has already been rejected because the `await` took too long.
      resolveTestPromise();

      // Wait for the test promise, which will cause an exception to be thrown
      // if the test promise was rejected.
      await testPromise;
    });
  });
});
