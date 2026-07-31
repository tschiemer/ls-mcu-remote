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

#include "nlohmann/json.hpp"

#include <libremidi/libremidi.hpp>
#include <libremidi/protocols/remote_control.hpp>

#include "LsProxy.h"

namespace LsMcuRemote {

    class Controller {

        using json = nlohmann::json;
        using MCU = libremidi::remote_control_protocol;

        public:

            //forward declarations
            class MidiDevice;
            class Data;

            static constexpr int kEncoderPbLevelStepsize = 1;


            MIDIClientRef handle;

            enum class State : uint8_t {
                Stopped, Starting, Running, Stopping
            };

            class MidiDevice {

                public:

                    enum ButtonFunction {
                        BFNone,

                        BFPageUp,
                        BFPageDown,

                        BFDBO,

                        BFEdit,
                        BFUpdate,
                        BFDelete,
                        BFCopy,
                        BFMove,
                        BFSet,
                        BFFan,

                        BFFind,

                        BFClear,
                        BFRec,

                        BFSelectPlayback, // requires target

                        // optional target -> if none main
                        BFGo,
                        BFRelease,
                        BFPause,
                        BFNextCue,
                        BFPreviousCue,
                        BFTap,

                        BFSelectFixture,
                        BFSelectGroup,
                        BFSelectNext,
                        BFSelectPrevious,

                        BFIntensity,
                        BFPosition,
                        BFColor,
                        BFBeam,
                        BFAdvanced,
                        BFGobo,
                        BFFx,

                        BFChaseSpeedMasterReset,
                        BFChaseSpeedMasterTap,
                        BFFxSizeMasterReset,
                        BFFxSpeedMasterReset,
                        BFFxSpeedMasterTap,

                        BFExecutor,
                        BFExecutorRow,

                        BFNextBank,
                        BFPreviousBank,
                        BFSelectBank, // requires target
                    };

                    enum EncoderFunction {
                        EFNone,
                        EFEncoder1,
                        EFEncoder2,
                        EFEncoder3,
                        EFEncoder4,
                        EFGrandmaster,
                        EFSmChase,
                        EFSmFxSize,
                        EFSmFxSpeed,
                        EFSelect,
                        EFBank,
                        EFPage,
                        EFButtonLayer,

                        EF_COUNT = EFButtonLayer
                    };

                    class ButtonAction {
                        public:
                            static constexpr int kNoTarget = -666;

                            ButtonFunction fun;
                            int target = kNoTarget;

                            NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ButtonAction, fun, target)

                        public:

                            static inline int executorTarget(int page, int col, int row){
                                return 100*(page +1) + 10*(col+1) + (row+1);
                            }
                            static inline void executorTarget(int target, int &page, int &col, int &row){
                                page = target / 100 - 1;
                                col = target / 10 % 10 - 1;
                                row = target % 10 - 1;
                            }

                            static inline int executorRowTarget(int page, int row){
                                // let's have column = 0
                                return executorTarget(page, -1, row);
                            }
                            static inline void executorRowTarget(int target, int &page, int &row){
                                page = target / 100 - 1;
                                // same format as single executor but without row
                                row = target % 10 - 1;
                            }

                    };

                    typedef std::map<std::string, ButtonAction> ButtonLayer;
                    typedef std::vector<MidiDevice::ButtonLayer> ButtonLayerSet;

                    enum class PlaybackId : uint8_t {
                        PIPb1,PIPb2,PIPb3,PIPb4,PIPb5,PIPb6,PIPb7,PIPb8,PIPb9,PIPb10,
                        PIPb11,PIPb12,PIPb13,PIPb14,PIPb15,PIPb16,PIPb17,PIPb18,PIPb19,PIPb20,
                        PIPb21,PIPb22,PIPb23,PIPb24,PIPb25,PIPb26,PIPb27,PIPb28,PIPb29,PIPb30,
                        PISmChase, PISmFxSize, PISmFxSpeed, PIGm, PINone
                    };
                    typedef std::vector<MidiDevice::PlaybackId> BankLayout;
                    typedef std::vector<BankLayout> BankLayoutSet;

                public:

    //                std::string_view portName;
                    MCU::device_type deviceType = MCU::device_type::mackie_control;

                    struct Bank_st {
                        int offset = 0;
                        bool fixed = false;
                        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(struct Bank_st, fixed, offset)

