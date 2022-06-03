#include <napi.h>
#include <uv.h>

#include "RtMidi.h"

#include "output.h"

void NodeMidiOutput::Init(Napi::Object target)
{
    Napi::HandleScope scope(env);

    Napi::FunctionReference t = Napi::Function::New(env, NodeMidiOutput::New);

    s_ct.Reset(t);
    t->SetClassName(Napi::String::New(env, "NodeMidiOutput"));


    InstanceMethod("getPortCount", &GetPortCount),
    InstanceMethod("getPortName", &GetPortName),

    InstanceMethod("openPort", &OpenPort),
    InstanceMethod("openVirtualPort", &OpenVirtualPort),
    InstanceMethod("closePort", &ClosePort),
    InstanceMethod("isPortOpen", &IsPortOpen),

    InstanceMethod("sendMessage", &Send),
    InstanceMethod("send", &Send),

    (target).Set(Napi::String::New(env, "Output"), Napi::GetFunction(t));
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

Napi::Value NodeMidiOutput::New(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);

    if (!info.IsConstructCall()) {
        Napi::TypeError::New(env, "Use the new operator to create instances of this object.").ThrowAsJavaScriptException();
        return env.Null();
    }

    NodeMidiOutput* output = new NodeMidiOutput();
    output->Wrap(info.This());

    return info.This();
}

Napi::Value NodeMidiOutput::GetPortCount(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;
    v8::Local<v8::Integer> result = Napi::Uint32::New(env, output-> out ? output->out->getPortCount() : 0);
    return result;
}

Napi::Value NodeMidiOutput::GetPortName(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;
    if (info.Length() == 0 || !info[0].IsUint32()) {
        Napi::TypeError::New(env, "First argument must be an integer").ThrowAsJavaScriptException();
        return env.Null();
    }

    unsigned int portNumber = Napi::To<unsigned int>(info[0]);
    try {
        Napi::String result = Napi::String::New(env, output->out ? output->out->getPortName(portNumber).c_str() : "");
        return result;
    }
    catch(RtMidiError& e) {
        return Napi::String::New(env, "");
    }
}

Napi::Value NodeMidiOutput::OpenPort(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;

    if (!output->out) return;

    if (info.Length() == 0 || !info[0].IsUint32()) {
        Napi::TypeError::New(env, "First argument must be an integer").ThrowAsJavaScriptException();
        return env.Null();
    }
    unsigned int portNumber = Napi::To<unsigned int>(info[0]);
    if (portNumber >= output->out->getPortCount()) {
        Napi::RangeError::New(env, "Invalid MIDI port number").ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        output->out->openPort(portNumber);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

Napi::Value NodeMidiOutput::OpenVirtualPort(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;

    if (!output->out) return;

    if (info.Length() == 0 || !info[0].IsString()) {
        Napi::TypeError::New(env, "First argument must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string name(info[0].As<Napi::String>().Utf8Value().c_str());

    try {
        output->out->openVirtualPort(name);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

Napi::Value NodeMidiOutput::ClosePort(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;

    if (!output->out) return;

    output->out->closePort();
    return;
}

Napi::Value NodeMidiOutput::IsPortOpen(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;

    if (!output->out) return;

    Napi::Boolean result = Napi::Boolean::New(env, output->out->isPortOpen());
    return result;
}

Napi::Value NodeMidiOutput::Send(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiOutput* output = this;

    if (!output->out) return;

    if (info.Length() == 0 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "First argument must be an array").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object message = info[0].To<Napi::Object>();
    int32_t messageLength = (message).Get(Napi::String::New(env, "length".As<Napi::Number>().Int32Value()));
    std::vector<unsigned char> messageOutput;
    for (int32_t i = 0; i != messageLength; ++i) {
        messageOutput.push_back(Napi::To<unsigned int>((message).Get(Napi::Number::New(env, i))));
    }
    try {
        output->out->sendMessage(&messageOutput);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}
