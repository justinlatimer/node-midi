#ifndef NODE_MIDI_OUTPUT_H
#define NODE_MIDI_OUTPUT_H

#include <napi.h>
#include <uv.h>

#include "RtMidi.h"

class NodeMidiOutput : public Napi::ObjectWrap<NodeMidiOutput>
{
private:
    RtMidiOut* out;
public:
    static Napi::FunctionReference s_ct;
    static void Init(Napi::Object target);

    NodeMidiOutput();
    ~NodeMidiOutput();

    static Napi::Value New(const Napi::CallbackInfo& info);

    static Napi::Value GetPortCount(const Napi::CallbackInfo& info);
    static Napi::Value GetPortName(const Napi::CallbackInfo& info);

    static Napi::Value OpenPort(const Napi::CallbackInfo& info);
    static Napi::Value OpenVirtualPort(const Napi::CallbackInfo& info);
    static Napi::Value ClosePort(const Napi::CallbackInfo& info);
    static Napi::Value IsPortOpen(const Napi::CallbackInfo& info);

    static Napi::Value Send(const Napi::CallbackInfo& info);
};

#endif // NODE_MIDI_OUTPUT_H