                        int current_ = 0;
                        int previous_ = 0;
                    } banks;

                    int buttonLayer = 0;

                    std::vector<EncoderFunction> vpots;

                    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MidiDevice, deviceType, banks, buttonLayer, vpots)

                protected:
                    friend Controller;

                    LsProxy::LsState * lsStateRef_;

                    Controller::Data * sharedData_;

//                    BankLayoutSet * sharedBanks_;
//                    ButtonLayerSet * sharedButtonLayers_;
//                    std::vector<std::map<

                    libremidi::midi_in * midiInRef_;
                    libremidi::midi_out * midiOutRef_;

                    libremidi::remote_control_processor * mcuRef_;

                    /**
                     * 8 channel + master
                     */
                    int faderLevels_[9] = {0,0,0,0,0,0,0,0,0};

                    /**
                     * For each encoder function sets the index of the vpot using this function
                     */
                    int encoderFunctionLUT_[EF_COUNT];

                protected:

                    inline BankLayout & currentBank(){
                        return sharedData_->banks.at(banks.current_);
//                        return sharedBanks_->at(banks.current_);
                    }

                    inline ButtonLayer & currentButtonLayer(){
                        return sharedData_->buttonLayers.at(buttonLayer);
//                        return sharedButtonLayers_->at(buttonLayer);
                    }

                    inline std::map<int,MCU::mixer_command> & currentExecutorLUT(){
                        return sharedData_->executorsPerLayerLUT_.at(buttonLayer);
                    }

                    void init();

                    void start();
                    void stop();

                    void reset();
//                    void resetLcd();

                    void updateVPotLeds();
                    void updateVPotLed(int i);

                    void updateVPotLcd();
                    void updateVPotLcd(int i);

                    void updateTrackColors();
                    void updateTrackColor(int i, int color);

                    void updatePbLcd();
                    void updatePbLcd(int i);

                    void updatePbLevels();
                    void updatePbLevel(PlaybackId pid, int level);

                    void updateExecutorState(int page, int col, int row, bool is_active);

                    void gotoBank(int bank);
                    void changeBank(int upOrDown);

                    inline PlaybackId lookupPlaybackId(int fader) {
                        return currentBank().at(fader);
                    }
                    int lookupFaderIndex(PlaybackId pid);

                    int lookupVPotIndex(PlaybackId pid);
                    inline int lookupVPotIndex(EncoderFunction ef){
                        return encoderFunctionLUT_[ef];
                    }

                    static const char * mixerCommand2ButtonKeyLUT(MCU::mixer_command cmd);
                    static libremidi::remote_control_protocol::mixer_command buttonKey2MixerCommandLUT(std::string key);

                    ButtonAction & lookupButtonAction(MCU::mixer_command cmd);

            };


            class Data {

                friend MidiDevice;

                public:


                    struct Lightshark_st {
                        std::string ip = kLsOscLsDefaultIpStr;
                        uint16_t port = kLsOscLsDefaultIncomingUdpPort;
                        uint16_t remotePort = kLsOscLsDefaultOutgoingUdpPort;
                        NLOHMANN_DEFINE_TYPE_INTRUSIVE(struct Lightshark_st, ip, port, remotePort)
                    } lightshark;

                    MidiDevice::BankLayoutSet banks;

                    MidiDevice::ButtonLayerSet buttonLayers;

                    std::map<std::string,MidiDevice> devices;

                    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Data, lightshark, banks, buttonLayers, devices)


                protected:

                    std::vector<std::map<int, MCU::mixer_command>> executorsPerLayerLUT_;

                    struct TapAssignment {
                        static const MCU::mixer_command NotAssigned = static_cast<MCU::mixer_command>(0xFF);
                        MCU::mixer_command chaseSpeed = NotAssigned;
                        MCU::mixer_command fxSpeed = NotAssigned;
                    };
                    std::vector<TapAssignment> tapPerLayerLUT_;

                    void initButtonLayerLUTs();

                public:

                    void init();


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


