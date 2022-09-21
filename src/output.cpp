#include <napi.h>

#include "RtMidi.h"

#include "output.h"

std::unique_ptr<Napi::FunctionReference> NodeMidiOutput::Init(const Napi::Env &env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NodeMidiOutput", {
                                                                 InstanceMethod<&NodeMidiOutput::GetPortCount>("getPortCount", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                 InstanceMethod<&NodeMidiOutput::GetPortName>("getPortName", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),

                                                                 InstanceMethod<&NodeMidiOutput::OpenPort>("openPort", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                 InstanceMethod<&NodeMidiOutput::OpenVirtualPort>("openVirtualPort", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                 InstanceMethod<&NodeMidiOutput::ClosePort>("closePort", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                 InstanceMethod<&NodeMidiOutput::IsPortOpen>("isPortOpen", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),

                                                                 InstanceMethod<&NodeMidiOutput::Send>("sendMessage", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                 InstanceMethod<&NodeMidiOutput::Send>("send", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                             });

    // Create a persistent reference to the class constructor
    std::unique_ptr<Napi::FunctionReference> constructor = std::make_unique<Napi::FunctionReference>();
    *constructor = Napi::Persistent(func);
    exports.Set("Output", func);

    return constructor;
}

NodeMidiOutput::NodeMidiOutput(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NodeMidiOutput>(info)
{
    try
    {
        handle.reset(new RtMidiOut());
    }
    catch (RtMidiError &e)
    {
        handle.reset();
        Napi::Error::New(info.Env(), "Failed to initialise RtMidi").ThrowAsJavaScriptException();
    }
}

NodeMidiOutput::~NodeMidiOutput()
{
    handle->closePort();
}

Napi::Value NodeMidiOutput::GetPortCount(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    return Napi::Number::New(env, handle->getPortCount());
}

Napi::Value NodeMidiOutput::GetPortName(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info.Length() == 0 || !info[0].IsNumber())
    {
        Napi::TypeError::New(env, "First argument must be an integer").ThrowAsJavaScriptException();
        return env.Null();
    }

    unsigned int portNumber = info[0].ToNumber();
    try
    {
        return Napi::String::New(env, handle->getPortName(portNumber));
    }
    catch (RtMidiError &e)
    {
        Napi::TypeError::New(env, "Internal RtMidi error").ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value NodeMidiOutput::OpenPort(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info.Length() == 0 || !info[0].IsNumber())
    {
        Napi::TypeError::New(env, "First argument must be an integer").ThrowAsJavaScriptException();
        return env.Null();
    }

    unsigned int portNumber = info[0].ToNumber();
    if (portNumber >= handle->getPortCount())
    {
        Napi::RangeError::New(env, "Invalid MIDI port number").ThrowAsJavaScriptException();
        return env.Null();
    }

    try
    {
        handle->openPort(portNumber);
    }
    catch (RtMidiError &e)
    {
        Napi::Error::New(env, "Internal RtMidi error").ThrowAsJavaScriptException();
    }

    return env.Null();
}

Napi::Value NodeMidiOutput::OpenVirtualPort(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info.Length() == 0 || !info[0].IsString())
    {
        Napi::TypeError::New(env, "First argument must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string name = info[0].ToString();

    try
    {
        handle->openVirtualPort(name);
    }
    catch (RtMidiError &e)
    {
        Napi::Error::New(env, "Internal RtMidi error").ThrowAsJavaScriptException();
    }

    return env.Null();
}

Napi::Value NodeMidiOutput::ClosePort(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    handle->closePort();
    return env.Null();
}

Napi::Value NodeMidiOutput::IsPortOpen(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    return Napi::Boolean::New(env, handle->isPortOpen());
}

Napi::Value NodeMidiOutput::Send(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info.Length() == 0 || !info[0].IsBuffer())
    {
        Napi::TypeError::New(env, "First argument must be a buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<unsigned char> buffer = info[0].As<Napi::Buffer<unsigned char>>();

    try
    {
        handle->sendMessage(buffer.Data(), buffer.Length());
    }
    catch (RtMidiError &e)
    {
        Napi::Error::New(env, "Internal RtMidi error").ThrowAsJavaScriptException();
    }

    return env.Null();
}
