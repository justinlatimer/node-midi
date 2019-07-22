#include <nan.h>
#include <queue>
#include <uv.h>

#include "lib/RtMidi/RtMidi.h"

#include "input.h"

const char* symbol_emit = "emit";
const char* symbol_message = "message";

void NodeMidiInput::Init(v8::Local<v8::Object> target)
{
    Nan::HandleScope scope;

    v8::Local<v8::FunctionTemplate> t = Nan::New<v8::FunctionTemplate>(NodeMidiInput::New);

    s_ct.Reset(t);
    t->SetClassName(Nan::New<v8::String>("NodeMidiInput").ToLocalChecked());
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(t, "getPortCount", GetPortCount);
    Nan::SetPrototypeMethod(t, "getPortName", GetPortName);

    Nan::SetPrototypeMethod(t, "openPort", OpenPort);
    Nan::SetPrototypeMethod(t, "openVirtualPort", OpenVirtualPort);
    Nan::SetPrototypeMethod(t, "closePort", ClosePort);
    Nan::SetPrototypeMethod(t, "isPortOpen", IsPortOpen);

    Nan::SetPrototypeMethod(t, "ignoreTypes", IgnoreTypes);

    Nan::Set(target, Nan::New<v8::String>("Input").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
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
    Nan::HandleScope scope;
    NodeMidiInput *input = static_cast<NodeMidiInput*>(async->data);
    uv_mutex_lock(&input->message_mutex);
    v8::Local<v8::Function> emitFunction = Nan::Get(input->handle(), Nan::New<v8::String>(symbol_emit).ToLocalChecked()).ToLocalChecked().As<v8::Function>();
    while (!input->message_queue.empty())
    {
        MidiMessage* message = input->message_queue.front();
        v8::Local<v8::Value> info[3];
        info[0] = Nan::New<v8::String>(symbol_message).ToLocalChecked();
        info[1] = Nan::New<v8::Number>(message->deltaTime);
        int32_t count = (int32_t)message->message.size();
        v8::Local<v8::Array> data = Nan::New<v8::Array>(count);
        for (int32_t i = 0; i < count; ++i) {
            Nan::Set(data, Nan::New<v8::Number>(i), Nan::New<v8::Integer>(message->message[i]));
        }
        info[2] = data;
        Nan::Call(emitFunction, input->handle(), 3, info);
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

NAN_METHOD(NodeMidiInput::New)
{
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowTypeError("Use the new operator to create instances of this object.");
    }

    NodeMidiInput* input = new NodeMidiInput();
    input->message_async.data = input;
    input->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NodeMidiInput::GetPortCount)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
    v8::Local<v8::Integer> result = Nan::New<v8::Uint32>(input->in ? input->in->getPortCount() : 0);
    info.GetReturnValue().Set(result);
}

NAN_METHOD(NodeMidiInput::GetPortName)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
    if (info.Length() == 0 || !info[0]->IsUint32()) {
        return Nan::ThrowTypeError("First argument must be an integer");
    }

    unsigned int portNumber = Nan::To<unsigned int>(info[0]).FromJust();
    try {
        v8::Local<v8::String> result = Nan::New<v8::String>(input->in ? input->in->getPortName(portNumber).c_str() : "").ToLocalChecked();
        info.GetReturnValue().Set(result);
    }
    catch(RtMidiError& e) {
        info.GetReturnValue().Set(Nan::New<v8::String>("").ToLocalChecked());
    }
}

NAN_METHOD(NodeMidiInput::OpenPort)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

    if (!input->in) return;

    if (info.Length() == 0 || !info[0]->IsUint32()) {
        return Nan::ThrowTypeError("First argument must be an integer");
    }
    unsigned int portNumber = Nan::To<unsigned int>(info[0]).FromJust();
    if (portNumber >= input->in->getPortCount()) {
        return Nan::ThrowRangeError("Invalid MIDI port number");
    }

    input->Ref();
    uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
    try {
        input->in->setCallback(&NodeMidiInput::Callback, Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This()));
        input->configured = true;
        input->in->openPort(portNumber);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

NAN_METHOD(NodeMidiInput::OpenVirtualPort)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

    if (!input->in) return;

    if (info.Length() == 0 || !info[0]->IsString()) {
        return Nan::ThrowTypeError("First argument must be a string");
    }

    std::string name(*Nan::Utf8String(info[0]));

    input->Ref();
    uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
    try {            
        input->in->setCallback(&NodeMidiInput::Callback, Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This()));
        input->configured = true;
        input->in->openVirtualPort(name);
    }
    catch(RtMidiError& e) {
        ;
    }
    return;
}

NAN_METHOD(NodeMidiInput::ClosePort)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

    if (!input->in) return;

    input->cleanUp();
    
    return;
}

NAN_METHOD(NodeMidiInput::IsPortOpen)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

    if (!input->in) return;

    v8::Local<v8::Boolean> result = Nan::New<v8::Boolean>(input->in->isPortOpen());
    info.GetReturnValue().Set(result);
}

NAN_METHOD(NodeMidiInput::IgnoreTypes)
{
    Nan::HandleScope scope;
    NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

    if (!input->in) return;

    if (info.Length() != 3 || !info[0]->IsBoolean() || !info[1]->IsBoolean() || !info[2]->IsBoolean()) {
        return Nan::ThrowTypeError("Arguments must be boolean");
    }

    bool filter_sysex = Nan::To<bool>(info[0]).FromJust();
    bool filter_timing = Nan::To<bool>(info[1]).FromJust();
    bool filter_sensing = Nan::To<bool>(info[2]).FromJust();
    input->in->ignoreTypes(filter_sysex, filter_timing, filter_sensing);
    return;
}
