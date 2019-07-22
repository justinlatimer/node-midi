#include <nan.h>

#include "input.h"
#include "output.h"

Nan::Persistent<v8::FunctionTemplate> NodeMidiInput::s_ct;
Nan::Persistent<v8::FunctionTemplate> NodeMidiOutput::s_ct;

extern "C" {
    void init (v8::Local<v8::Object> target)
    {
        NodeMidiOutput::Init(target);
        NodeMidiInput::Init(target);
    }
    NODE_MODULE(midi, init)
}
