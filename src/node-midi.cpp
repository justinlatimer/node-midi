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
    static void Init(v8::Handle<v8::Object> target)
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

        Nan::SetPrototypeMethod(t, "sendMessage", SendMessage);

        target->Set(Nan::New<v8::String>("output").ToLocalChecked(), t->GetFunction());
    }

    NodeMidiOutput()
    {
        out = new RtMidiOut();
    }

    ~NodeMidiOutput()
    {
        delete out;
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
        v8::Local<v8::Integer> result = Nan::New<v8::Uint32>(output->out->getPortCount());
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(GetPortName)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
        if (info.Length() == 0 || !info[0]->IsUint32()) {
            return Nan::ThrowTypeError("First argument must be an integer");
        }

        unsigned int portNumber = info[0]->Uint32Value();
        v8::Local<v8::String> result = Nan::New<v8::String>(output->out->getPortName(portNumber).c_str()).ToLocalChecked();
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(OpenPort)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
        if (info.Length() == 0 || !info[0]->IsUint32()) {
            return Nan::ThrowTypeError("First argument must be an integer");
        }
        unsigned int portNumber = info[0]->Uint32Value();
        if (portNumber >= output->out->getPortCount()) {
            return Nan::ThrowRangeError("Invalid MIDI port number");
        }

        output->out->openPort(portNumber);
        return;
    }

    static NAN_METHOD(OpenVirtualPort)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
        if (info.Length() == 0 || !info[0]->IsString()) {
            return Nan::ThrowTypeError("First argument must be a string");
        }

        std::string name(*v8::String::Utf8Value(info[0].As<v8::String>()));

        output->out->openVirtualPort(name);
        return;
    }

    static NAN_METHOD(ClosePort)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
        output->out->closePort();
        return;
    }

    static NAN_METHOD(SendMessage)
    {
        Nan::HandleScope scope;
        NodeMidiOutput* output = Nan::ObjectWrap::Unwrap<NodeMidiOutput>(info.This());
        if (info.Length() == 0 || !info[0]->IsArray()) {
            return Nan::ThrowTypeError("First argument must be an array");
        }

        v8::Local<v8::Object> message = info[0]->ToObject();
        int32_t messageLength = message->Get(Nan::New<v8::String>("length").ToLocalChecked())->Int32Value();
        std::vector<unsigned char> messageOutput;
        for (int32_t i = 0; i != messageLength; ++i) {
            messageOutput.push_back(message->Get(Nan::New<v8::Integer>(i))->Int32Value());
        }
        output->out->sendMessage(&messageOutput);
        return;
    }
};

const char* symbol_emit = "emit";
const char* symbol_message = "message";

class NodeMidiInput : public Nan::ObjectWrap
{
private:
    RtMidiIn* in;

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
    static void Init(v8::Handle<v8::Object> target)
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

        Nan::SetPrototypeMethod(t, "ignoreTypes", IgnoreTypes);

        target->Set(Nan::New<v8::String>("input").ToLocalChecked(), t->GetFunction());
    }

    NodeMidiInput()
    {
        in = new RtMidiIn();
        uv_mutex_init(&message_mutex);
    }

    ~NodeMidiInput()
    {
        in->closePort();
        delete in;
        uv_mutex_destroy(&message_mutex);
    }

    static NAUV_WORK_CB(EmitMessage)
    {
        Nan::HandleScope scope;
        NodeMidiInput *input = static_cast<NodeMidiInput*>(async->data);
        uv_mutex_lock(&input->message_mutex);
        v8::Local<v8::Function> emitFunction = input->handle()->Get(Nan::New<v8::String>(symbol_emit).ToLocalChecked()).As<v8::Function>();
        while (!input->message_queue.empty())
        {
            MidiMessage* message = input->message_queue.front();
            v8::Local<v8::Value> info[3];
            info[0] = Nan::New<v8::String>(symbol_message).ToLocalChecked();
            info[1] = Nan::New<v8::Number>(message->deltaTime);
            int32_t count = (int32_t)message->message.size();
            v8::Local<v8::Array> data = Nan::New<v8::Array>(count);
            for (int32_t i = 0; i < count; ++i) {
                data->Set(Nan::New<v8::Number>(i), Nan::New<v8::Integer>(message->message[i]));
            }
            info[2] = data;
            Nan::MakeCallback(input->handle(), emitFunction, 3, info);
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
        uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
        input->Wrap(info.This());

        info.GetReturnValue().Set(info.This());
    }

    static NAN_METHOD(GetPortCount)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        v8::Local<v8::Integer> result = Nan::New<v8::Uint32>(input->in->getPortCount());
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(GetPortName)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        if (info.Length() == 0 || !info[0]->IsUint32()) {
            return Nan::ThrowTypeError("First argument must be an integer");
        }

        unsigned int portNumber = info[0]->Uint32Value();
        v8::Local<v8::String> result = Nan::New<v8::String>(input->in->getPortName(portNumber).c_str()).ToLocalChecked();
        info.GetReturnValue().Set(result);
    }

    static NAN_METHOD(OpenPort)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        if (info.Length() == 0 || !info[0]->IsUint32()) {
            return Nan::ThrowTypeError("First argument must be an integer");
        }
        unsigned int portNumber = info[0]->Uint32Value();
        if (portNumber >= input->in->getPortCount()) {
            return Nan::ThrowRangeError("Invalid MIDI port number");
        }

        input->Ref();
        input->in->setCallback(&NodeMidiInput::Callback, Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This()));
        input->in->openPort(portNumber);
        return;
    }

    static NAN_METHOD(OpenVirtualPort)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        if (info.Length() == 0 || !info[0]->IsString()) {
            return Nan::ThrowTypeError("First argument must be a string");
        }

        std::string name(*v8::String::Utf8Value(info[0].As<v8::String>()));

        input->Ref();
        input->in->setCallback(&NodeMidiInput::Callback, Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This()));
        input->in->openVirtualPort(name);
        return;
    }

    static NAN_METHOD(ClosePort)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        if (input->in->isPortOpen()) {
            input->Unref();
        }
        input->in->cancelCallback();
        input->in->closePort();
        uv_close((uv_handle_t*)&input->message_async, NULL);
        return;
    }

    static NAN_METHOD(IgnoreTypes)
    {
        Nan::HandleScope scope;
        NodeMidiInput* input = Nan::ObjectWrap::Unwrap<NodeMidiInput>(info.This());
        if (info.Length() != 3 || !info[0]->IsBoolean() || !info[1]->IsBoolean() || !info[2]->IsBoolean()) {
            return Nan::ThrowTypeError("Arguments must be boolean");
        }

        bool filter_sysex = info[0]->BooleanValue();
        bool filter_timing = info[1]->BooleanValue();
        bool filter_sensing = info[2]->BooleanValue();
        input->in->ignoreTypes(filter_sysex, filter_timing, filter_sensing);
        return;
    }
};

Nan::Persistent<v8::FunctionTemplate> NodeMidiOutput::s_ct;
Nan::Persistent<v8::FunctionTemplate> NodeMidiInput::s_ct;

extern "C" {
    void init (v8::Handle<v8::Object> target)
    {
        NodeMidiOutput::Init(target);
        NodeMidiInput::Init(target);
    }
    NODE_MODULE(midi, init)
}
