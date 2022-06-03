#include <napi.h>
#include <uv.h>
#include <queue>
#include <uv.h>

#include "RtMidi.h"

#include "input.h"

const char* symbol_emit = "emit";
const char* symbol_message = "message";

void NodeMidiInput::Init(Napi::Object target)
{
    Napi::HandleScope scope(env);

    Napi::FunctionReference t = Napi::Function::New(env, NodeMidiInput::New);

    s_ct.Reset(t);
    t->SetClassName(Napi::String::New(env, "NodeMidiInput"));


    InstanceMethod("getPortCount", &GetPortCount),
    InstanceMethod("getPortName", &GetPortName),

    InstanceMethod("openPort", &OpenPort),
    InstanceMethod("openVirtualPort", &OpenVirtualPort),
    InstanceMethod("closePort", &ClosePort),
    InstanceMethod("isPortOpen", &IsPortOpen),

    InstanceMethod("ignoreTypes", &IgnoreTypes),

    (target).Set(Napi::String::New(env, "Input"), Napi::GetFunction(t));
}

NodeMidiInput::NodeMidiInput()
{
    try {
        in = new RtMidiIn();
    }
    catch(RtMidiError& e) {
        in = NULL;
    }
    configured = false;
    uv_mutex_init(&message_mutex);
}

NodeMidiInput::~NodeMidiInput()
{
    if (in) {
        cleanUp();
        delete in;
    }
    uv_mutex_destroy(&message_mutex);
}

void NodeMidiInput::cleanUp()
{
    if (this->configured) {
        this->Unref();
        try {
            this->in->cancelCallback();
            this->in->closePort();
        }
        catch(RtMidiError& e) {
            ;
        }
        uv_close((uv_handle_t*)&this->message_async, NULL);
        this->configured = false;
    }
}

NAUV_WORK_CB(NodeMidiInput::EmitMessage)
{
    Napi::HandleScope scope(env);
    NodeMidiInput *input = static_cast<NodeMidiInput*>(async->data);
    uv_mutex_lock(&input->message_mutex);
    Napi::Function emitFunction = (input->handle()).Get(Napi::String>(symbol_emit)).As<Napi::Function::New(env);
    while (!input->message_queue.empty())
    {
        MidiMessage* message = input->message_queue.front();
        Napi::Value info[3];
        info[0] = Napi::String::New(env, symbol_message);
        info[1] = Napi::Number::New(env, message->deltaTime);
        int32_t count = (int32_t)message->message.size();
        Napi::Array data = Napi::Array::New(env, count);
        for (int32_t i = 0; i < count; ++i) {
            (data).Set(Napi::Number::New(env, i), Napi::Number::New(env, message->message[i]));
        }
        info[2] = data;
        Napi::FunctionReference callback_emit(emitFunction);
        callback_emit.Call(input->handle(), 3, info);
        input->message_queue.pop();
        delete message;
    }
    uv_mutex_unlock(&input->message_mutex);
}

void NodeMidiInput::Callback(double deltaTime, std::vector<unsigned char> *message, void *userData)
{
    NodeMidiInput *input = static_cast<NodeMidiInput*>(userData);
    MidiMessage* data = new MidiMessage();
    data->deltaTime = deltaTime;
    data->message = *message;
    uv_mutex_lock(&input->message_mutex);
    input->message_queue.push(data);
    uv_mutex_unlock(&input->message_mutex);
    uv_async_send(&input->message_async);
}

Napi::Value NodeMidiInput::New(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);

    if (!info.IsConstructCall()) {
        Napi::TypeError::New(env, "Use the new operator to create instances of this object.").ThrowAsJavaScriptException();
        return env.Null();
    }

    NodeMidiInput* input = new NodeMidiInput();
    input->message_async.data = input;
    input->Wrap(info.This());

    return info.This();
}

Napi::Value NodeMidiInput::GetPortCount(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;
    v8::Local<v8::Integer> result = Napi::Uint32::New(env, input->in ? input->in->getPortCount() : 0);
    return result;
}

Napi::Value NodeMidiInput::GetPortName(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;
    if (info.Length() == 0 || !info[0].IsUint32()) {
        Napi::TypeError::New(env, "First argument must be an integer").ThrowAsJavaScriptException();
        return env.Null();
    }

    unsigned int portNumber = Napi::To<unsigned int>(info[0]);
    try {
        Napi::String result = Napi::String::New(env, input->in ? input->in->getPortName(portNumber).c_str() : "");
        return result;
    }
    catch(RtMidiError& e) {
        return Napi::String::New(env, "");
    }
}

Napi::Value NodeMidiInput::OpenPort(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;

    if (!input->in) return;

    if (info.Length() == 0 || !info[0].IsUint32()) {
        Napi::TypeError::New(env, "First argument must be an integer").ThrowAsJavaScriptException();
        return env.Null();
    }
    unsigned int portNumber = Napi::To<unsigned int>(info[0]);
    if (portNumber >= input->in->getPortCount()) {
        Napi::RangeError::New(env, "Invalid MIDI port number").ThrowAsJavaScriptException();
        return env.Null();
    }

    input->Ref();
    uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
    try {
        input->in->setCallback(&NodeMidiInput::Callback, info.This()).Unwrap<NodeMidiInput>();
        input->configured = true;
        input->in->openPort(portNumber);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

Napi::Value NodeMidiInput::OpenVirtualPort(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;

    if (!input->in) return;

    if (info.Length() == 0 || !info[0].IsString()) {
        Napi::TypeError::New(env, "First argument must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string name(info[0].As<Napi::String>().Utf8Value().c_str());

    input->Ref();
    uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
    try {            
        input->in->setCallback(&NodeMidiInput::Callback, info.This()).Unwrap<NodeMidiInput>();
        input->configured = true;
        input->in->openVirtualPort(name);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

Napi::Value NodeMidiInput::ClosePort(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;

    if (!input->in) return;

    input->cleanUp();
    
    return;
}

Napi::Value NodeMidiInput::IsPortOpen(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;

    if (!input->in) return;

    Napi::Boolean result = Napi::Boolean::New(env, input->in->isPortOpen());
    return result;
}

Napi::Value NodeMidiInput::IgnoreTypes(const Napi::CallbackInfo& info)
{
    Napi::HandleScope scope(env);
    NodeMidiInput* input = this;

    if (!input->in) return;

    if (info.Length() != 3 || !info[0].IsBoolean() || !info[1].IsBoolean() || !info[2].IsBoolean()) {
        Napi::TypeError::New(env, "Arguments must be boolean").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool filter_sysex = info[0].As<Napi::Boolean>().Value();
    bool filter_timing = info[1].As<Napi::Boolean>().Value();
    bool filter_sensing = info[2].As<Napi::Boolean>().Value();
    input->in->ignoreTypes(filter_sysex, filter_timing, filter_sensing);
    return;
}
