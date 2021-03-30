#include <nan.h>

#include "input.h"
#include "output.h"

Nan::Persistent<v8::FunctionTemplate> NodeMidiInput::s_ct;
Nan::Persistent<v8::FunctionTemplate> NodeMidiOutput::s_ct;

NAN_MODULE_INIT(InitAll) {
    NodeMidiOutput::Init(target);
    NodeMidiInput::Init(target);
}

NODE_MODULE(midi, InitAll)
