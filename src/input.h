#ifndef NODE_MIDI_INPUT_H
#define NODE_MIDI_INPUT_H

#include <nan.h>
#include <queue>
#include <uv.h>

#include "lib/RtMidi/RtMidi.h"

class NodeMidiInput : public Nan::ObjectWrap
{
private:
    RtMidiIn* in;
    bool configured;

public:
    uv_async_t message_async;
    uv_mutex_t message_mutex;

    struct MidiMessage
    {
        double deltaTime;
        std::vector<unsigned char> message;
    };
    std::queue<MidiMessage*> message_queue;

    static Nan::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Local<v8::Object> target);

    NodeMidiInput();
    ~NodeMidiInput();
    void cleanUp();

    static NAUV_WORK_CB(EmitMessage);
    static void Callback(double deltaTime, std::vector<unsigned char> *message, void *userData);

    static NAN_METHOD(New);

    static NAN_METHOD(GetPortCount);
    static NAN_METHOD(GetPortName);

    static NAN_METHOD(OpenPort);
    static NAN_METHOD(OpenVirtualPort);
    static NAN_METHOD(ClosePort);
    static NAN_METHOD(IsPortOpen);

    static NAN_METHOD(IgnoreTypes);
};

#endif // NODE_MIDI_INPUT_H
