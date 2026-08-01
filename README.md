# ls-mcu-remote
Controlling Lightshark remotely via Mackie Control MIDI devices.

Want to use a physical surface for lightshark and be physically placed anywhere?
This software allows you to use standard MIDI MCU devices (with motorized faders) to remote control Lightshark (Cores).

A library encompassing lightshark specific defines and a proxy interface are exposed and can be used in your own projects. Also see [building](#building).

Platform independent libraries were used but was solely written and tested on macOS 15.7.7. So, might need some adjustments for other platforms.

This software was quickly written to serve myself to the degree I need it upgrading existing hardware without any intention of further development.
Pull requests for fixes or extensions generally welcome, bug reports ok but I may not bother to resolve anything, help yourself. 

https://github.com/tschiemer/ls-mcu-remote

Tested devices:
- Behringer X-TOUCH Extender

----

**Table of Contents**

- [Using](#using)
  - [Default config](#default-config)
- [Known Lightshark Bug](#known-lightshark-bug)
- [Configuration file](#configuration-file)
  - [Lightshark connection](#lightshark-connection)
  - [(Playback) Banks](#-playback--banks)
  - [Buttons](#buttons)
  - [Devices](#devices)
- [Building](#building)
- [Third Party Libraries](#third-party-libraries)
- [License](#license)

---

## Using  

```
Usage: ls-mcu-remote <path-to-config-file.json>
Usage: ls-mcu-remote (-p|-h|-?)

Controlling Lightshark remotely via a Mackie Control XT MIDI device
Please see config files for explanation of possible options.

Options:
	 -h, -?        Prints this cute help
	 -p            Probe/list MIDI devices

Thanks to:
libremidi: https://github.com/celtera/libremidi
OSCPP: https://github.com/kaoskorobase/oscpp
nlohmann::json: https://github.com/nlohmann/json
asio: https://github.com/chriskohlhoff/asio.git

ls-mcu-remote-0.1.0
https://github.com/tschiemer/ls-mcu-remote
```

Example `ls-mcu-remote -p` output (use the value in the brackets `X-TOUCH_INT` for the config file):
```
1 MIDI input sources:
IN Port 0:X-Touch-Ext (X-TOUCH_INT), manufacturer BEHRINGER
1 MIDI output sinks:
OUT Port 0:X-Touch-Ext (X-TOUCH_INT), manufacturer BEHRINGER
```

### Default config

The default config file that comes with the project is configured as follows.

**Lightshark**

Likely you'll have to change the IP address of the lightshark device in your network.

```json
"lightshark": {
    "host": "10.0.0.10",
    "port": 8000,
    "remotePort": 9000,
    "syncIntervalMs": 100
  }
```
***Option: `syncIntervalMs`***

Defines the interval in milliseconds when sync requests are sent to lightshark.

To give a sense of meaningful feedback a lower value is meaningful.

But be aware, that lower values also mean that the complete state of lightshark is transmitted over the network at this interval creating more traffic.

If `syncIntervalMs = 0` then there is no automatic sync. You could use the button function `sync` instead, if you want.



**Banks**

| Bank #  | 0    | 1     | 2     | 3     | 4                  | 
|---------|------|-------|-------|-------|--------------------|
| Fader 1 | PB 1 | PB 9  | PB 17 | PB 25 | FX speed master    |
| Fader 2 | PB 2 | PB 10 | PB 18 | PB 26 | FX size master     |
| Fader 3 | PB 3 | PB 11 | PB 19 | PB 27 |                    |
| Fader 4 | PB 4 | PB 12 | PB 20 | PB 28 | Chase speed master |
| Fader 5 | PB 5 | PB 13 | PB 21 | PB 29 |                    |
| Fader 6 | PB 6 | PB 14 | PB 22 | PB 30 |                    |
| Fader 7 | PB 7 | PB 15 | PB 23 |       |                    |
| Fader 8 | PB 8 | PB 16 | PB 24 |       | GM                 |
| Fader 9 | GM   | GM    | GM    | GM    | GM                 |

**Button layers**

| Button   | Layer 1                  | Layer 2                  | Layer 3           |
|----------|--------------------------|--------------------------|-------------------|
| Vpot 1   | Clear                    | Clear                    | Clear             |
| Vpot 2   | Rec                      | Rec                      | Rec               |
| Vpot 3   | Find                     | Find                     | Find              |
| Vpot 4   |                          |                          |                   |
| Vpot 5   |                          |                          |                   |
| Vpot 6   |                          |                          |                   |
| Vpot 7   |                          |                          |                   |
| Vpot 8   | Go to bank 4             | Go to bank 4             | Go to bank 4      |
| Rec 1    | Executor 111             | Executor 211             | GO PB 1           |
| Rec 2    | Executor 121             | Executor 211             | GO PB 2           |
| Rec 3    | Executor 131             | Executor 211             | GO PB 3           |
| Rec 4    | Executor 141             | Executor 211             | GO PB 4           |
| Rec 5    | Executor 151             | Executor 211             | GO PB 5           |
| Rec 6    | Executor 161             | Executor 211             | GO PB 6           |
| Rec 7    | Executor 171             | Executor 211             | GO PB 7           |
| Rec 8    | GO selected PB           | GO selected PB           | GO PB 8           |
| Solo 1   | Executor 112             | Executor 212             | Previous Cue PB 1 |
| Solo 2   | Executor 122             | Executor 212             | Previous Cue PB 2 |
| Solo 3   | Executor 132             | Executor 212             | Previous Cue PB 3 |
| Solo 4   | Executor 142             | Executor 212             | Previous Cue PB 4 |
| Solo 5   | Executor 152             | Executor 212             | Previous Cue PB 5 |
| Solo 6   | Executor 162             | Executor 212             | Previous Cue PB 6 |
| Solo 7   | Executor 172             | Executor 212             | Previous Cue PB 7 |
| Solo 8   | Previous Cue selected PB | Previous Cue selected PB | Previous Cue PB 8 |
| Mute 1   | Executor 113             | Executor 213             | Release PB 1      |
| Mute 2   | Executor 123             | Executor 213             | Release PB 2      |
| Mute 3   | Executor 133             | Executor 213             | Release PB 3      |
| Mute 4   | Executor 143             | Executor 213             | Release PB 4      |
| Mute 5   | Executor 153             | Executor 213             | Release PB 5      |
| Mute 6   | Executor 163             | Executor 213             | Release PB 6      |
| Mute 7   | Executor 173             | Executor 213             | Release PB 7      |
| Mute 8   | Release selected PB      | Release selected PB      | Release PB 8      |
| Select 1 | Select PB 1              | Select PB 1              | Select PB 1       |
| Select 2 | Select PB 2              | Select PB 2              | Select PB 2       |
| Select 3 | Select PB 3              | Select PB 3              | Select PB 3       |
| Select 4 | Select PB 4              | Select PB 4              | Select PB 4       |
| Select 5 | Select PB 5              | Select PB 5              | Select PB 5       |
| Select 6 | Select PB 6              | Select PB 6              | Select PB 6       |
| Select 7 | Select PB 7              | Select PB 7              | Select PB 7       |
| Select 8 | Select PB 8              | Select PB 8              | Select PB 8       |

**Device**

```json
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
    "host": "10.0.0.10",
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
    }
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
  "host": "10.0.0.10",
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


| Playback identifiers | Meaning                                                  |
|----------------------|----------------------------------------------------------|
| *null*               | No Playback assigned                                     |
| `pbXY`               | Normal playback faders XY where XY is an integer in 1-30 |
| `chase`              | Playback of chase speed master                           |
| `fxSize`             | Playback of FX size master                               |
| `fxSpeed`            | Playback of FX speed master                              |
| `gm`                 | grandmaster                                              |

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

| Button identifier    | 
|----------------------|
| vpot_click_0         |
| vpot_click_1         |
| vpot_click_2         |
| vpot_click_3         |
| vpot_click_4         |
| vpot_click_5         |
| vpot_click_6         |
| vpot_click_7         |
||
| rec_0                |
| rec_1                |
| rec_2                |
| rec_3                |
| rec_4                |
| rec_5                |
| rec_6                |
| rec_7                |
||
| solo_0               |
| solo_1               |
| solo_2               |
| solo_3               |
| solo_4               |
| solo_5               |
| solo_6               |
| solo_7               |
||
| mute_0               |
| mute_1               |
| mute_2               |
| mute_3               |
| mute_4               |
| mute_5               |
| mute_6               |
| mute_7               |
||
| sel_0                |
| sel_1                |
| sel_2                |
| sel_3                |
| sel_4                |
| sel_5                |
| sel_6                |
| sel_7                |
||
| assign_track         |
| assign_send          |
| assign_pan           |
| assign_plugin        |
| assign_eq            |
| assign_instrument    |
||
| bank_left            |
| bank_right           |
| channel_left         |
| channel_right        |
| flip                 |
| global               |
||
| name_value_button    |
| smpte_beats_button   |
||
| f1                   |
| f2                   |
| f3                   |
| f4                   |
| f5                   |
| f6                   |
| f7                   |
| f8                   |
||
| midi_tracks          |
| inputs               |
| audio_tracks         |
| audio_instruments    |
| aux                  |
| busses               |
| outputs              |
| user                 |
||
| shift                |
| option               |
| control              |
| alt                  |
||
| save                 |
| undo                 |
| cancel               |
| enter                |
||
| markers              |
| nudge                |
| cycle                |
| drop                 |
| replace              |
| click                |
| solo                 |
||
| rewind               |
| forward              |
| stop                 |
| play                 |
| record               |
||
| up                   |
| down                 |
| left                 |
| right                |
| zoom                 |
| scrub                |
||
| user_switch_1        |
| user_switch_2        |
||
| fader_touched_0      |
| fader_touched_1      |
| fader_touched_2      |
| fader_touched_3      |
| fader_touched_4      |
| fader_touched_5      |
| fader_touched_6      |
| fader_touched_7      |
| fader_touched_master |
||
| smpte_led            |
| beats_led            |
| rude_solo_led        |
||
| relay_click          |

A list of possible actions follows, where no target is mentioned, none is needed. 

Please note that all (or most) buttons with assigned functions will visually respond to being pressed.  In particular:
- buttons bound to executors will show the executor state (any delay is due to sync interval)

| Action identifier | Target                                                                                                                                                                                                                                     | 
|-------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| pageUp            |                                                                                                                                                                                                                                            |
| pageDown          |                                                                                                                                                                                                                                            |
||
| dbo               |                                                                                                                                                                                                                                            |
||
| edit              |                                                                                                                                                                                                                                            |
| update            |                                                                                                                                                                                                                                            |
| delete            |                                                                                                                                                                                                                                            |
| copy              |                                                                                                                                                                                                                                            |
| move              |                                                                                                                                                                                                                                            |
| set               |                                                                                                                                                                                                                                            |
| fan               |                                                                                                                                                                                                                                            |
||
| clear             |                                                                                                                                                                                                                                            |
| rec               |                                                                                                                                                                                                                                            |
| find              |                                                                                                                                                                                                                                            |
||
| select            |                                                                                                                                                                                                                                            |
||
| go                | If no target given corresponds to lightshark's master GO button (which is the GO button of the currently selected playback).                                                                                                               |
|                   | If a target (an integer offset in [0,7]) is given, the GO button belongs to the playback of the current bank with given offset (ex. if target = 0 and the current bank has playback 1 at offset 0, then it is the GO button of playback 1) |
| release           | like go                                                                                                                                                                                                                                    |
| pause             | like go                                                                                                                                                                                                                                    |
| nextCue           | like go                                                                                                                                                                                                                                    |
| previousCue       | like go                                                                                                                                                                                                                                    |
| tap               | like go                                                                                                                                                                                                                                    |
||
| selectFixture     |                                                                                                                                                                                                                                            |
| selectGroup       |                                                                                                                                                                                                                                            |
| selectNext        |                                                                                                                                                                                                                                            |
| selectPrevious    |                                                                                                                                                                                                                                            |
||
| intensity         |                                                                                                                                                                                                                                            |
| position          |                                                                                                                                                                                                                                            |
| color             |                                                                                                                                                                                                                                            |
| beam              |                                                                                                                                                                                                                                            |
| advanced          |                                                                                                                                                                                                                                            |
| gobo              |                                                                                                                                                                                                                                            |
| fx                |                                                                                                                                                                                                                                            |
||
| chaseReset        |                                                                                                                                                                                                                                            |
| chaseTap          |                                                                                                                                                                                                                                            |
| fxSizeReset       |                                                                                                                                                                                                                                            |
| fxSpeedReset      |                                                                                                                                                                                                                                            |
| fxSpeedTap        |                                                                                                                                                                                                                                            |
||
| executor          | Integer index of executor such that `target := 100 x page + 10 x column + row` (example: Page 1, Column 5, Row 2 -> 152)                                                                                                                   |
| executorRow       | Integer index of executor row such that `target := 100 x page + row` (example: Page 1,  Row 2 -> 102)                                                                                                                                      |
||
| nextBank          |                                                                                                                                                                                                                                            |
| previousBank      |                                                                                                                                                                                                                                            |
| selectBank        | Index of bank to go to. ill memorize which was the last bank, if already at selected bank, will go to last. (useful to have a button to go to a master layer)                                                                              |
| sync              | Sends sync request to lightshark (only meaningful if automatic sync is disabled)                                                                                                                                                           |

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

| key          | description                                                                                                                                                |
|--------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|
| deviceType   | (Optional) String value in `["logicControl","logicControlXT","mackieControl" (default),"mackieControlXT"]`                                                 |
|              | Note that if the type is wrong, it likely will not work.                                                                                                   |
| banks.fixed  | (Optional) boolean, `default = false`. If `true` bank can not be changed. If `false` will perform bank changes along with all other devices.               |
| banks.offset | (Optional) int, `default = 0`. If given will use this bank offset (ex. a value of 1 will make it start at the second bank)                                 |
| buttonLayer  | (Optional) int, `default = 0`. If given will start at this button layer.                                                                                   |
| vpots        | Required array of 8 elements of function Ids as defined below at VPOT Encoder Functions, the ith element will define the function of the ith encoder/vpot. |

**VPot Encoder Functions**

| Encoder function id | description                                                                       |
|---------------------|-----------------------------------------------------------------------------------|
| *null*              | No function                                                                       |
| encoder1            | Lightshark encoder 1                                                              |
| encoder2            |                                                                                   |
| encoder3            |                                                                                   |
| encoder4            |                                                                                   |
| grandmaster         | Lightshark grandmaster                                                            |
| chase               | Chase speed master                                                                |
| fxSize              | FX size master                                                                    |
| fxSpeed             | FX speed master                                                                   |
| select              | Select previous or next fixture/group (like the lightshark buttons previous/next) |
| bank                | Bank up or down                                                                   |
| page                | Lightshark page up or down                                                        |
| buttonLayer         | Change this devices button layer to previous or next                              |

## Building

---
***Dependency Note: Awaiting library patch***

Some pull request for the used library libremidi is still pending.

In this code version some local changes were used not available here.

In Effect: **this source code likely won't quite work for you (yet)** and will have to be fixed up a bit after the [pull request](https://github.com/celtera/libremidi/pull/235) has been merged.

---


Building the project standalone should be straightforward using cmake.

Check out the files in [src/include/ls-mcu-remote](src/include/ls-mcu-remote) to get an idea of the interface.

To use the library in your own projects you could (more or less) do as follows:

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

Many thanks to all the people who in one way or other have contributed to the following libraries used in this project:

- [libremidi](https://github.com/celtera/libremidi)
- [oscpp](https://github.com/kaoskorobase/oscpp)
- [nlohmann::json](https://github.com/nlohmann/json)
- [asio](https://github.com/chriskohlhoff/asio.git)


## License

Copyright (C) 2026 Philip Tschiemer

[GNU Affero General Public License v3](LICENSE)
