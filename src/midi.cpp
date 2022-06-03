#include <napi.h>
#include <uv.h>

#include "input.h"
#include "output.h"

Napi::FunctionReference NodeMidiInput::s_ct;
Napi::FunctionReference NodeMidiOutput::s_ct;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    NodeMidiOutput::Init(env, target, module);
    NodeMidiInput::Init(env, target, module);
}

NODE_API_MODULE(midi, InitAll)
