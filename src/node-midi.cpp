#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <node_version.h>
#include <nan.h>
#include <queue>
#include <uv.h>

#include "lib/RtMidi/RtMidi.h"
#include "lib/RtMidi/RtMidi.cpp"

class NodeMidiOutput : public node::ObjectWrap
{
private:
    RtMidiOut* out;
public:
    static v8::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Handle<v8::Object> target)
    {
        NanScope();

        v8::Local<v8::FunctionTemplate> t = NanNew<v8::FunctionTemplate>(NodeMidiOutput::New);

        NanAssignPersistent(s_ct, t);
        t->SetClassName(NanNew<v8::String>("NodeMidiOutput"));
        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "getPortCount", GetPortCount);
        NODE_SET_PROTOTYPE_METHOD(t, "getPortName", GetPortName);

        NODE_SET_PROTOTYPE_METHOD(t, "openPort", OpenPort);
        NODE_SET_PROTOTYPE_METHOD(t, "openVirtualPort", OpenVirtualPort);
        NODE_SET_PROTOTYPE_METHOD(t, "closePort", ClosePort);
        NODE_SET_PROTOTYPE_METHOD(t, "isPortOpen", IsPortOpen);

        NODE_SET_PROTOTYPE_METHOD(t, "sendMessage", SendMessage);

        target->Set(NanNew<v8::String>("output"), t->GetFunction());
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
        NanScope();
        if (!args.IsConstructCall()) {
            NanThrowTypeError("Use the new operator to create instances of this object.");
            NanReturnUndefined();
        }

        NodeMidiOutput* output = new NodeMidiOutput();
        output->Wrap(args.This());
        NanReturnValue(args.This());
    }

    static NAN_METHOD(GetPortCount)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        v8::Local<v8::Integer> result = NanNew<v8::Uint32>(output->out->getPortCount());
        NanReturnValue(result);
    }

    static NAN_METHOD(GetPortName)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            NanThrowTypeError("First argument must be an integer");
            NanReturnUndefined();
        }

        unsigned int portNumber = args[0]->Uint32Value();
        v8::Local<v8::String> result = NanNew<v8::String>(output->out->getPortName(portNumber).c_str());
        NanReturnValue(result);
    }

    static NAN_METHOD(OpenPort)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (output->out->isPortOpen()) {
            NanReturnValue(NanNew<v8::Boolean>(false));
        }
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            NanThrowTypeError("First argument must be an integer");
            NanReturnUndefined();
        }
        unsigned int portNumber = args[0]->Uint32Value();
        if (portNumber >= output->out->getPortCount()) {
            NanThrowRangeError("Invalid MIDI port number");
            NanReturnUndefined();
        }

        output->out->openPort(portNumber);
        NanReturnValue(NanNew<v8::Boolean>(true));
    }

    static NAN_METHOD(OpenVirtualPort)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (output->out->isPortOpen()) {
            NanReturnValue(NanNew<v8::Boolean>(false));
        }
        if (args.Length() == 0 || !args[0]->IsString()) {
            NanThrowTypeError("First argument must be a string");
            NanReturnUndefined();
        }

        std::string name(*NanAsciiString(args[0]));
        output->out->openVirtualPort(name);
        NanReturnValue(NanNew<v8::Boolean>(true));
    }

    static NAN_METHOD(ClosePort)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (!output->out->isPortOpen()) {
            NanReturnValue(NanNew<v8::Boolean>(false));
        }

        output->out->closePort();
        NanReturnValue(NanNew<v8::Boolean>(true));
    }

    static NAN_METHOD(IsPortOpen)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        v8::Local<v8::Boolean> result = NanNew<v8::Boolean>(output->out->isPortOpen());
        NanReturnValue(result);
    }

    static NAN_METHOD(SendMessage)
    {
        NanScope();
        NodeMidiOutput* output = node::ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (args.Length() == 0 || !args[0]->IsArray()) {
            NanThrowTypeError("First argument must be an array");
            NanReturnUndefined();
        }

        v8::Local<v8::Object> message = args[0]->ToObject();
        int32_t messageLength = message->Get(NanNew<v8::String>("length"))->Int32Value();
        std::vector<unsigned char> messageOutput;
        for (int32_t i = 0; i != messageLength; ++i) {
            messageOutput.push_back(message->Get(NanNew<v8::Integer>(i))->Int32Value());
        }
        output->out->sendMessage(&messageOutput);
        NanReturnUndefined();
    }
};

const char* symbol_emit = "emit";
const char* symbol_message = "message";

