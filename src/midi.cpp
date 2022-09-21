#include <napi.h>

#include "input.h"
#include "output.h"

struct MidiInstanceData
{
    std::unique_ptr<Napi::FunctionReference> output;
    std::unique_ptr<Napi::FunctionReference> input;
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
    auto outputRef = NodeMidiOutput::Init(env, exports);
    auto inputRef = NodeMidiInput::Init(env, exports);

    // Store the constructor as the add-on instance data. This will allow this
    // add-on to support multiple instances of itself running on multiple worker
    // threads, as well as multiple instances of itself running in different
    // contexts on the same thread.
    env.SetInstanceData<MidiInstanceData>(new MidiInstanceData{
        std::move(outputRef),
        std::move(inputRef)});

    return exports;
}

NODE_API_MODULE(midi, InitAll)
