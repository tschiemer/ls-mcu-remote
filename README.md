# ls-mcu-remote
Controlling Lightshark remotely via Mackie Control MIDI devices.

Want to use a physical surface for lightshark and be physically placed anywhere?
This software allows you to use standard MIDI MCU devices (with motorized faders) to remote control Lightshark (Cores).

Platform independet libraries were used but was solely written and tested on macOS 15.7.7. So, might need some adjustments for other platforms.

This software was quickly written to serve myself to the degree I need it upgrading existing hardware without any intention of further development.
Pull requests for fixes or extensions generally welcome, bug reports ok but I may not bother to resolve anything, help yourself. 

https://github.com/tschiemer/ls-mcu-remote


**Table of Contents**

- [Using](#using)
- [Known Lightshark Bug](#known-lightshark-bug)
- [Configuration file](#configuration-file)
  - [Lightshark connection](#lightshark-connection)
  - [(Playback) Banks](#-playback--banks)
  - [Buttons](#buttons)
  - [Devices](#devices)
- [Building](#building)
- [Third Party Libraries](#third-party-libraries)
- [License](#license)


## Using 

```
Usage: ls-mcu-remote <path-to-config-file.json>
Usage: ls-mcu-remote (-p|-h|-?)

Controlling Lightshark remotely via a Mackie Control XT MIDI device
Please see config files for explanation of possible options.

Options:
	 -h, -?        Prints this cute help
	 -p            Probe/list MIDI devices

```

Example `ls-mcu-remote -p` output (use the value in the brackets `X-TOUCH_INT` for the config file):
```
1 MIDI input sources:
IN Port 0:X-Touch-Ext (X-TOUCH_INT), manufacturer BEHRINGER
1 MIDI output sinks:
OUT Port 0:X-Touch-Ext (X-TOUCH_INT), manufacturer BEHRINGER
```

## Known Lightshark Bug

Lightshark version 1.16.01 (the most current at time of writing) has a bug for
all OSC commands that set a level. Only specific values (ie 9-13 + 32) are
concerned. But this affects the surface interaction.

The bug is mentioned but **not bypassed in this library / app.**

Also see [the bug report](https://community.lightshark.es/c/q_a/osc-too-many-responses-or-inconsistent-format#comment_wrapper_109567879).

## Configuration file

For a working example see file [config/default.json](config/default.json)

Note:
- If the file is not valid JSON or it does not respect the datatypes as defined, an error is thrown and the execution aborted.
- If the file is valid JSON and it respects the datatypes as defined but invalid data ranges are given the program may just quit at some point.  

Basic structure as follows, some explanations below
```json
{
  "lightshark": {
    "ip": "10.0.0.10",
    "port": 8000,
    "remotePort": 9000
  },
  "banks": [
    ["pb1","pb2","pb3","pb4","pb5","pb6","pb7","pb8","gm"],
    ["pb9","pb10","pb11","pb12","pb13","pb14","pb15","pb16","gm"],
    ["pb17","pb18","pb19","pb20","pb21","pb22","pb23","pb24","gm"],
    ["pb25","pb26","pb27","pb28","pb29","pb30", null, null, "gm"],
    ["chase", "fxSize", "fxSpeed", null, null, null, null,"gm","gm"]
  ],

  "buttonLayers": [{

    "vpot_click_0": {
      "fun": "clear"
    },
    ...
   }],
   
  "devices": {
    "X-TOUCH_INT":
    {
    "_comment": "the object* name is the midi port name which will be used to identify the device",

    "deviceType": "mackieControlXT",

    "banks" : {
      "fixed": false,
      "offset": 0
    },

    "buttonLayer": 0,

    "vpots" : ["encoder1", "encoder2", "encoder3", "encoder4", "select", "buttonLayer", "page", "bank"]
  }
}
}
```

### Lightshark connection

```json
"lightshark": {
  "ip": "10.0.0.10",
  "port": 8000,
  "remotePort": 9000
}
```

Note that `port` refers to the port on which lightshark receives OSC messages and `remotePort` is the local port to which lightshark sends any OSC messages.

In particular, enter exactly as seen in the corresponding configuration view of lightshark.

### (Playback) Banks

Field key `banks`

You can define the playback arrangement and the number of banks.

In particular:
- you could use only one fixed bank for all or a specific device.
- you could arbitrarily rearrange the order of playbacks.
- These banks are shared among all devices!
- devices can either be fixed to a particular bank or are affected by bank change commands.
- devices can be given a bank offset (ex. device one starts at bank 0, device two at bank 1 so you would get 16 consecutive playbacks)

The `bank` field must be an array of arrays of _8 or 9_ playback identifiers. The position within the array defines 

Any per-device bank-specific configuration (fixed, offset, ..) is defined in the [device object](#device).


| Playback identifiers | Meaning
| ----------------------| ---
| *null*               | No Playback assigned
| `pbXY`               | Normal playback faders XY where XY is an integer in 1-30
| `chase`              | Playback of chase speed master
| `fxSize`             | Playback of FX size master
| `fxSpeed`            | Playback of FX speed master
| `gm`                 | grandmaster

```json
"banks": [
  ["pb1","pb2","pb3","pb4","pb5","pb6","pb7","pb8","gm"],
  ["pb9","pb10","pb11","pb12","pb13","pb14","pb15","pb16","gm"],
  ["pb17","pb18","pb19","pb20","pb21","pb22","pb23","pb24","gm"],
  ["pb25","pb26","pb27","pb28","pb29","pb30", null, null, "gm"],
  ["chase", "fxSize", "fxSpeed", null, null, null, null,"gm","gm"]
]
```

### Buttons

Buttons are structured as banks/layers of button maps such that MCU device button functions are mapped to actions.

Devices can be assigned specific button layers (also see [devices](#devices))

Trivial example of two layers with two buttons each where:

- On layer 0
  - the *Select channel 1* button on the MCU device corresponds to lightshark clear button
  - the *Select channel 2* button on the MCU device corresponds to lightshark executor button on page 1 column 2 row 3
- On layer 1
  - the *Select channel 1* button on the MCU device corresponds to lightshark DBO button
  - the *Select channel 2* button on the MCU device corresponds to the select playback N of lightshark button, where playback N is defined as the playback with offset 1 of the current bank.   

```json
  "buttonLayers": [
    {
      "sel_0": {
        "fun": "clear"
       },
      "sel_1": {
        "fun": "executor",
        "target": 123
       },
    },
    {
      "sel_0": {
        "fun": "dbo"
       },
      "sel_1": {
        "fun": "select",
        "target": 1
       },
    }
  }
```

#### Format

```
"buttonLayers": [ <button-layer0>, <button-layer1>, ... ]
  
<button-layer> := {
                    "<button-id>" : <button-action>
                  }
                  
<button-action> := {
                      "fun": <action>[,
                      "target": <target>]
                   }
```

Button actions mostly map directly to lightshark buttons.
Functionality rather straightforward.

A list of possible button keys follows. These match the buttons found on mackie control devices.

| Button identifier | 
| --- |
|
| vpot_click_0
| vpot_click_1
| vpot_click_2
| vpot_click_3
| vpot_click_4
| vpot_click_5
| vpot_click_6
| vpot_click_7
|
| rec_0
| rec_1
| rec_2
| rec_3
| rec_4
| rec_5
| rec_6
| rec_7
|
| solo_0
| solo_1
| solo_2
| solo_3
| solo_4
| solo_5
| solo_6
| solo_7
|
| mute_0
| mute_1
| mute_2
| mute_3
| mute_4
| mute_5
| mute_6
| mute_7
|
| sel_0
| sel_1
| sel_2
| sel_3
| sel_4
| sel_5
| sel_6
| sel_7
|
| assign_track
| assign_send
| assign_pan
| assign_plugin
| assign_eq
| assign_instrument
|
| bank_left
| bank_right
| channel_left
| channel_right
| flip
| global
|
| name_value_button
| smpte_beats_button
|
| f1
| f2
| f3
| f4
| f5
| f6
| f7
| f8
|
| midi_tracks
| inputs
| audio_tracks
| audio_instruments
| aux
| busses
| outputs
| user
|
| shift
| option
| control
| alt
|
| save
| undo
| cancel
| enter
|
| markers
| nudge
| cycle
| drop
| replace
| click
| solo
|
| rewind
| forward
| stop
| play
| record
|
| up
| down
| left
| right
| zoom
| scrub
|
| user_switch_1
| user_switch_2
|
| fader_touched_0
| fader_touched_1
| fader_touched_2
| fader_touched_3
| fader_touched_4
| fader_touched_5
| fader_touched_6
| fader_touched_7
| fader_touched_master
|
| smpte_led
| beats_led
| rude_solo_led
|
| relay_click

A list of possible actions follows, where no target is mentioned, none is needed. 

Please note that all (or most) buttons with assigned functions will visually respond to being pressed.  In particular:
- buttons bound to executors will show the executor state (any delay is due to sync interval)

| Action identifier | Target  | 
|----------------------------|--------|
| pageUp                     |
| pageDown                   |        
||
| dbo                        |
||
| edit                       |        
| update                     |
| delete                     |
| copy                       |
| move                       |
| set                        |
| fan                        |        
|| 
| clear                      |
| rec                        |
| find                       |
||
| select                     |
||
| go                         | If no target given corresponds to lightshark's master GO button (which is the GO button of the currently selected playback).
|  | If a target (an integer offset in [0,7]) is given, the GO button belongs to the playback of the current bank with given offset (ex. if target = 0 and the current bank has playback 1 at offset 0, then it is the GO button of playback 1) 
| release                    | like go
| pause                      | like go
| nextCue                    | like go
| previousCue                | like go
| tap                        | like go
||
| selectFixture              |
| selectGroup                |
| selectNext                 |
| selectPrevious             |
||
| intensity                  |
| position                   |
| color                      |
| beam                       |
| advanced                   |
| gobo                       |
| fx                         |
||
| chaseReset                 |
| chaseTap                   |
| fxSizeReset                |
| fxSpeedReset               |
| fxSpeedTap                 |
||
| executor                   | Integer index of executor such that `target := 100 x page + 10 x column + row` (example: Page 1, Column 5, Row 2 -> 152)
| executorRow                | Integer index of executor row such that `target := 100 x page + row` (example: Page 1,  Row 2 -> 102)
||
| nextBank                   |
| previousBank               |
| selectBank                 | Index of bank to go to. ill memorize which was the last bank, if already at selected bank, will go to last. (useful to have a button to go to a master layer) 


### Devices

```json
"devices": {
  "X-TOUCH_INT": {
    "deviceType": "mackieControlXT",
    "banks": {
      "fixed": false,
      "offset": 0
    },
    "buttonLayer": 0,
    "vpots": ["encoder1", "encoder2", "encoder3", "encoder4", "select", "buttonLayer", "page", "bank"]
 },
 "<port-name-of-MCU-device2>": {
  }
}
```

The `devices` field is a mapped list where the device's portname that is used to identify the device to connect to serves as map-key.

The fields of a device object are as follows:

| key        | description
|------------| --- |
| deviceType | (Optional) String value in `["logicControl","logicControlXT","mackieControl" (default),"mackieControlXT"]`
|            | Note that if the type is wrong, it likely will not work.
| banks.fixed | (Optional) boolean, `default = false`. If `true` bank can not be changed. If `false` will perform bank changes along with all other devices.
| banks.offset | (Optional) int, `default = 0`. If given will use this bank offset (ex. a value of 1 will make it start at the second bank)
| buttonLayer | (Optional) int, `default = 0`. If given will start at this button layer.
| vpots | Required array of 8 elements of function Ids as defined below at VPOT Encoder Functions, the ith element will define the function of the ith encoder/vpot.

**VPot Encoder Functions**

| Encoder function id | description
|---------------------| ---
| *null*            | No function
| encoder1            | Lightshark encoder 1
| encoder2            | 
| encoder3            | 
| encoder4            | 
| grandmaster | Lightshark grandmaster
| chase | Chase speed master
| fxSize | FX size master
| fxSpeed | FX speed master
| select | Select previous or next fixture/group (like the lightshark buttons previous/next)
| bank | Bank up or down
| page | Lightshark page up or down
| buttonLayer | Change this devices button layer to previous or next 


## Building

Building the project standalone should be straightforward using cmake.

A library interface is exposed and it should be possible to include it easoly as follows (more or less)

```cmake

include(FetchContent)
FetchContent_Declare(ls-mcu-remote
        GIT_REPOSITORY https://github.com/tschiemer/ls-mcu-remote.git
        GIT_TAG        master
        )
FetchContent_MakeAvailable(ls-mcu-remote)


target_link_libraries(my-target-woot PUBLIC ls-mcu-remote::lsmcuremote)

```


## Third Party Libraries

- [libremidi](https://github.com/celtera/libremidi)
- [oscpp](https://github.com/kaoskorobase/oscpp)
- [nlohmann::json](https://github.com/nlohmann/json)
- [asio](https://github.com/chriskohlhoff/asio.git)


## License

Copyright (C) 2026 Philip Tschiemer

[GNU Affero General Public License v3](LICENSE)
