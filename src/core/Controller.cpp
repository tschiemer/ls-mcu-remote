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

#include "Controller.h"

#include <utility>

#if defined(__APPLE__)
#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#error This example was written for CoreMIDI, so you will have to adapt it for your own system
#endif

#include <iostream>
#include <fstream>


// in case the feature set is not merged yet in the master branch
#ifdef libremidi::remote_control_protocol::which_mixer_control_type

using which_mixer_control_type = libremidi::remote_control_protocol::which_mixer_control_type;
using which_mixer_control_index = libremidi::remote_control_protocol::which_mixer_control_index;
using relative_midi_to_value = libremidi::remote_control_protocol::relative_midi_to_value;

#else

using mixer_control = libremidi::remote_control_protocol::mixer_control;

#define type_vpot_rotation vpot_rotation_0
#define type_vpot_led vpot_led_0
#define type_timecode_digit timecode_digit_0
#define type_assignment_digit assignment_digit_0
#define type_other external_control

static mixer_control which_mixer_control_type(mixer_control ctl)
{
    if (mixer_control::vpot_rotation_0 <= ctl && ctl <= mixer_control::vpot_rotation_7) return mixer_control::type_vpot_rotation;
    if (mixer_control::vpot_led_0 <= ctl && ctl <= mixer_control::vpot_led_7) return mixer_control::type_vpot_led;
    if (mixer_control::timecode_digit_0 <= ctl && ctl <= mixer_control::timecode_digit_9) return mixer_control::type_timecode_digit;
    if (mixer_control::assignment_digit_0 <= ctl && ctl <= mixer_control::assignment_digit_1) return mixer_control::type_assignment_digit;

    return mixer_control::type_other;
}

static inline int which_mixer_control_index(mixer_control type, mixer_control ctl)
{
    return (uint8_t)ctl - (uint8_t)type;
}


static inline int which_mixer_control_index(mixer_control type, uint8_t ctl_byte)
{
    return which_mixer_control_index(type, static_cast<mixer_control>(ctl_byte));
}

static inline int relative_midi_to_value(int midi){
    if (midi < 0b010000)
        return midi;

    return (0b01000000 - midi);
}
#endif // libremidi::which_mixer_control_type



namespace LsMcuRemote {


//    Controller::Controller(){
//
//        std::cout << "Creating MIDIClient.." << std::endl;
//
//        auto res = MIDIClientCreate(CFSTR("My App"), 0, 0, &handle);
//        if (res != noErr)
//            throw std::runtime_error("Could not start CoreMIDI");
//    }
//
//    Controller::~Controller(){
//
//        std::cout << "Disposing of MIDIClient" << std::endl;
//        MIDIClientDispose(handle);
//    }

    void Controller::initLsProxy(){

        lsProxy_.configure({
           .lightsharkHostIp = data_.lightshark.ip,
           .lightsharkPort = data_.lightshark.port,
           .localPort = data_.lightshark.remotePort,
//                .syncInterval = LsProxy::NoAutoSync
//                .syncInterval = std::chrono::milliseconds(500),
           .onGrandmasterSync = [&](PlaybackLevel_t level){
               if (state_ != State::Running)
                   return;

               // do nothing at the moment
           },
           .onSubmasterSync = [&](Submasters sm, PlaybackLevel_t level){

               if (state_ != State::Running)
                   return;
           },
           .onPageSync = [&](int page){

               if (state_ != State::Running)
                   return;
           },
           .onPlaybackSync = [&](int pb, PlaybackLevel_t level, bool is_active){

               if (state_ != State::Running)
                   return;
           },
           .onExecutorSync = [&](int page, int col, int row, bool is_active){

               if (state_ != State::Running)
                   return;
           }
        });

        lsProxy_.start();

        while(lsProxy_.state() == LsProxy::State::Starting)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (lsProxy_.state() != LsProxy::State::Running)
            throw new std::runtime_error("Failed to start LsProxy");

    }

