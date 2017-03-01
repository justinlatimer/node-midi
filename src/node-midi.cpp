#include <nan.h>
#include <queue>
#include <uv.h>

#include "lib/RtMidi/RtMidi.h"
#include "lib/RtMidi/RtMidi.cpp"

class NodeMidiOutput : public Nan::ObjectWrap
{
private:
    RtMidiOut* out;
public:
    static Nan::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Local<v8::Object> target)
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

        Nan::Set(target, Nan::New<v8::String>("output").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
    }

    NodeMidiOutput()
    {
        try {
            out = new RtMidiOut();
        }
        catch(RtMidiError &e) {
            out = nullptr;
        }
    }

    ~NodeMidiOutput()
    {
        if (out) {
            delete out;
        }
    }

    static NAN_METHOD(New)
    {
        Nan::HandleScope scope;

        if (!info.IsConstructCall()) {
            return Nan::ThrowTypeError("Use the new operator to create instances of this object.");
        }

        NodeMidiOutput* output = new NodeMidiOutput();
        output->Wrap(info.This());

        info.GetReturnValue().Set(info.This());
    }

    static NAN_METHOD(GetPortCount)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
        v8::Local<v8::Integer> result = Nan::New<v8::Uint32>(output-> out ? output->out->getPortCount() : 0);
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(GetPortName)
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

    static NAN_METHOD(OpenPort)
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

    static NAN_METHOD(OpenVirtualPort)
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

    static NAN_METHOD(ClosePort)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

        if (!output->out) return;

        output->out->closePort();
        return;
    }

    static NAN_METHOD(IsPortOpen)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());

        if (!output->out) return;

        v8::Local<v8::Boolean> result = Nan::New<v8::Boolean>(output->out->isPortOpen());
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(SendMessage)
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
};

const char* symbol_emit = "emit";
const char* symbol_message = "message";

class NodeMidiInput : public Nan::ObjectWrap
{
private:
    RtMidiIn* in;
    bool configured;

public:
    uv_async_t message_async;
    uv_mutex_t message_mutex;

    struct MidiMessage
    {
        double deltaTime;
        std::vector<unsigned char> message;
    };
    std::queue<MidiMessage*> message_queue;

    static Nan::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Local<v8::Object> target)
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

        Nan::Set(target, Nan::New<v8::String>("input").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
    }

    NodeMidiInput()
    {
        try {
            in = new RtMidiIn();
        }
        catch(RtMidiError& e) {
            in = nullptr;
        }
        configured = false;
        uv_mutex_init(&message_mutex);
    }

    ~NodeMidiInput()
    {
        if (in) {
            cleanUp();
            delete in;
        }
        uv_mutex_destroy(&message_mutex);
    }

    void cleanUp()
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

    static NAUV_WORK_CB(EmitMessage)
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

    static void Callback(double deltaTime, std::vector<unsigned char> *message, void *userData)
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

    static NAN_METHOD(New)
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

    static NAN_METHOD(GetPortCount)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        v8::Local<v8::Integer> result = Nan::New<v8::Uint32>(input->in ? input->in->getPortCount() : 0);
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(GetPortName)
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

    static NAN_METHOD(OpenPort)
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

    static NAN_METHOD(OpenVirtualPort)
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

    static NAN_METHOD(ClosePort)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

        if (!input->in) return;

        input->cleanUp();
        
        return;
    }

    static NAN_METHOD(IsPortOpen)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());

        if (!input->in) return;

        v8::Local<v8::Boolean> result = Nan::New<v8::Boolean>(input->in->isPortOpen());
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(IgnoreTypes)
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
};

Nan::Persistent<v8::FunctionTemplate> NodeMidiOutput::s_ct;
Nan::Persistent<v8::FunctionTemplate> NodeMidiInput::s_ct;

extern "C" {
    void init (v8::Local<v8::Object> target)
    {
        NodeMidiOutput::Init(target);
        NodeMidiInput::Init(target);
    }
    NODE_MODULE(midi, init)
}
