/**
* ls_mcu_remote
* Copyright (C) 2026  Philip Tschiemer
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <chrono>
#include <thread>
#include <iostream>
#include <filesystem>

#include <getopt.h>

#if defined(__APPLE__)
#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#error This example was written for CoreMIDI, so you will have to adapt it for your own system
#endif

#include "Controller.h"

static char * argv0 = nullptr;

static LsMcuRemote::Controller controller;

void probeMidi(bool allowVirtual = false){

#ifndef LIBREMIDI_VERSION
    std::cerr << "probing for midi devices.." << std::endl;
#else

    // On Windows 10, apparently the MIDI devices aren't exactly available as soon as the app open...
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    libremidi::observer midi{
            {.track_hardware = true, .track_virtual = allowVirtual}};

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
        // Check inputs.
        auto ports = midi.get_input_ports();
        std::cout << ports.size() << " MIDI input sources:\n";
        int i = 0;
        for (auto& port : ports){

            std::cout << "IN Port " << i++ << ":" << port.device_name << " (" << port.port_name << ")";

            if (port.type & libremidi::transport_type::hardware){
                std::cout << ", manufacturer " << port.manufacturer;
//                std::cout << ", device id " << port.device;
            }

            std::cout << std::endl;
        }
    }

    {
        // Check outputs.
        auto ports = midi.get_output_ports();
        std::cout << ports.size() << " MIDI output sinks:\n";
        int i = 0;
        for (auto& port : ports){

            std::cout << "OUT Port " << i++ << ":" << port.device_name << " (" << port.port_name << ")";

            if (port.type & libremidi::transport_type::hardware){
                std::cout << ", manufacturer " << port.manufacturer;
//                std::cout << ", device id " << port.device;
            }

            std::cout << std::endl;
        }
    }

#endif //LIBREMIDI
}

void help(){

    printf(
            "Usage: %s <path-to-config-file.json>\n"
            "Usage: %s (-p|-h|-?)\n"
            "Controlling Lightshark remotely via a Mackie Control XT MIDI device\n"
            "Please see config files for explanation of possible options.\n"
            "\nOptions:\n"
            "\t -h, -?        Prints this cute help\n"
            "\t -p            Probe/list MIDI devices\n"
            "\n"
            "Thanks to:\n"
            "libremidi: https://github.com/celtera/libremidi\n"
            "OSCPP: https://github.com/kaoskorobase/oscpp\n"
            "\n"
            "https://github.com/tschiemer/ls-mcu-remote"
            , argv0, argv0);
}

//void enumerate_api(libremidi::API api)
//{
//    std::string_view api_name = libremidi::get_api_display_name(api);
//    std::cout << "Displaying ports for: " << api_name << std::endl;
//
//    // On Windows 10, apparently the MIDI devices aren't exactly available as soon as the app open...
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//    libremidi::observer midi{
//            {.track_hardware = true, .track_virtual = false}, libremidi::observer_configuration_for(api)};
//
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));
//
//    {
//        // Check inputs.
//        auto ports = midi.get_input_ports();
//        std::cout << ports.size() << " MIDI input sources:\n";
//        int i = 0;
//        for (auto& port : ports){
////       std::cout << " - " << i++ << ": " << port << '\n';
//
//            std::cout << "IN Port " << i++ << ":" << port.device_name << " (" << port.port_name << ")";
//
//            std::cout << std::endl;
//        }
//    }
//
//    {
//        // Check outputs.
//        auto ports = midi.get_output_ports();
//        std::cout << ports.size() << " MIDI output sinks:\n";
//        int i = 0;
//        for (auto& port : ports){
//
//            std::cout << "OUT Port " << i++ << ":" << port.device_name << " (" << port.port_name << ")";
//
//
//            std::cout << std::endl;
//        }
////      std::cout << " - " << i++ << ": " << port << '\n';
//    }
//
//    std::cout << "\n";
//}

int main(int argc, char * argv[]){

//    enumerate_api(libremidi::API::UNSPECIFIED);
//
//    libremidi::observer obs{{
//                                        .track_hardware = true,
//                                        .track_virtual = false,
//                                    .input_added = [&](const libremidi::input_port& p) {
//                                        std::cerr << " : input added " << p.port_name << "\n";
//                                    },
//                                    .input_removed = [&](const libremidi::input_port& p) {
//                                        std::cerr << " : input removed " << p.port_name << "\n";
//                                    },
//                                    .output_added = [&](const libremidi::output_port& p) {
//                                        std::cerr << " : output added " << p.port_name << "\n";
//                                    },
//                                    .output_removed = [&](const libremidi::output_port& p) {
//                                        std::cerr << " : output removed " << p.port_name << "\n";
//                                    },
//
//                            }, libremidi::API::COREMIDI};
//    CFRunLoopRun();
//
//    return 0;

    argv0 = argv[0];

    if (argc <= 1){
        help();
        return EXIT_FAILURE;
    }

    int opt;

    while ((opt = getopt(argc, argv, "?hp")) != -1) {
        switch (opt) {
            case 'p':
                probeMidi();
                return EXIT_SUCCESS;
                break;

            case '?':
            case 'h':
                help();
                return EXIT_SUCCESS;

            default: /* '?' */
                printf("Unknown option: %c\n", opt);
                help();
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc){
        printf("ERROR missing argument\n");
        help();
        return EXIT_FAILURE;
    }

    const char * configFile = argv[optind];

    if (!std::filesystem::exists(configFile)){
        fprintf(stderr, "Config file not found: %s\n", configFile);
        return EXIT_FAILURE;
    }

    controller.initFromConfig(configFile);

    controller.start();

    controller.runloop();

    controller.stop();

    return EXIT_SUCCESS;
}