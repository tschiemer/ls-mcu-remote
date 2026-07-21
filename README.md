# ls-mcu-remote
`Controlling Lightshark remotely via a Mackie Control XT MIDI device`

https://github.com/tschiemer/ls-mcu-remote

## Known Lightshark Bug

Lightshark version 1.16.01 (the most current at time of writing) has a bug for
all OSC commands that set a level. Only specific values (ie 9-13 + 32) are
concerned. But this affects the surface interaction.

The bug is mentioned but not bypassed in this library / app.

Also see [the bug report](https://community.lightshark.es/c/q_a/osc-too-many-responses-or-inconsistent-format#comment_wrapper_109567879).


## License

Copyright (C) 2026 Philip Tschiemer

[GNU Affero General Public License v3](LICENSE)

## Third Party

- [libremidi](https://github.com/celtera/libremidi)
- [oscpp](https://github.com/kaoskorobase/oscpp)
- [nlohmann::json](https://github.com/nlohmann/json)

