#ifndef NODE_MIDI_OUTPUT_H
#define NODE_MIDI_OUTPUT_H

#include <napi.h>

#include "RtMidi.h"

class NodeMidiOutput : public Napi::ObjectWrap<NodeMidiOutput>
{
private:
    std::unique_ptr<RtMidiOut> handle;

public:
    static std::unique_ptr<Napi::FunctionReference> Init(const Napi::Env &env, Napi::Object target);

    NodeMidiOutput(const Napi::CallbackInfo &info);
    ~NodeMidiOutput();

    Napi::Value GetPortCount(const Napi::CallbackInfo &info);
    Napi::Value GetPortName(const Napi::CallbackInfo &info);

    Napi::Value OpenPort(const Napi::CallbackInfo &info);
    Napi::Value OpenVirtualPort(const Napi::CallbackInfo &info);
    Napi::Value ClosePort(const Napi::CallbackInfo &info);
    Napi::Value IsPortOpen(const Napi::CallbackInfo &info);

    Napi::Value Send(const Napi::CallbackInfo &info);
};

#endif // NODE_MIDI_OUTPUT_H
