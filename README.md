# cCamillaDSP
Companion c++ library for CamillaDSP.
This is my attempt at a port of pyCamillaDSP to c++. It should work with CamillaDSP version 1.0.0 and up.

## Dependencies
cCamillaDSP requires easywsclient https://github.com/dhbaird/easywsclient and JSMN https://github.com/zserge/jsmn.

## Communicating with the CamillaDSP process
This library provides an easy way to communicate with CamillaDSP via a websocket.

## TODO
Most of the methods around CamillaDSP configuration management have not yet been implemented.

# Documentation from pyCamillaDSP
## This has not yet been updated to reflect the port to c++

Simple example to connect to CamillaDSP to read the version (assuming CamillaDSP is running on the same machine and listening on port 1234):
```python
from camilladsp import CamillaConnection

cdsp = CamillaConnection("127.0.0.1", 1234)
cdsp.connect()
print("Version: {}".format(cdsp.get_version()))
```

### Classes
All communication functionality is provided by the class CamillaConnection. The contructor accepts two arguments: host and port.
```
CamillaConnection(host, port)
```

### Exceptions

The custom exception `CamillaError` is raised when CamillaDSP replies to a command with an error message. The error message is given as the message of the exception.

Different exceptions are raised in different situations. Consider the following example:
```python
from camilladsp import CamillaConnection, CamillaError
cdsp = CamillaConnection("127.0.0.1", 1234)

myconfig = # get a config from somewhere
try:
    cdsp.connect()
    cdsp.validate_config(myconfig)
except ConnectionRefusedError as e:
    print("Can't connect to CamillaDSP, is it running? Error:", e)
except CamillaError as e:
    print("CamillaDSP replied with error:", e)
except IOError as e:
    print("Websocket is not connected:", e)
```
- `ConnectionRefusedError` means that CamillaDSP isn't responding on the given host and port. 
- `CamillaError` means that the command was sent and CamillaDSP replied with an error.
- `IOError` can mean a few things, but the most likely is that the websocket connection was lost. This happens if the CamillaDSP process exits or is restarted. 


## Methods

The CamillaConnection class provides the following methods

### Basics

| Method   |  Description  |
|----------|---------------|
|`connect()` | Connect to the Websocket server. Must be called before any other method can be used.|
|`disconnect()` | Close the connection to the websocket.|
|`is_connected()` | Is websocket connected? Returns True or False.|
|`get_version()` | Read CamillaDSP version, returns a tuple with 3 elements.|
|`get_library_version()` | Read pyCamillaDSP version, returns a tuple with 3 elements.|
|`get_state()` | Get current processing state. Returns a ProcessingState enum value, see "Enums" below.|
|`get_stop_reason()` | Get the reason that processing stopped. Returns a StopReason enum value, see "Enums" below. |
|`def get_supported_device_types()`| Read what device types the running CamillaDSP process supports. Returns a tuple with two lists of device types, the first for playback and the second for capture. |
|`stop()` | Stop processing and wait for new config if wait mode is active, else exit. |
|`exit()` | Stop processing and exit.|


### Config handling
| Method   |  Description  |
|----------|---------------|
|`reload()` | Reload config from disk.|
|`get_config_name()` | Get path to current config file.|
|`set_config_name(value)` | Set path to config file.|
|`get_config_raw()` | Get the active configuration in yaml format as a string.|
|`set_config_raw(value)` | Upload a new configuration in yaml format as a string.|
|`get_config()` | Get the active configuration as an object.|
|`get_previous_config()` | Get the previously active configuration as an object.|
|`set_config(config)` | Upload a new configuration from an object.|
|`validate_config(config)` | Validate a configuration object. Returns the validated config with all optional fields filled with defaults. Raises a CamillaError on errors.|
|`read_config_file(path)` | Read a config file from `path`. Returns the loaded config with all optional fields filled with defaults. Raises a CamillaError on errors.|
|`read_config(config)` | Read a config from yaml string and return the contents as an object, with defaults filled out with their default values.|

### Reading status
| Method   |  Description  |
|----------|---------------|
|`get_signal_range()` | Get current signal range.|
|`get_signal_range_dB()` | Get current signal range in dB.|
|`get_capture_signal_rms()` | Get capture signal level rms in dB. Full scale is 0 dB. Returns a list with one element per channel.|
|`get_playback_signal_rms()` | Get playback signal level rms in dB. Full scale is 0 dB. Returns a list with one element per channel.|
|`get_capture_signal_peak()` | Get capture signal level peak in dB. Full scale is 0 dB. Returns a list with one element per channel.|
|`get_playback_signal_peak()` | Get playback signal level peak in dB. Full scale is 0 dB. Returns a list with one element per channel.|
|`get_capture_rate_raw()` | Get current capture rate, raw value.|
|`get_capture_rate()` | Get current capture rate. Returns the nearest common rate, as long as it's within +-4% of the measured value.|
|`get_update_interval()` | Get current update interval in ms.|
|`set_update_interval(value)` | Set current update interval in ms.|
|`get_rate_adjust()` | Get current value for rate adjust.|
|`get_buffer_level()` | Get current buffer level of the playback device.|
|`get_clipped_samples()` | Get number of clipped samples since the config was loaded.|

### Volume control
| Method   |  Description  |
|----------|---------------|
|`get_volume()` | Get current volume setting in dB.|
|`set_volume(value)` | Set volume in dB.|
|`get_mute()` | Get current mute setting.|
|`set_mute(value)` | Set mute, true or false.|


## Enums

### ProcessingState
- RUNNING: Processing is running.
- PAUSED: Processing is paused.
- INACTIVE: CamillaDSP is inactive, and waiting for a new config to be supplied.
- STARTING: The processing is being set up.
- STALLED: The processing is stalled because the capture device isn't providing any data.

### StopReason
- NONE: Processing hasn't stopped yet. 
- DONE: The capture device reached the end of the stream.
- CAPTUREERROR: The capture device encountered an error.
- PLAYBACKERROR: The playback device encountered an error.
- CAPTUREFORMATCHANGE: The sample format of the capture device changed. 
- PLAYBACKFORMATCHANGE: The sample format of the playback device changed. 

The StopReason enums also carry additional data:
- CAPTUREERROR and PLAYBACKERROR: Carries the error message as a string.
- CAPTUREFORMATCHANGE and PLAYBACKFORMATCHANGE: Carries the estimated new sample rate as an integer. A value of 0 means the new rate is unknown.

The additional data can be accessed by reading the `data` property:
```python
reason = cdsp.get_stop_reason()
if reason == StopReason.CAPTUREERROR:
    error_msg = reason.data
    print(f"Capture failed, error: {error_msg})
```


# Included examples:

## read_rms
Read the playback signal level continuously and print in the terminal, until stopped by Ctrl+c. 
```sh
python read_rms.py 1234
```

## get_config
Read the configuration and print some parameters. 
```sh
python get_config.py 1234
```

## set_volume
Set the volume control to a new value. First argument is websocket port, second is new volume in dB.
For this to work, CamillaDSP must be running a configuration that has Volume filters in the pipeline for every channel.
```sh
python set_volume.py 1234 -12.3
```

## play_wav
Play a wav file. This example reads a configuration from a file, updates the capture device fto point at a given .wav file, and sends this modified config to CamillaDSP.
Usage example:
```sh
python play_wav.py 1234 /path/to/wavtest.yml /path/to/music.wav
```
