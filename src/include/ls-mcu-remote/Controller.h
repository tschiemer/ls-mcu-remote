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

#ifndef LS_MCU_REMOTE_CONTROLLER_H
#define LS_MCU_REMOTE_CONTROLLER_H

#include <iostream>
#include <map>

#include <nlohmann/json.hpp>

#include <libremidi/libremidi.hpp>
#include <libremidi/protocols/remote_control.hpp>

#include "LsProxy.h"

namespace LsMcuRemote {

    class Controller {

        using json = nlohmann::json;
        using mcu = libremidi::remote_control_protocol;

        public:


            MIDIClientRef handle;

            enum class State : uint8_t {
                Stopped, Starting, Running, Stopping
            };

            class MidiDevice {

                public:

                enum ButtonFunction {
                    BFNone,
                    BFClear,
                    BFRec,
                    BFSet,
                    BFEdit,
                    BFFind,
                    BFGo,
                    BFStop,
                    BFPause,
                    BFNextCue,
                    BFPreviousCue,
                    BFNextPage,
                    BFPreviousPage,
                    BFNextBank,
                    BFPreviousBank,
                    BFVPotMode
                };

                class Button {
                    public:

                    int midiCc;
                    ButtonFunction function;

                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Button, midiCc, function)
                };

                class VPot {
                    public:

                    enum EncoderFunction {
                        EFNone,
                        EFEncoder1, EFEncoder2, EFEncoder3, EFEncoder4,
                        EFBank,
                        EFPage,
                        EFActionLayer
                    };

                    EncoderFunction encoder;
                    ButtonFunction btn;
                    
                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(VPot, encoder, btn)

                };

                std::string_view portName;
                int deviceType = 15;

                struct Playbacks_st {
                    int offset = 0;
                    bool fixed = false;
                    bool linked = false;
                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(struct Playbacks_st, fixed, offset, linked)

                    int current = 0;
                } playbacks;


                std::vector<Button> buttons;

                std::vector<VPot> vpots;

                NLOHMANN_DEFINE_TYPE_INTRUSIVE(MidiDevice, deviceType, playbacks, buttons, vpots)

                protected:
                    friend Controller;

//                    std::shared_ptr<Controller> controller_;

                    std::shared_ptr<libremidi::midi_in> midiIn_;
                    std::shared_ptr<libremidi::midi_out> midiOut_;

                    std::shared_ptr<libremidi::remote_control_processor> mcu_;

                    inline bool isConnected(){
                        return midiIn_->is_port_open() && midiOut_->is_port_open();
                    }

//                    std::function<void(const libremidi::message& message)>

            };


            class Data {

                public:

                struct Lightshark_st {
                    std::string ip = kLsOscLsDefaultIpStr;
                    uint16_t port = kLsOscLsDefaultIncomingUdpPort;
                    uint16_t remotePort = kLsOscLsDefaultOutgoingUdpPort;
                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(struct Lightshark_st, ip, port, remotePort)
                } lightshark;

                std::map<std::string,MidiDevice> devices;

//                std::map<MidiDevice> devices;

                NLOHMANN_DEFINE_TYPE_INTRUSIVE(Data, lightshark, devices)
            };

        private:

            Controller(){

            };

        public:

            static Controller& getInstance(){
                static Controller instance; // Guaranteed to be destroyed.
                // Instantiated on first use.
                return instance;
            }

            Controller(Controller const&)               = delete;
            void operator=(Controller const&)  = delete;


        private:

            State state_;

            Data data_;

            LsProxy lsProxy_;

            std::shared_ptr<libremidi::observer> midiObserver_;


        protected:

//            void initMidiDevice(MidiDevice &midiDevice);

            void initLsProxy();
            void deinitLsProxy();

            void initMidiDevices();
            void deinitMidiDevices();

            void initMidiObserver();
            void deinitMidiObserver();


        public:

//            Controller();
//            ~Controller();

            void initFromConfig(const char filepath[]);

            inline State state(){ return state_; }

            void start();
            void stop();

            void runloop();


        protected:

            void midiDeviceEncoder(MidiDevice &device, MidiDevice::VPot::EncoderFunction encoderFunction, int relativeValue);
            void midiDeviceButton(MidiDevice &device, MidiDevice::ButtonFunction buttonFunction, bool pressed);


            void changeActionLayer(int actionLayerUpDown);
            void gotoActionLayer(int actionLayer);

            void changeBank(int bankUpDown);
            void gotoBank(int bank);

            void gotoPage(int page);
            void changePage(int pageUpDown);
    };


    NLOHMANN_JSON_SERIALIZE_ENUM(Controller::MidiDevice::VPot::EncoderFunction, {
        {Controller::MidiDevice::VPot::EFNone, nullptr},
        {Controller::MidiDevice::VPot::EFEncoder1, "encoder1"},
        {Controller::MidiDevice::VPot::EFEncoder2, "encoder2"},
        {Controller::MidiDevice::VPot::EFEncoder3, "encoder3"},
        {Controller::MidiDevice::VPot::EFEncoder4, "encoder4"},
        {Controller::MidiDevice::VPot::EFBank, "bank"},
        {Controller::MidiDevice::VPot::EFPage, "page"},
        {Controller::MidiDevice::VPot::EFActionLayer, "actionLayer"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM(Controller::MidiDevice::ButtonFunction , {
        {Controller::MidiDevice::BFNone, nullptr},
        {Controller::MidiDevice::BFClear, "clear"},
        {Controller::MidiDevice::BFRec, "rec"},
        {Controller::MidiDevice::BFSet, "set"},
        {Controller::MidiDevice::BFEdit, "edit"},
        {Controller::MidiDevice::BFFind, "find"},
        {Controller::MidiDevice::BFGo, "go"},
        {Controller::MidiDevice::BFStop, "stop"},
        {Controller::MidiDevice::BFPause, "pause"},
        {Controller::MidiDevice::BFNextCue, "nextCue"},
        {Controller::MidiDevice::BFPreviousCue, "previousCue"},
        {Controller::MidiDevice::BFNextPage, "nextPage"},
        {Controller::MidiDevice::BFPreviousPage, "previousPage"},
        {Controller::MidiDevice::BFNextBank, "nextBank"},
        {Controller::MidiDevice::BFPreviousBank, "previousBank"},
        {Controller::MidiDevice::BFVPotMode, "vPotMode"}
    })

} // LsMcuRemote

#endif //LS_MCU_REMOTE_CONTROLLER_H
