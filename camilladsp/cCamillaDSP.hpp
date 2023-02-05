#include <tuple>
#include <list>
#include <string>

using namespace std;

enum STANDARD_RATES
{
    RATE_8000 = 8000,
    RATE_11025 = 11025,
    RATE_16000 = 16000,
    RATE_22050 = 22050,
    RATE_32000 = 32000,
    RATE_44100 = 44100,
    RATE_48000 = 48000,
    RATE_88200 = 88200,
    RATE_96000 = 96000,
    RATE_176400 = 176400,
    RATE_192000 = 192000,
    RATE_352800 = 352800,
    RATE_384000 = 384000,
    RATE_705600 = 705600,
    RATE_768000 = 768000
};

enum ProcessingState
{
    RUNNING,
    PAUSED,
    INACTIVE,
    STARTING,
    STALLED
};

enum StopReason
{
    NONE,
    DONE,
    CAPTUREERROR,
    PLAYBACKERROR,
    CAPTUREFORMATCHANGE,
    PLAYBACKFORMATCHANGE
};

//Is websocket connected? Returns True or False.
bool is_connected();

//Read CamillaDSP version, returns a tuple of (major, minor, patch).
tuple<int, int, int> get_version();

//Read ccamilladsp version, returns a tuple of (major, minor, patch).
tuple<int, int, int> get_library_version();

//Connect to the websocket of CamillaDSP.
void connect(std::string host, int port);

//Close the connection to the websocket.
void disconnect();

//Read what device types the running CamillaDSP process supports. 
//Returns a tuple with two lists of device types, the first for playback and the second for capture.
tuple<list<string>, list<string>> get_supported_device_types();

//Get current processing state.
ProcessingState get_state();

//Get current stop reason.
StopReason get_stop_reason();

//Get current signal range. Maximum value is 2.0.
float get_signal_range();

//Get current signal range in dB. Full scale is 0 dB.
float get_signal_range_dB();

//Parse the signal levels returned as JSON into a list
list<float> _parse_signal_levels(string message);

//Get capture signal level rms in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_capture_signal_rms();

//Get playback signal level rms in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_playback_signal_rms();

//Get capture signal level peak in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_capture_signal_peak();

//Get playback signal level peak in dB. Full scale is 0 dB. Returns a list with one element per channel.
list<float> get_playback_signal_peak();

//Get current volume setting in dB.
float get_volume();

//Set volume in dB.
void set_volume(float value);

//Get current mute setting.
bool get_mute();

//Set mute, true or false.
void set_mute(bool value);

//Get current capture rate, raw value.
int get_capture_rate_raw();

//Get current capture rate. Returns the nearest common rate, as long as it's within +-4% of the measured value.
STANDARD_RATES get_capture_rate();

//Get current update interval in ms.
int get_update_interval();

//Set current update interval in ms.
void set_update_interval(int value);

//Get current value for rate adjust, 1.0 means 1:1 resampling.
float get_rate_adjust();

//Get current buffer level of the playback device.
int get_buffer_level();

//Get number of clipped samples since the config was loaded.
int get_clipped_samples();

//Stop processing and wait for new config if wait mode is active, else exit.
void stop();

//Stop processing and exit.
void exit();

//Reload config from disk.
void reload();

//Get path to current config file.
string get_config_name();

//Set path to config file.
void set_config_name(string value);

//Get the active configuation in yaml format as a string.
string get_config_raw();

//Upload a new configuation in yaml format as a string.
void set_config_raw(string config_string);






/*
//Get the active configuation as a Python object.
void get_config();

//Get the previously active configuation as a Python object.
void get_previous_config();

//Read a config from yaml string and return the contents
//as a Python object, with defaults filled out with their default values.
void read_config(config_string);

//Read a config file from disk and return the contents as a Python object.
void read_config_file(filename);

//Upload a new configuation from a Python object.
void set_config(config_object);

//Validate a configuration object.
//Returns the validated config with all optional fields filled with defaults.
//Raises a CamillaError on errors.
void validate_config(config_object);
*/
