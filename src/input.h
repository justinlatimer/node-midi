#ifndef NODE_MIDI_INPUT_H
#define NODE_MIDI_INPUT_H

#include <napi.h>
#include <queue>

#include "RtMidi.h"

class NodeMidiInput : public Napi::ObjectWrap<NodeMidiInput>
{
private:
    struct MidiMessage
    {
        double deltaTime;
        unsigned char *message;
        size_t messageLength;
    };

    static void CallbackJs(Napi::Env env, Napi::Function callback, nullptr_t *context, MidiMessage *data);
    using TSFN_t = Napi::TypedThreadSafeFunction<nullptr_t, MidiMessage, CallbackJs>;

    std::unique_ptr<RtMidiIn> handle;

    TSFN_t handleMessage;

public:
    static std::unique_ptr<Napi::FunctionReference> Init(const Napi::Env &env, Napi::Object target);

    NodeMidiInput(const Napi::CallbackInfo &info);
    ~NodeMidiInput();

    static void Callback(double deltaTime, std::vector<unsigned char> *message, void *userData);

    Napi::Value GetPortCount(const Napi::CallbackInfo &info);
    Napi::Value GetPortName(const Napi::CallbackInfo &info);

    Napi::Value OpenPort(const Napi::CallbackInfo &info);
    Napi::Value OpenVirtualPort(const Napi::CallbackInfo &info);
    Napi::Value ClosePort(const Napi::CallbackInfo &info);
    Napi::Value IsPortOpen(const Napi::CallbackInfo &info);

    Napi::Value IgnoreTypes(const Napi::CallbackInfo &info);
};

#endif // NODE_MIDI_INPUT_H
