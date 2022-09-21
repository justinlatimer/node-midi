#include <napi.h>
#include <queue>

#include "RtMidi.h"

#include "input.h"

const char *symbol_emit = "emit";
const char *symbol_message = "message";

std::unique_ptr<Napi::FunctionReference> NodeMidiInput::Init(const Napi::Env &env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NodeMidiInput", {
                                                                InstanceMethod<&NodeMidiInput::GetPortCount>("getPortCount", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                InstanceMethod<&NodeMidiInput::GetPortName>("getPortName", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),

                                                                InstanceMethod<&NodeMidiInput::OpenPort>("openPort", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                InstanceMethod<&NodeMidiInput::OpenVirtualPort>("openVirtualPort", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                InstanceMethod<&NodeMidiInput::ClosePort>("closePort", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                                InstanceMethod<&NodeMidiInput::IsPortOpen>("isPortOpen", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),

                                                                InstanceMethod<&NodeMidiInput::IgnoreTypes>("ignoreTypes", static_cast<napi_property_attributes>(napi_writable | napi_configurable)),
                                                            });

    // Create a persistent reference to the class constructor
    std::unique_ptr<Napi::FunctionReference> constructor = std::make_unique<Napi::FunctionReference>();
    *constructor = Napi::Persistent(func);
    exports.Set("Input", func);

    return constructor;
}

NodeMidiInput::NodeMidiInput(const Napi::CallbackInfo &info) : Napi::ObjectWrap<NodeMidiInput>(info)
{
    if (info.Length() == 0 || !info[0].IsFunction())
    {
        Napi::Error::New(info.Env(), "Expected a callback").ThrowAsJavaScriptException();
        return;
    }

    try
    {
        handle.reset(new RtMidiIn());
    }
    catch (RtMidiError &e)
    {
        handle.reset();
        Napi::Error::New(info.Env(), "Failed to initialise RtMidi").ThrowAsJavaScriptException();
        return;
    }

    emitMessage = Napi::Persistent(info[0].As<Napi::Function>());
}

NodeMidiInput::~NodeMidiInput()
{
    cleanup();
}

void NodeMidiInput::setupCallback(const Napi::Env &env)
{
    if (!configured)
    {
        configured = true;

        handleMessage = TSFN_t::New(
            env,
            emitMessage.Value(),
            "Midi Input",
            0,
            1);

        handle->setCallback(&NodeMidiInput::Callback, this);
    }
}

void NodeMidiInput::cleanup()
{
    handle->closePort();

    if (configured)
    {
        configured = false;

        handle->cancelCallback();
        handleMessage.Abort();
        handleMessage.Release();
    }
}

void NodeMidiInput::Callback(double deltaTime, std::vector<unsigned char> *message, void *userData)
{
    NodeMidiInput *input = static_cast<NodeMidiInput *>(userData);

    MidiMessage *data = new MidiMessage();
    data->deltaTime = deltaTime;
    data->messageLength = message->size();
    data->message = new unsigned char[data->messageLength];
    memcpy(data->message, message->data(), data->messageLength * sizeof(unsigned char));

    // Forward to CallbackJs
    input->handleMessage.NonBlockingCall(data);
}

void deleteArray(const Napi::Env &env, unsigned char *ptr)
{
    delete[] ptr;
}

void NodeMidiInput::CallbackJs(Napi::Env env, Napi::Function callback, void *context, MidiMessage *data)
{
    if (env != nullptr)
    {
        Napi::Value deltaTime = Napi::Number::New(env, data->deltaTime);

        Napi::Value message = Napi::Buffer<unsigned char>::New(env, data->message, data->messageLength, deleteArray);
        // data->message is now owned by the buffer
        data->message = nullptr;

        callback.Call({deltaTime, message});
    }

    if (data != nullptr)
    {
        if (data->message != nullptr)
        {
            delete[] data->message;
        }

        // We're finished with the data.
        delete data;
    }
}

Napi::Value NodeMidiInput::GetPortCount(const Napi::CallbackInfo &info)
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

Napi::Value NodeMidiInput::GetPortName(const Napi::CallbackInfo &info)
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

Napi::Value NodeMidiInput::OpenPort(const Napi::CallbackInfo &info)
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
        setupCallback(env);
        handle->openPort(portNumber);
    }
    catch (RtMidiError &e)
    {
        Napi::Error::New(env, "Internal RtMidi error").ThrowAsJavaScriptException();
    }

    return env.Null();
}

Napi::Value NodeMidiInput::OpenVirtualPort(const Napi::CallbackInfo &info)
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
        setupCallback(env);
        handle->openVirtualPort(name);
    }
    catch (RtMidiError &e)
    {
        Napi::Error::New(env, "Internal RtMidi error").ThrowAsJavaScriptException();
    }

    return env.Null();
}

Napi::Value NodeMidiInput::ClosePort(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    cleanup();
    return env.Null();
}

Napi::Value NodeMidiInput::IsPortOpen(const Napi::CallbackInfo &info)
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

Napi::Value NodeMidiInput::IgnoreTypes(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    if (!handle)
    {
        Napi::Error::New(env, "RtMidi not initialised").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info.Length() != 3 || !info[0].IsBoolean() || !info[1].IsBoolean() || !info[2].IsBoolean())
    {
        Napi::TypeError::New(env, "Arguments must be boolean").ThrowAsJavaScriptException();
        return env.Null();
    }

    bool filter_sysex = info[0].ToBoolean();
    bool filter_timing = info[1].ToBoolean();
    bool filter_sensing = info[2].ToBoolean();
    handle->ignoreTypes(filter_sysex, filter_timing, filter_sensing);

    return env.Null();
}