    void Controller::deinitLsProxy(){
        lsProxy_.stop();
    }

    void Controller::initMidiDevices(){

//        std::cerr << "initMidiDevices()" << std::endl;
//        std::cerr << "Controller ptr " << this << std::endl;

        for(auto && [portName, midiDevice] : data_.devices){

//            midiDevice.controller_ = shared_from;

            midiDevice.midiIn_ = std::make_shared<libremidi::midi_in>(libremidi::midi_in{{
                 .on_message = [&](const libremidi::message& message) {

                     std::cerr << "in " << message.size() << std::endl;
                         if (state_ == State::Running) {
                             midiDevice.mcu_->on_midi(message);
                         }
                     }
             }});

            midiDevice.midiOut_ = std::make_shared<libremidi::midi_out>();

            midiDevice.mcu_ = std::make_shared<libremidi::remote_control_processor>(libremidi::remote_control_processor{
                    {
//                device_type = midiDevice.deviceType;
                            .midi_out = [&](libremidi::message &&msg) {
                                if (state_ == State::Running && midiDevice.midiOut_->is_port_open())
                                    midiDevice.midiOut_->send_message(msg);
                            },
                            .on_command = [&](libremidi::remote_control_protocol::mixer_command cmd, bool pressed) {
                                if (state_ != State::Running)
                                    return;

                                std::cerr << "command: " <<(int)cmd << " -> " << (pressed ? "pressed" : "released") << "\n";
                            },
                            .on_control = [&](libremidi::remote_control_protocol::mixer_control ctl, int v) {
                                if (state_ != State::Running)
                                    return;

                                std::cerr << "control: " << (int)ctl << " -> " << v << "\n";

                                auto type = which_mixer_control_type(ctl);
                                auto index = which_mixer_control_index(type, ctl);

                                switch(type){
                                    case mixer_control::type_vpot_rotation:
                                        {
                                            int rvalue = relative_midi_to_value(v);
                                            midiDeviceEncoder(midiDevice, midiDevice.vpots[index].encoder, rvalue);
                                        }
                                        break;
                                    default:
                                        break;
                                }

                            },
                            //                .on_fader = [](uint8_t fader, uint16_t v) {
                            .on_fader = [&](libremidi::remote_control_protocol::fader fader, uint16_t level) {
                                if (state_ != State::Running)
                                    return;

                                std::cerr << "fader: " << (int) fader << " -> " << level << "\n";


                            }
                    }
            });
            std::cerr << "midiDevice ptr " << midiDevice.mcu_ << std::endl;
        }

    }

    void Controller::deinitMidiDevices(){

        for(auto &[portName,midiDevice] : data_.devices){
//            midiDevice.controller_ = nullptr;

//            if (midiDevice.mcu_){
//                delete midiDevice.mcu_;
//                midiDevice.mcu_ = nullptr;
//            }
        }
    }

    void Controller::initMidiObserver(){

        midiObserver_ = std::make_shared<libremidi::observer>(libremidi::observer({
            .input_added = [&](const libremidi::input_port& p){
                std::cout << "input added " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                //Q what happens when multiple devices have the same port name, is that possible even in libremidi?

                if (device.midiIn_->is_port_open())
                    device.midiIn_->close_port();

                device.midiIn_->open_port(p);

                std::cout << "opened " << p.port_name << std::endl;

                std::cout << "mcu @ " << device.mcu_ << std::endl;

                if (device.midiOut_->is_port_open())
                    device.mcu_->start();
            },
            .input_removed = [&](const libremidi::input_port& p){
                std::cout << "input removed " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                if (device.midiIn_->is_port_open())
                    device.midiIn_->close_port();
            },
            .output_added = [&](const libremidi::output_port& p){
                std::cout << "output added " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                //Q what happens when multiple devices have the same port name, is that possible even in libremidi?

                if (device.midiOut_->is_port_open())
                    device.midiOut_->close_port();

                device.midiOut_->open_port(p);

                std::cout << "opened " << p.port_name << std::endl;

                std::cout << "mcu @ " << device.mcu_ << std::endl;

                if (device.midiIn_->is_port_open())
                    device.mcu_->start();
            },
            .output_removed = [&](const libremidi::output_port& p){
                std::cout << "output removed " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                if (device.midiOut_->is_port_open())
                    device.midiOut_->close_port();
            }
    }));
    }