//            static inline MCU::fader pid2fader(MidiDevice::PlaybackId pid){
//                return static_cast<MCU::fader>(libremidi::to_underlying(pid) - libremidi::to_underlying(MidiDevice::PlaybackId::PIPb1));
//            }
//
//            static inline MidiDevice::PlaybackId ls2pid(int i){
//                return static_cast<MidiDevice::PlaybackId>(i);
//            }

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

            void midiDeviceEncoder(MidiDevice &device, MidiDevice::EncoderFunction encoderFunction, int relativeValue);
            bool midiDeviceButton(MidiDevice &device, MidiDevice::ButtonAction & action, bool pressed);
            void midiDeviceFader(MidiDevice &device, MidiDevice::PlaybackId, int level);

            void lsPageChange();
            void lsDeviceFader(MidiDevice::PlaybackId, int level);
            void lsExecutor(int page, int col, int row, bool is_active);

            void changeActionLayer(int actionLayerUpDown);
            void gotoActionLayer(int actionLayer);

            void gotoBank(int bank);
            void changeBank(int bankUpDown);

            void gotoPage(int page);
            void changePage(int pageUpDown);
    };


    NLOHMANN_JSON_SERIALIZE_ENUM(Controller::MidiDevice::EncoderFunction, {
        {Controller::MidiDevice::EFNone, nullptr},
        {Controller::MidiDevice::EFEncoder1, "encoder1"},
        {Controller::MidiDevice::EFEncoder2, "encoder2"},
        {Controller::MidiDevice::EFEncoder3, "encoder3"},
        {Controller::MidiDevice::EFEncoder4, "encoder4"},
        {Controller::MidiDevice::EFGrandmaster, "grandmaster"},
        {Controller::MidiDevice::EFSmChase, "chase"},
        {Controller::MidiDevice::EFSmFxSize, "fxSize"},
        {Controller::MidiDevice::EFSmFxSpeed, "fxSpeed"},
        {Controller::MidiDevice::EFSelect, "select"},
        {Controller::MidiDevice::EFBank, "bank"},
        {Controller::MidiDevice::EFPage, "page"},
        {Controller::MidiDevice::EFButtonLayer, "buttonLayer"}
    })


    NLOHMANN_JSON_SERIALIZE_ENUM(Controller::MidiDevice::ButtonFunction , {
        {Controller::MidiDevice::BFNone, nullptr},

        {Controller::MidiDevice::BFPageUp, "pageUp"},
        {Controller::MidiDevice::BFPageDown, "pageDown"},

        {Controller::MidiDevice::BFDBO, "dbo"},

        {Controller::MidiDevice::BFEdit, "edit"},
        {Controller::MidiDevice::BFUpdate, "update"},
        {Controller::MidiDevice::BFDelete, "delete"},
        {Controller::MidiDevice::BFCopy, "copy"},
        {Controller::MidiDevice::BFMove, "move"},
        {Controller::MidiDevice::BFSet, "set"},
        {Controller::MidiDevice::BFFan, "fan"},

        {Controller::MidiDevice::BFClear, "clear"},
        {Controller::MidiDevice::BFRec, "rec"},
        {Controller::MidiDevice::BFFind, "find"},

        {Controller::MidiDevice::BFSelectPlayback, "select"},

        {Controller::MidiDevice::BFGo, "go"},
        {Controller::MidiDevice::BFRelease, "release"},
        {Controller::MidiDevice::BFPause, "pause"},
        {Controller::MidiDevice::BFNextCue, "nextCue"},
        {Controller::MidiDevice::BFPreviousCue, "previousCue"},
        {Controller::MidiDevice::BFTap, "tap"},

        {Controller::MidiDevice::BFSelectFixture, "selectFixture"},
        {Controller::MidiDevice::BFSelectGroup, "selectGroup"},
        {Controller::MidiDevice::BFSelectNext, "selectNext"},
        {Controller::MidiDevice::BFSelectPrevious, "selectPrevious"},

        {Controller::MidiDevice::BFIntensity, "intensity"},
        {Controller::MidiDevice::BFPosition, "position"},
        {Controller::MidiDevice::BFColor, "color"},
        {Controller::MidiDevice::BFBeam, "beam"},
        {Controller::MidiDevice::BFAdvanced, "advanced"},
        {Controller::MidiDevice::BFGobo, "gobo"},
        {Controller::MidiDevice::BFFx, "fx"},

        {Controller::MidiDevice::BFChaseSpeedMasterReset, "chaseReset"},
        {Controller::MidiDevice::BFChaseSpeedMasterTap, "chaseTap"},
        {Controller::MidiDevice::BFFxSizeMasterReset, "fxSizeReset"},
        {Controller::MidiDevice::BFFxSpeedMasterReset, "fxSpeedReset"},
        {Controller::MidiDevice::BFFxSpeedMasterTap, "fxSpeedTap"},

        {Controller::MidiDevice::BFExecutor, "executor"},
        {Controller::MidiDevice::BFExecutorRow, "executorRow"},

        {Controller::MidiDevice::BFNextBank, "nextBank"},
        {Controller::MidiDevice::BFPreviousBank, "previousBank"},
        {Controller::MidiDevice::BFSelectBank, "selectBank"}

    })


    NLOHMANN_JSON_SERIALIZE_ENUM(Controller::MidiDevice::PlaybackId , {
        {Controller::MidiDevice::PlaybackId::PINone, nullptr},
        {Controller::MidiDevice::PlaybackId::PIPb1, "pb1"},
        {Controller::MidiDevice::PlaybackId::PIPb2, "pb2"},
        {Controller::MidiDevice::PlaybackId::PIPb3, "pb3"},
        {Controller::MidiDevice::PlaybackId::PIPb4, "pb4"},
        {Controller::MidiDevice::PlaybackId::PIPb5, "pb5"},
        {Controller::MidiDevice::PlaybackId::PIPb6, "pb6"},
        {Controller::MidiDevice::PlaybackId::PIPb7, "pb7"},
        {Controller::MidiDevice::PlaybackId::PIPb8, "pb8"},
        {Controller::MidiDevice::PlaybackId::PIPb9, "pb9"},
        {Controller::MidiDevice::PlaybackId::PIPb10, "pb10"},
        {Controller::MidiDevice::PlaybackId::PIPb11, "pb11"},
        {Controller::MidiDevice::PlaybackId::PIPb12, "pb12"},
        {Controller::MidiDevice::PlaybackId::PIPb13, "pb13"},
        {Controller::MidiDevice::PlaybackId::PIPb14, "pb14"},
        {Controller::MidiDevice::PlaybackId::PIPb15, "pb15"},
        {Controller::MidiDevice::PlaybackId::PIPb16, "pb16"},
        {Controller::MidiDevice::PlaybackId::PIPb17, "pb17"},
        {Controller::MidiDevice::PlaybackId::PIPb18, "pb18"},
        {Controller::MidiDevice::PlaybackId::PIPb19, "pb19"},
        {Controller::MidiDevice::PlaybackId::PIPb20, "pb20"},
        {Controller::MidiDevice::PlaybackId::PIPb21, "pb21"},
        {Controller::MidiDevice::PlaybackId::PIPb22, "pb22"},
        {Controller::MidiDevice::PlaybackId::PIPb23, "pb23"},
        {Controller::MidiDevice::PlaybackId::PIPb24, "pb24"},
        {Controller::MidiDevice::PlaybackId::PIPb25, "pb25"},
        {Controller::MidiDevice::PlaybackId::PIPb26, "pb26"},
        {Controller::MidiDevice::PlaybackId::PIPb27, "pb27"},
        {Controller::MidiDevice::PlaybackId::PIPb28, "pb28"},
        {Controller::MidiDevice::PlaybackId::PIPb29, "pb29"},
        {Controller::MidiDevice::PlaybackId::PIPb30, "pb30"},
        {Controller::MidiDevice::PlaybackId::PISmChase, "chase"},
        {Controller::MidiDevice::PlaybackId::PISmFxSize, "fxSize"},
        {Controller::MidiDevice::PlaybackId::PISmFxSpeed, "fxSpeed"},
        {Controller::MidiDevice::PlaybackId::PIGm, "gm"}
    })


} // LsMcuRemote

