#include <v8.h>
#include <node.h>

#include "lib/RtMidi/RtMidi.h"
#include "lib/RtMidi/RtMidi.cpp"

using namespace node;

#define SAFE_NODE_SET_PROTOTYPE_METHOD(templ, name, callback)             \
do {                                                                      \
  v8::Local<v8::Signature> __callback##_SIG = v8::Signature::New(templ);  \
  v8::Local<v8::FunctionTemplate> __callback##_TEM =                      \
    v8::FunctionTemplate::New(callback, v8::Handle<v8::Value>(),          \
                          __callback##_SIG);                              \
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol(name),            \
                                  __callback##_TEM);                      \
} while (0)

class NodeMidiOutput : ObjectWrap
{
private:
    RtMidiOut* out;
public:
    static v8::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Handle<v8::Object> target)
    {
        v8::HandleScope scope;
        
        v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(New);
        
        s_ct = v8::Persistent<v8::FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(v8::String::NewSymbol("NodeMidiOutput"));
        
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "getPortCount", GetPortCount);
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "getPortName", GetPortName);
        
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "openPort", OpenPort);
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "closePort", ClosePort);
        
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "sendMessage", SendMessage);
        
        target->Set(v8::String::NewSymbol("output"),
                    s_ct->GetFunction());
    }
    
    NodeMidiOutput()
    {
        out = new RtMidiOut();
    }
    
    ~NodeMidiOutput()
    {
        delete out;
    }

    static v8::Handle<v8::Value> New(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiOutput* output = new NodeMidiOutput();
        output->Wrap(args.This());
        return args.This();
    }
    
    static v8::Handle<v8::Value> GetPortCount(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiOutput* output = ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        v8::Local<v8::Integer> result = v8::Uint32::New(output->out->getPortCount());
        return scope.Close(result);
    }
    
    static v8::Handle<v8::Value> GetPortName(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiOutput* output = ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            return ThrowException(v8::Exception::TypeError(
                v8::String::New("First argument must be an integer")));
        }
        unsigned int portNumber = args[0]->Uint32Value();
        v8::Local<v8::String> result = v8::String::New(output->out->getPortName(portNumber).c_str());
        return scope.Close(result);
    }
    
    static v8::Handle<v8::Value> OpenPort(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiOutput* output = ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            return ThrowException(v8::Exception::TypeError(
                v8::String::New("First argument must be an integer")));
        }
        unsigned int portNumber = args[0]->Uint32Value();
        output->out->openPort(portNumber);
        return scope.Close(v8::Boolean::New(true));
    }
    
    static v8::Handle<v8::Value> ClosePort(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiOutput* output = ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        output->out->closePort();
        return scope.Close(v8::Boolean::New(true));
    }
    
    static v8::Handle<v8::Value> SendMessage(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiOutput* output = ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        if (args.Length() == 0 || !args[0]->IsArray()) {
            return ThrowException(v8::Exception::TypeError(
                v8::String::New("First argument must be an array")));
        }
        v8::Local<v8::Object> message = args[0]->ToObject();
        size_t messageLength = message->Get(v8::String::New("length"))->Int32Value();
        std::vector<unsigned char> messageOutput;
        for (size_t i = 0; i != messageLength; ++i) {
            messageOutput.push_back(message->Get(v8::Integer::New(i))->Int32Value());
        }
        output->out->sendMessage(&messageOutput);
        return scope.Close(v8::Boolean::New(true));
    }
    
};

class NodeMidiInput : ObjectWrap
{
private:
    RtMidiIn* in;
public:
    static v8::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Handle<v8::Object> target)
    {
        v8::HandleScope scope;
        
        v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(New);
        
        s_ct = v8::Persistent<v8::FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(v8::String::NewSymbol("NodeMidiInput"));
        
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "getPortCount", GetPortCount);
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "getPortName", GetPortName);
        
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "openPort", OpenPort);
        SAFE_NODE_SET_PROTOTYPE_METHOD(s_ct, "closePort", ClosePort);
        
        target->Set(v8::String::NewSymbol("input"),
                    s_ct->GetFunction());
    }
    
    NodeMidiInput()
    {
        in = new RtMidiIn();
    }
    
    ~NodeMidiInput()
    {
        delete in;
    }
    
    static v8::Handle<v8::Value> New(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiInput* input = new NodeMidiInput();
        input->Wrap(args.This());
        return args.This();
    }
    
    static v8::Handle<v8::Value> GetPortCount(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiInput* input = ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        v8::Local<v8::Integer> result = v8::Uint32::New(input->in->getPortCount());
        return scope.Close(result);
    }
    
    static v8::Handle<v8::Value> GetPortName(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiInput* input = ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            return ThrowException(v8::Exception::TypeError(
                v8::String::New("First argument must be an integer")));
        }
        unsigned int portNumber = args[0]->Uint32Value();
        v8::Local<v8::String> result = v8::String::New(input->in->getPortName(portNumber).c_str());
        return scope.Close(result);
    }
    
    static v8::Handle<v8::Value> OpenPort(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiInput* input = ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        if (args.Length() == 0 || !args[0]->IsUint32()) {
            return ThrowException(v8::Exception::TypeError(
                v8::String::New("First argument must be an integer")));
        }
        unsigned int portNumber = args[0]->Uint32Value();
        input->in->openPort(portNumber);
        return scope.Close(v8::Boolean::New(true));
    }
    
    static v8::Handle<v8::Value> ClosePort(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        NodeMidiInput* input = ObjectWrap::Unwrap<NodeMidiInput>(args.This());
        input->in->closePort();
        return scope.Close(v8::Boolean::New(true));
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
    
    NODE_MODULE(nodemidi, init);
}