    void Controller::deinitMidiObserver(){
//        if (midiObserver_) {
//            delete midiObserver_;
//            midiObserver_ = nullptr;
//        }
    }

    void Controller::initFromConfig(const char filepath[]){
        std::ifstream i(filepath);

        json j;
        try {
            i >> j;
        } catch (std::exception e){
            std::cout << "JSON parse error: " << e.what() << std::endl;
        }
        data_ = j;

        if (data_.devices.size() == 0)
            throw new std::invalid_argument("at least one device needed in configuration");

        j = data_;

        for(auto & [portName, midiDevice] : data_.devices){
            if (midiDevice.vpots.size() != 8)
                throw new std::invalid_argument("configured vpot array must have 8 elements");
        }


//        std::cout << j << std::endl;

    }


    void Controller::start(){
        if (state_ != State::Stopped)
            return;

        state_ = State::Starting;

        initLsProxy();

        initMidiDevices();

        initMidiObserver();

        state_ = State::Running;
    }

    void Controller::stop(){
        if (state_ != State::Running)
            return;
        state_ = State::Stopping;

        deinitMidiObserver();

        deinitLsProxy();

        state_ = State::Stopped;
    }

    void Controller::runloop(){
        if (state_ != State::Running)
            throw new std::runtime_error("Controller not started");

#if defined(__APPLE__)
        // On macOS, observation can *only* be done in the main thread
        // with an active CFRunLoop.
        CFRunLoopRun();
#else
#error not ready for non-apple hardware
#endif
    }

    void Controller::changeActionLayer(int actionLayer){
        if (state_ != State::Running)
            return;
    }

    void Controller::changeBank(int bank){
        if (state_ != State::Running)
            return;
    }

    void Controller::gotoPage(int page){
        if (state_ != State::Running)
            return;

        if (page < kLsOscPageMin || kLsOscPageMax < page)
            return;

        if (lsProxy_.lsState().page == page)
            return;

        lsProxy_.selectPage(page);
    }

    void Controller::changePage(int pageUpDown) {
        if (state_ != State::Running)
            return;

        if (pageUpDown == 0)
            return;

        int page = lsProxy_.lsState().page + pageUpDown;

        std::cerr << "current page " << lsProxy_.lsState().page << std::endl;
        std::cerr << "target page " << page << std::endl;

        gotoPage(page);
    }

    void Controller::midiDeviceEncoder(MidiDevice &midiDevice, MidiDevice::VPot::EncoderFunction encoderFunction, int relativeValue){


        std::cerr << "relative value = " << relativeValue << std::endl;

        if (relativeValue == 0)
            return;

        using EF = MidiDevice::VPot::EncoderFunction;
        switch(encoderFunction){
            case EF::EFEncoder1:
                lsProxy_.encoderChange(0,relativeValue);
                break;
            case EF::EFEncoder2:
                lsProxy_.encoderChange(1,relativeValue);
                break;
            case EF::EFEncoder3:
                lsProxy_.encoderChange(2,relativeValue);
                break;
            case EF::EFEncoder4:
                lsProxy_.encoderChange(3,relativeValue);
                break;
            case EF::EFBank:
                break;
            case EF::EFPage:
                std::cerr << "page " << relativeValue << std::endl;
                changePage(relativeValue);
                break;
            case EF::EFActionLayer:
                break;
            case EF::EFNone:
                break;
        }
    }

    void Controller::midiDeviceButton(MidiDevice &midiDevice, MidiDevice::ButtonFunction buttonFunction, bool pressed){

    }

} // LsMcuRemote