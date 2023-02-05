#include <iostream>
#include "easywsclient.hpp"
#include "cCamillaDSP.hpp"
#include "jsmn.hpp"
#include <cstring>
#include <cmath>
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif

using namespace std;

struct response {
    string command;
    string result;
    string value;
} rsp;

tuple<int, int, int> _library_version = make_tuple(0, 0, 1);
using easywsclient::WebSocket;
static WebSocket::pointer _ws = NULL;
tuple <int, int, int> _version;
bool _await_response = false;
response _reply;
string _last_command;

static int jsoneq(string json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json.c_str() + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

void _handle_reply(const std::string &message)
{
    _await_response = false; // mark that the response has been received

    _reply.command = "";
    _reply.result = "";
    _reply.value = "";

    jsmn_parser p;
    jsmntok_t t[32]; /* We expect no more than 32 tokens */

    int i;
    int r;

    jsmn_init(&p);
    r = jsmn_parse(&p, message.c_str(), strlen(message.c_str()), t, sizeof(t) / sizeof(t[0]));
    if (r < 0) {
        throw ios_base::failure("Failed to parse JSON: " + r);
    }

    if (message.find(_last_command) != string::npos)
    {
        string command = message.substr(t[1].start, t[1].end - t[1].start);
        string result, value;

        for (i = 2; i < r; i++) {
            if (jsoneq(message, &t[i], "result") == 0) {
                result = message.substr(t[i + 1].start, t[i + 1].end - t[i + 1].start);
                i++;
            }
            else if (jsoneq(message, &t[i], "value") == 0) {
                value = message.substr(t[i + 1].start, t[i + 1].end - t[i + 1].start);
                i++;
            }
        }

        if (result == "Error")
        {
            if (value.length() != 0)
            {
                throw ios_base::failure("CamillaDSP Error: " + value);
            }
            else
            {
                throw ios_base::failure("CamillaDSP Error: Command returned an error");
            }
        }

        _reply.command = command;
        _reply.result = result;
        _reply.value = value;
    }
    else
    {
        throw ios_base::failure("Invalid response received: " + message);
    }
}

ProcessingState _state_from_string(string value)
{
    if (value == "Running")
        return ProcessingState::RUNNING;
    else if (value == "Paused")
        return ProcessingState::PAUSED;
    else if (value == "Inactive")
        return ProcessingState::INACTIVE;
    else if (value == "Starting")
        return ProcessingState::STARTING;
    else if (value == "Stalled")
        return ProcessingState::STALLED;
    else
        throw out_of_range("Invalid value for ProcessingState: " + value);
}

StopReason _reason_from_reply(string value)
{
    if (value == "None")
        return StopReason::NONE;
    else if (value == "Done")
        return StopReason::DONE;
    else if (value == "CaptureError")
        return StopReason::CAPTUREERROR;
    else if (value == "PlaybackError")
        return StopReason::PLAYBACKERROR;
    else if (value == "CaptureFormatChange")
        return StopReason::CAPTUREFORMATCHANGE;
    else if (value == "PlaybackFormatChange")
        return StopReason::PLAYBACKFORMATCHANGE;
    else
        throw out_of_range("Invalid value for StopReason: " + value);
}

//Is websocket connected? Returns True or False.
bool is_connected()
{
    return _ws->getReadyState() != WebSocket::CLOSED;
}

void _update_version(string resp)
{
    _version = make_tuple(stoi(resp.substr(0, 1)), stoi(resp.substr(2, 1)), stoi(resp.substr(4, 1)));
}

//Read CamillaDSP version, returns a tuple of (major, minor, patch).
tuple<int, int, int> get_version()
{
    return _version;
}

//Read ccamilladsp version, returns a tuple of (major, minor, patch).
tuple<int, int, int> get_library_version()
{
    return _library_version;
}

response _query(string command, string arg = "")
{
    if (_ws)
    {
        string query;

        if (!arg.empty()) {
            query = "{\"" + command + "\":" + arg + "}";
        }
        else {
            query = "\"" + command + "\"";
        }

        try {
            _await_response = true;
            _last_command = command;

            _ws->send(query);
            while (_ws->getReadyState() != WebSocket::CLOSED && _await_response) {
                _ws->poll();
                _ws->dispatch(_handle_reply);
            }
        }
        catch (exception()) {
            throw ios_base::failure("Lost connection to CamillaDSP");
        }

        return _reply;
    }
    else
    {
        throw ios_base::failure("Not connected to CamillaDSP");
    }
}

//Connect to the websocket of CamillaDSP.
void connect(std::string host, int port)
{
    _ws = WebSocket::from_url("ws://" + host + ":" + std::to_string(port));
    response rawvers = _query("GetVersion");
    _update_version(rawvers.value);
}

//Close the connection to the websocket.
void disconnect()
{
    if (_ws) {
        _ws->close();
    }

    delete _ws;
}

//Read what device types the running CamillaDSP process supports. 
//Returns a tuple with two lists of device types, the first for playback and the second for capture.
tuple<list<string>, list<string>> get_supported_device_types()
{
    _query("GetSupportedDeviceTypes");

    string message = _reply.value.c_str();

    jsmn_parser p;
    jsmntok_t t[32]; /* We expect no more than 32 tokens */

    jsmn_init(&p);
    int r = jsmn_parse(&p, message.c_str(), message.length(), t, sizeof(t) / sizeof(t[0]));
    if (r < 0) {
        throw ios_base::failure("Failed to parse JSON: " + r);
    }

    list<string> playback_devices;
    list<string> capture_devices;

    bool end_of_playback = false;

    for (int i = 2; i < r; i++)
    {
        if (t[i].type == JSMN_ARRAY)
        {
            end_of_playback = true;
            continue;
        }

        if (!end_of_playback)
        {
            playback_devices.push_back(message.substr(t[i].start, t[i].end - t[i].start));
        }
        else
        {
            capture_devices.push_back(message.substr(t[i].start, t[i].end - t[i].start));
        }
    }

    return make_tuple(playback_devices, capture_devices);
}

//Get current processing state.
ProcessingState get_state()
{
    _query("GetState");
    return _state_from_string(_reply.value);
}

//Get current stop reason.
StopReason get_stop_reason()
{
    _query("GetStopReason");
    return _reason_from_reply(_reply.value);
}

//Get current signal range. Maximum value is 2.0.
float get_signal_range()
{
    _query("GetSignalRange");
    return stof(_reply.value);
}

//Get current signal range in dB. Full scale is 0 dB.
float get_signal_range_dB()
{
    float sigrange = get_signal_range();
    float range_dB;

    if (sigrange > 0.0)
        range_dB = (float)20.0 * (float)log10(sigrange / 2.0);
    else
        range_dB = -1000;

    return range_dB;
}

//Parse the signal levels returned as JSON into a list
list<float> _parse_signal_levels(string message)
{
    jsmn_parser p;
    jsmntok_t t[32]; /* We expect no more than 32 tokens */

    jsmn_init(&p);
    int r = jsmn_parse(&p, message.c_str(), message.length(), t, sizeof(t) / sizeof(t[0]));
    if (r < 0) {
        throw ios_base::failure("Failed to parse JSON: " + r);
    }

    list<float> capture_signal_rms;

    for (int i = 1; i < r; i++)
    {
        if (t[i].type == JSMN_ARRAY)
        {
            continue;
        }

        capture_signal_rms.push_back(stof(message.substr(t[i].start, t[i].end - t[i].start)));
    }

    return capture_signal_rms;
}

//Get capture signal level rms in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_capture_signal_rms()
{
    _query("GetCaptureSignalRms");
    return _parse_signal_levels(_reply.value);
}

//Get playback signal level rms in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_playback_signal_rms()
{
    _query("GetPlaybackSignalRms");
    return _parse_signal_levels(_reply.value);
}

//Get capture signal level peak in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_capture_signal_peak()
{
    _query("GetCaptureSignalPeak");
    return _parse_signal_levels(_reply.value);
}

//Get playback signal level peak in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_playback_signal_peak()
{
    _query("GetPlaybackSignalPeak");
    return _parse_signal_levels(_reply.value);
}

//Get current volume setting in dB.
float get_volume()
{
    _query("GetVolume");
    return stof(_reply.value);
}

//Set volume in dB.
void set_volume(float value)
{
    _query("SetVolume", to_string(value));
}

//Get current mute setting.
bool get_mute()
{
    _query("GetMute");
    return _reply.value != "false";
}

//Set mute, true or false.
void set_mute(bool value)
{
    _query("SetMute", value ? "True" : "False");
}

//Get current capture rate, raw value.
int get_capture_rate_raw()
{
    _query("GetCaptureRate");
    return stoi(_reply.value);
}

//Get current capture rate. Returns the nearest common rate, as long as it's within +-4% of the measured value.
STANDARD_RATES get_capture_rate()
{
    int rate = get_capture_rate_raw();

    if (10584 < rate && rate < 798720)
    {
        float minrate = (float)0.96 * rate;
        float maxrate = (float)1.04 * rate;

        list<STANDARD_RATES> all_Rates = {
            STANDARD_RATES::RATE_8000,
            STANDARD_RATES::RATE_11025,
            STANDARD_RATES::RATE_16000,
            STANDARD_RATES::RATE_22050,
            STANDARD_RATES::RATE_32000,
            STANDARD_RATES::RATE_44100,
            STANDARD_RATES::RATE_48000,
            STANDARD_RATES::RATE_88200,
            STANDARD_RATES::RATE_96000,
            STANDARD_RATES::RATE_176400,
            STANDARD_RATES::RATE_192000,
            STANDARD_RATES::RATE_352800,
            STANDARD_RATES::RATE_384000,
            STANDARD_RATES::RATE_705600,
            STANDARD_RATES::RATE_768000
        };

        for (STANDARD_RATES test_rate : all_Rates)
        {
            int trate = static_cast<int>(test_rate);

            if (minrate < trate && trate < maxrate)
            {
                return test_rate;
            }
        }
    }

    throw out_of_range("Rate doesn't match standard rates.");
}

//Get current update interval in ms.
int get_update_interval()
{
    _query("GetUpdateInterval");
    return stoi(_reply.value);
}

//Set current update interval in ms.
void set_update_interval(int value)
{
    _query("SetUpdateInterval", to_string(value));
}

//Get current value for rate adjust, 1.0 means 1:1 resampling.
float get_rate_adjust()
{
    _query("GetRateAdjust");
    return stof(_reply.value);
}

//Get current buffer level of the playback device.
int get_buffer_level()
{
    _query("GetBufferLevel");
    return stoi(_reply.value);
}

//Get number of clipped samples since the config was loaded.
int get_clipped_samples()
{
    _query("GetClippedSamples");
    return stoi(_reply.value);
}

//Stop processing and wait for new config if wait mode is active, else exit.
void stop()
{
    _query("Stop");
}

//Stop processing and exit.
void exit()
{
    _query("Exit");
}

//Reload config from disk.
void reload()
{
    _query("Reload");
}

//Get path to current config file.
string get_config_name()
{
    _query("GetConfigName");
    return _reply.value;
}

//Set path to config file.
void set_config_name(string value)
{
    _query("SetConfigName", value);
}

//Get the active configuation in yaml format as a string.
string get_config_raw()
{
    _query("GetConfig");
    return _reply.value;
}

//Upload a new configuation in yaml format as a string.
void set_config_raw(string config_string)
{
    _query("SetConfig", config_string);
}






/*
//Get the active configuation as a Python object.
void get_config()
{
    config_string = self.get_config_raw()
    config_object = yaml.safe_load(config_string)
    return config_object
}

//Get the previously active configuation as a Python object.
void get_previous_config()
{
    config_string = self._query("GetPreviousConfig")
    config_object = yaml.safe_load(config_string)
    return config_object
}

//Read a config from yaml string and return the contents
//as a Python object, with defaults filled out with their default values.
void read_config(config_string)
{
    config_raw = self._query("ReadConfig", arg=config_string)
    config_object = yaml.safe_load(config_raw)
    return config_object
}

//Read a config file from disk and return the contents as a Python object.
void read_config_file(filename)
{
    config_raw = self._query("ReadConfigFile", arg=filename)
    config = yaml.safe_load(config_raw)
    return config
}

//Upload a new configuation from a Python object.
void set_config(config_object)
{
    config_raw = yaml.dump(config_object)
    self.set_config_raw(config_raw)
}

//Validate a configuration object.
//Returns the validated config with all optional fields filled with defaults.
//Raises a CamillaError on errors.
void validate_config(config_object)
{
    config_string = yaml.dump(config_object)
    validated_string = self._query("ValidateConfig", arg=config_string)
    validated_object = yaml.safe_load(validated_string)
    return validated_object
}
*/