NAMESPACE_LIBREMIDI {

NLOHMANN_JSON_SERIALIZE_ENUM(libremidi::remote_control_protocol::device_type, {

    {libremidi::remote_control_protocol::device_type::logic_control, "logicControl"},
    {libremidi::remote_control_protocol::device_type::logic_control_xt, "logicControlXT"},
    {libremidi::remote_control_protocol::device_type::mackie_control, "mackieControl"},
    {libremidi::remote_control_protocol::device_type::mackie_control_xt, "mackieControlXT"}

})

NLOHMANN_JSON_SERIALIZE_ENUM(libremidi::remote_control_protocol::mixer_command, {

    {libremidi::remote_control_protocol::mixer_command::vpot_click_0, "vpot_click_0"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_1, "vpot_click_1"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_2, "vpot_click_2"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_3, "vpot_click_3"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_4, "vpot_click_4"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_5, "vpot_click_5"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_6, "vpot_click_6"},
    {libremidi::remote_control_protocol::mixer_command::vpot_click_7, "vpot_click_7"},

    {libremidi::remote_control_protocol::mixer_command::rec_0, "rec_0"},
    {libremidi::remote_control_protocol::mixer_command::rec_1, "rec_1"},
    {libremidi::remote_control_protocol::mixer_command::rec_2, "rec_2"},
    {libremidi::remote_control_protocol::mixer_command::rec_3, "rec_3"},
    {libremidi::remote_control_protocol::mixer_command::rec_4, "rec_4"},
    {libremidi::remote_control_protocol::mixer_command::rec_5, "rec_5"},
    {libremidi::remote_control_protocol::mixer_command::rec_6, "rec_6"},
    {libremidi::remote_control_protocol::mixer_command::rec_7, "rec_7"},

    {libremidi::remote_control_protocol::mixer_command::solo_0, "solo_0"},
    {libremidi::remote_control_protocol::mixer_command::solo_1, "solo_1"},
    {libremidi::remote_control_protocol::mixer_command::solo_2, "solo_2"},
    {libremidi::remote_control_protocol::mixer_command::solo_3, "solo_3"},
    {libremidi::remote_control_protocol::mixer_command::solo_4, "solo_4"},
    {libremidi::remote_control_protocol::mixer_command::solo_5, "solo_5"},
    {libremidi::remote_control_protocol::mixer_command::solo_6, "solo_6"},
    {libremidi::remote_control_protocol::mixer_command::solo_7, "solo_7"},

    {libremidi::remote_control_protocol::mixer_command::mute_0, "mute_0"},
    {libremidi::remote_control_protocol::mixer_command::mute_1, "mute_1"},
    {libremidi::remote_control_protocol::mixer_command::mute_2, "mute_2"},
    {libremidi::remote_control_protocol::mixer_command::mute_3, "mute_3"},
    {libremidi::remote_control_protocol::mixer_command::mute_4, "mute_4"},
    {libremidi::remote_control_protocol::mixer_command::mute_5, "mute_5"},
    {libremidi::remote_control_protocol::mixer_command::mute_6, "mute_6"},
    {libremidi::remote_control_protocol::mixer_command::mute_7, "mute_7"},

    {libremidi::remote_control_protocol::mixer_command::sel_0, "sel_0"},
    {libremidi::remote_control_protocol::mixer_command::sel_1, "sel_1"},
    {libremidi::remote_control_protocol::mixer_command::sel_2, "sel_2"},
    {libremidi::remote_control_protocol::mixer_command::sel_3, "sel_3"},
    {libremidi::remote_control_protocol::mixer_command::sel_4, "sel_4"},
    {libremidi::remote_control_protocol::mixer_command::sel_5, "sel_5"},
    {libremidi::remote_control_protocol::mixer_command::sel_6, "sel_6"},
    {libremidi::remote_control_protocol::mixer_command::sel_7, "sel_7"},

    // TODO metering
    {libremidi::remote_control_protocol::mixer_command::assign_track, "assign_track"},
    {libremidi::remote_control_protocol::mixer_command::assign_send, "assign_send"},
    {libremidi::remote_control_protocol::mixer_command::assign_pan, "assign_pan"},
    {libremidi::remote_control_protocol::mixer_command::assign_plugin, "assign_plugin"},
    {libremidi::remote_control_protocol::mixer_command::assign_eq, "assign_eq"},
    {libremidi::remote_control_protocol::mixer_command::assign_instrument, "assign_instrument"},

    {libremidi::remote_control_protocol::mixer_command::bank_left, "bank_left"},
    {libremidi::remote_control_protocol::mixer_command::bank_right, "bank_right"},
    {libremidi::remote_control_protocol::mixer_command::channel_left, "channel_left"},
    {libremidi::remote_control_protocol::mixer_command::channel_right, "channel_right"},
    {libremidi::remote_control_protocol::mixer_command::flip, "flip"},
    {libremidi::remote_control_protocol::mixer_command::global, "global"},

    {libremidi::remote_control_protocol::mixer_command::name_value_button, "name_value_button"},
    {libremidi::remote_control_protocol::mixer_command::smpte_beats_button, "smpte_beats_button"},

    {libremidi::remote_control_protocol::mixer_command::f1, "f1"},
    {libremidi::remote_control_protocol::mixer_command::f2, "f2"},
    {libremidi::remote_control_protocol::mixer_command::f3, "f3"},
    {libremidi::remote_control_protocol::mixer_command::f4, "f4"},
    {libremidi::remote_control_protocol::mixer_command::f5, "f5"},
    {libremidi::remote_control_protocol::mixer_command::f6, "f6"},
    {libremidi::remote_control_protocol::mixer_command::f7, "f7"},
    {libremidi::remote_control_protocol::mixer_command::f8, "f8"},

    {libremidi::remote_control_protocol::mixer_command::midi_tracks, "midi_tracks"},
    {libremidi::remote_control_protocol::mixer_command::inputs, "inputs"},
    {libremidi::remote_control_protocol::mixer_command::audio_tracks, "audio_tracks"},
    {libremidi::remote_control_protocol::mixer_command::audio_instruments, "audio_instruments"},
    {libremidi::remote_control_protocol::mixer_command::aux, "aux"},
    {libremidi::remote_control_protocol::mixer_command::busses, "busses"},
    {libremidi::remote_control_protocol::mixer_command::outputs, "outputs"},
    {libremidi::remote_control_protocol::mixer_command::user, "user"},

    {libremidi::remote_control_protocol::mixer_command::shift, "shift"},
    {libremidi::remote_control_protocol::mixer_command::option, "option"},
    {libremidi::remote_control_protocol::mixer_command::control, "control"},
    {libremidi::remote_control_protocol::mixer_command::alt, "alt"},

    {libremidi::remote_control_protocol::mixer_command::save, "save"},
    {libremidi::remote_control_protocol::mixer_command::undo, "undo"},
    {libremidi::remote_control_protocol::mixer_command::cancel, "cancel"},
    {libremidi::remote_control_protocol::mixer_command::enter, "enter"},

    {libremidi::remote_control_protocol::mixer_command::markers, "markers"},
    {libremidi::remote_control_protocol::mixer_command::nudge, "nudge"},
    {libremidi::remote_control_protocol::mixer_command::cycle, "cycle"},
    {libremidi::remote_control_protocol::mixer_command::drop, "drop"},
    {libremidi::remote_control_protocol::mixer_command::replace, "replace"},
    {libremidi::remote_control_protocol::mixer_command::click, "click"},
    {libremidi::remote_control_protocol::mixer_command::solo, "solo"},

    {libremidi::remote_control_protocol::mixer_command::rewind, "rewind"},
    {libremidi::remote_control_protocol::mixer_command::forward, "forward"},
    {libremidi::remote_control_protocol::mixer_command::stop, "stop"},
    {libremidi::remote_control_protocol::mixer_command::play, "play"},
    {libremidi::remote_control_protocol::mixer_command::record, "record"},

    {libremidi::remote_control_protocol::mixer_command::up, "up"},
    {libremidi::remote_control_protocol::mixer_command::down, "down"},
    {libremidi::remote_control_protocol::mixer_command::left, "left"},
    {libremidi::remote_control_protocol::mixer_command::right, "right"},
    {libremidi::remote_control_protocol::mixer_command::zoom, "zoom"},
    {libremidi::remote_control_protocol::mixer_command::scrub, "scrub"},

    {libremidi::remote_control_protocol::mixer_command::user_switch_1, "user_switch_1"},
    {libremidi::remote_control_protocol::mixer_command::user_switch_2, "user_switch_2"},

    {libremidi::remote_control_protocol::mixer_command::fader_touched_0, "fader_touched_0"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_1, "fader_touched_1"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_2, "fader_touched_2"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_3, "fader_touched_3"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_4, "fader_touched_4"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_5, "fader_touched_5"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_6, "fader_touched_6"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_7, "fader_touched_7"},
    {libremidi::remote_control_protocol::mixer_command::fader_touched_master, "fader_touched_master"},

    {libremidi::remote_control_protocol::mixer_command::smpte_led, "smpte_led"},
    {libremidi::remote_control_protocol::mixer_command::beats_led, "beats_led"},
    {libremidi::remote_control_protocol::mixer_command::rude_solo_led, "rude_solo_led"},

    {libremidi::remote_control_protocol::mixer_command::relay_click, "relay_click"}
})
} // NAMESPACE_LIBREMIDI

#endif //LS_MCU_REMOTE_CONTROLLER_H