class NodeMidiInput : public node::ObjectWrap
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

    static v8::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Handle<v8::Object> target)
    {
        NanScope();

        v8::Local<v8::FunctionTemplate> t = NanNew<v8::FunctionTemplate>(NodeMidiInput::New);

        NanAssignPersistent(s_ct, t);
        t->SetClassName(NanNew<v8::String>("NodeMidiInput"));
        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "getPortCount", GetPortCount);
        NODE_SET_PROTOTYPE_METHOD(t, "getPortName", GetPortName);

        NODE_SET_PROTOTYPE_METHOD(t, "openPort", OpenPort);
        NODE_SET_PROTOTYPE_METHOD(t, "openVirtualPort", OpenVirtualPort);
        NODE_SET_PROTOTYPE_METHOD(t, "closePort", ClosePort);
        NODE_SET_PROTOTYPE_METHOD(t, "isPortOpen", IsPortOpen);

        NODE_SET_PROTOTYPE_METHOD(t, "ignoreTypes", IgnoreTypes);

        target->Set(NanNew<v8::String>("input"), t->GetFunction());
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
        NanScope();
        NodeMidiInput *input = static_cast<NodeMidiInput*>(async->data);
        uv_mutex_lock(&input->message_mutex);
        v8::Local<v8::Function> emitFunction = NanObjectWrapHandle(input)->Get(NanNew<v8::String>(symbol_emit)).As<v8::Function>();
        while (!input->message_queue.empty())
        {
            MidiMessage* message = input->message_queue.front();
            v8::Local<v8::Value> args[3];
            args[0] = NanNew<v8::String>(symbol_message);
            args[1] = NanNew<v8::Number>(message->deltaTime);
            int32_t count = (int32_t)message->message.size();
            v8::Local<v8::Array> data = NanNew<v8::Array>(count);
            for (int32_t i = 0; i < count; ++i) {
                data->Set(NanNew<v8::Number>(i), NanNew<v8::Integer>(message->message[i]));
            }
            args[2] = data;
            NanMakeCallback(NanObjectWrapHandle(input), emitFunction, 3, args);
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
        NanScope();
        if (!args.IsConstructCall()) {
            NanThrowTypeError("Use the new operator to create instances of this object.");
            NanReturnUndefined();
        }

        NodeMidiInput* input = new NodeMidiInput();
        input->message_async.data = input;
        input->Wrap(args.This());
        NanReturnValue(args.This());
    }

    static NAN_METHOD(GetPortCount)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        v8::Local<v8::Integer> result = NanNew<v8::Uint32>(input->in->getPortCount());
        NanReturnValue(result);
    }

    static NAN_METHOD(GetPortName)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            NanThrowTypeError("First argument must be an integer");
            NanReturnUndefined();
        }

        unsigned int portNumber = args[0]->Uint32Value();
        v8::Local<v8::String> result = NanNew<v8::String>(input->in->getPortName(portNumber).c_str());
        NanReturnValue(result);
    }

    static NAN_METHOD(OpenPort)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (input->in->isPortOpen()) {
            NanReturnValue(NanNew<v8::Boolean>(false));
        }
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            NanThrowTypeError("First argument must be an integer");
            NanReturnUndefined();
        }
        unsigned int portNumber = args[0]->Uint32Value();
        if (portNumber >= input->in->getPortCount()) {
            NanThrowRangeError("Invalid MIDI port number");
            NanReturnUndefined();
        }

        input->Ref();
        input->in->setCallback(&NodeMidiInput::Callback, input);
        uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
        input->in->openPort(portNumber);
        NanReturnValue(NanNew<v8::Boolean>(true));
    }

    static NAN_METHOD(OpenVirtualPort)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (input->in->isPortOpen()) {
            NanReturnValue(NanNew<v8::Boolean>(false));
        }
        if (args.Length() == 0 || !args[0]->IsString()) {
            NanThrowTypeError("First argument must be a string");
            NanReturnUndefined();
        }

        input->Ref();
        std::string name(*NanAsciiString(args[0]));
        input->in->setCallback(&NodeMidiInput::Callback, input);
        uv_async_init(uv_default_loop(), &input->message_async, NodeMidiInput::EmitMessage);
        input->in->openVirtualPort(name);
        NanReturnValue(NanNew<v8::Boolean>(true));
    }

    static NAN_METHOD(ClosePort)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (!input->in->isPortOpen()) {
            NanReturnValue(NanNew<v8::Boolean>(false));
        }

        input->Unref();
        input->in->closePort();
        input->in->cancelCallback();
        uv_close((uv_handle_t*)&input->message_async, NULL);
        NanReturnValue(NanNew<v8::Boolean>(true));
    }

    static NAN_METHOD(IsPortOpen)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        v8::Local<v8::Boolean> result = NanNew<v8::Boolean>(input->in->isPortOpen());
        NanReturnValue(result);
    }

    static NAN_METHOD(IgnoreTypes)
    {
        NanScope();
        NodeMidiInput* input = node::ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (args.Length() != 3 || !args[0]->IsBoolean() || !args[1]->IsBoolean() || !args[2]->IsBoolean()) {
            NanThrowTypeError("Arguments must be boolean");
            NanReturnUndefined();
        }

        bool filter_sysex = args[0]->BooleanValue();
        bool filter_timing = args[1]->BooleanValue();
        bool filter_sensing = args[2]->BooleanValue();
        input->in->ignoreTypes(filter_sysex, filter_timing, filter_sensing);
        NanReturnUndefined();
    }
};

v8::Persistent<v8::FunctionTemplate> NodeMidiOutput::s_ct;
v8::Persistent<v8::FunctionTemplate> NodeMidiInput::s_ct;

extern "C" {
    void init (v8::Handle<v8::Object> target)
    {
        NodeMidiOutput::Init(target);
        NodeMidiInput::Init(target);
    }
    NODE_MODULE(midi, init)
}
