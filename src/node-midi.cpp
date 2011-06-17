#include <v8.h>
#include <node.h>

using namespace node;
using namespace v8;

class NodeMidiOutput : ObjectWrap
{
private:
public:
    static Persistent<FunctionTemplate> s_ct;
    static void Init(Handle<Object> target)
    {
        HandleScope scope;
        
        Local<FunctionTemplate> t = FunctionTemplate::New(New);
        
        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("NodeMidiOutput"));
        
        NODE_SET_PROTOTYPE_METHOD(s_ct, "send", Send);
        
        target->Set(String::NewSymbol("output"),
                    s_ct->GetFunction());
    }
    
    NodeMidiOutput()
    {
    }
    
    ~NodeMidiOutput()
    {
    }
    
    static Handle<Value> New(const Arguments& args)
    {
        HandleScope scope;
        NodeMidiOutput* output = new NodeMidiOutput();
        output->Wrap(args.This());
        return args.This();
    }
    
    static Handle<Value> Send(const Arguments& args)
    {
        HandleScope scope;
        NodeMidiOutput* output = ObjectWrap::Unwrap<NodeMidiOutput>(args.This());
        return Boolean::New(true);
    }
    
};

Persistent<FunctionTemplate> NodeMidiOutput::s_ct;

extern "C" {
    void init (Handle<Object> target)
    {
        NodeMidiOutput::Init(target);
    }
    
    NODE_MODULE(nodemidi, init);
}