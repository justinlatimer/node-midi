#include <nan.h>

#include "lib/RtMidi/RtMidi.h"

#include "output.h"

void NodeMidiOutput::Init(v8::Local<v8::Object> target)
{
    Nan::HandleScope scope;

    v8::Local<v8::FunctionTemplate> t = Nan::New<v8::FunctionTemplate>(NodeMidiOutput::New);

    s_ct.Reset(t);
    t->SetClassName(Nan::New<v8::String>("NodeMidiOutput").ToLocalChecked());
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(t, "getPortCount", GetPortCount);
    Nan::SetPrototypeMethod(t, "getPortName", GetPortName);

    Nan::SetPrototypeMethod(t, "openPort", OpenPort);
    Nan::SetPrototypeMethod(t, "openVirtualPort", OpenVirtualPort);
    Nan::SetPrototypeMethod(t, "closePort", ClosePort);
    Nan::SetPrototypeMethod(t, "isPortOpen", IsPortOpen);

    Nan::SetPrototypeMethod(t, "sendMessage", SendMessage);

    Nan::Set(target, Nan::New<v8::String>("Output").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
}

NodeMidiOutput::NodeMidiOutput()
{
    try {
        out = new RtMidiOut();
    }
    catch(RtMidiError &e) {
        out = NULL;
    }
}

NodeMidiOutput::~NodeMidiOutput()
{
    if (out) {
        delete out;
    }
}

NAN_METHOD(NodeMidiOutput::New)
{
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowTypeError("Use the new operator to create instances of this object.");
    }

    NodeMidiOutput* output = new NodeMidiOutput();
    output->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NodeMidiOutput::GetPortCount)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
    v8::Local<v8::Integer> result = Nan::New<v8::Uint32>(output-> out ? output->out->getPortCount() : 0);
    info.GetReturnValue().Set(result);
}

NAN_METHOD(NodeMidiOutput::GetPortName)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
    if (info.Length() == 0 || !info[0]->IsUint32()) {
        return Nan::ThrowTypeError("First argument must be an integer");
    }

    unsigned int portNumber = Nan::To<unsigned int>(info[0]).FromJust();
    try {
        v8::Local<v8::String> result = Nan::New<v8::String>(output->out ? output->out->getPortName(portNumber).c_str() : "").ToLocalChecked();
        info.GetReturnValue().Set(result);
    }
    catch(RtMidiError& e) {
        info.GetReturnValue().Set(Nan::New<v8::String>("").ToLocalChecked());
    }
}

NAN_METHOD(NodeMidiOutput::OpenPort)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

    if (!output->out) return;

    if (info.Length() == 0 || !info[0]->IsUint32()) {
        return Nan::ThrowTypeError("First argument must be an integer");
    }
    unsigned int portNumber = Nan::To<unsigned int>(info[0]).FromJust();
    if (portNumber >= output->out->getPortCount()) {
        return Nan::ThrowRangeError("Invalid MIDI port number");
    }

    try {
        output->out->openPort(portNumber);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

NAN_METHOD(NodeMidiOutput::OpenVirtualPort)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

    if (!output->out) return;

    if (info.Length() == 0 || !info[0]->IsString()) {
        return Nan::ThrowTypeError("First argument must be a string");
    }

    std::string name(*Nan::Utf8String(info[0]));

    try {
        output->out->openVirtualPort(name);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

NAN_METHOD(NodeMidiOutput::ClosePort)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

    if (!output->out) return;

    output->out->closePort();
    return;
}

NAN_METHOD(NodeMidiOutput::IsPortOpen)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

    if (!output->out) return;

    v8::Local<v8::Boolean> result = Nan::New<v8::Boolean>(output->out->isPortOpen());
    info.GetReturnValue().Set(result);
}

NAN_METHOD(NodeMidiOutput::SendMessage)
{
    Nan::HandleScope scope;
    NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

    if (!output->out) return;

    if (info.Length() == 0 || !info[0]->IsArray()) {
        return Nan::ThrowTypeError("First argument must be an array");
    }

    v8::Local<v8::Object> message = Nan::To<v8::Object>(info[0]).ToLocalChecked();
    int32_t messageLength = Nan::To<int32_t>(Nan::Get(message, Nan::New<v8::String>("length").ToLocalChecked()).ToLocalChecked()).FromJust();
    std::vector<unsigned char> messageOutput;
    for (int32_t i = 0; i != messageLength; ++i) {
        messageOutput.push_back(Nan::To<unsigned int>(Nan::Get(message, Nan::New<v8::Integer>(i)).ToLocalChecked()).FromJust());
    }
    try {
        output->out->sendMessage(&messageOutput);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}
