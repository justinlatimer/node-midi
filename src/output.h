#ifndef NODE_MIDI_OUTPUT_H
#define NODE_MIDI_OUTPUT_H

#include <nan.h>

#include "lib/RtMidi/RtMidi.h"

class NodeMidiOutput : public Nan::ObjectWrap
{
private:
    RtMidiOut* out;
public:
    static Nan::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Local<v8::Object> target);

    NodeMidiOutput();
    ~NodeMidiOutput();

    static NAN_METHOD(New);

    static NAN_METHOD(GetPortCount);
    static NAN_METHOD(GetPortName);

    static NAN_METHOD(OpenPort);
    static NAN_METHOD(OpenVirtualPort);
    static NAN_METHOD(ClosePort);
    static NAN_METHOD(IsPortOpen);

    static NAN_METHOD(SendMessage);
};

#endif // NODE_MIDI_OUTPUT_H
