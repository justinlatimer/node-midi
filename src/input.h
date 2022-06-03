#ifndef NODE_MIDI_INPUT_H
#define NODE_MIDI_INPUT_H

#include <napi.h>
#include <uv.h>
#include <queue>
#include <uv.h>

#include "RtMidi.h"

class NodeMidiInput : public Napi::ObjectWrap<NodeMidiInput>
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

    static Napi::FunctionReference s_ct;
    static void Init(Napi::Object target);

    NodeMidiInput();
    ~NodeMidiInput();
    void cleanUp();

    static NAUV_WORK_CB(EmitMessage);
    static void Callback(double deltaTime, std::vector<unsigned char> *message, void *userData);

    static Napi::Value New(const Napi::CallbackInfo& info);

    static Napi::Value GetPortCount(const Napi::CallbackInfo& info);
    static Napi::Value GetPortName(const Napi::CallbackInfo& info);

    static Napi::Value OpenPort(const Napi::CallbackInfo& info);
    static Napi::Value OpenVirtualPort(const Napi::CallbackInfo& info);
    static Napi::Value ClosePort(const Napi::CallbackInfo& info);
    static Napi::Value IsPortOpen(const Napi::CallbackInfo& info);

    static Napi::Value IgnoreTypes(const Napi::CallbackInfo& info);
};

#endif // NODE_MIDI_INPUT_H
