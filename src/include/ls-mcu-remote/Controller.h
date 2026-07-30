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
        using MCU = libremidi::remote_control_protocol;

        public:

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
                    int deviceType = 15;

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
                    BankLayoutSet * sharedBanks_;
                    ButtonLayerSet * sharedButtonLayers_;

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

//                    std::map<MCU::mixer_command, ButtonAction&> buttonActionLUT_;
//
//                    std::map<int,MCU::mixer_command> executorLUT;

                protected:

                    inline BankLayout & currentBank(){
                        return sharedBanks_->at(banks.current_);
                    }

                    inline ButtonLayer & currentButtonLayer(){
                        return sharedButtonLayers_->at(buttonLayer);
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
//                    static const char * ButtonKey2LUT(MCU::mixer_command cmd);

//                    void remapButtonActionLUT();
                    ButtonAction & lookupButtonAction(MCU::mixer_command cmd);

//                    void resetExecutorLUT();



            };


            class Data {

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

        {Controller::MidiDevice::BFChaseSpeedMasterReset, "BFChaseSpeedMasterReset"},
        {Controller::MidiDevice::BFChaseSpeedMasterTap, "BFChaseSpeedMasterTap"},
        {Controller::MidiDevice::BFFxSizeMasterReset, "BFFxSizeMasterReset"},
        {Controller::MidiDevice::BFFxSpeedMasterReset, "BFFxSpeedMasterReset"},
        {Controller::MidiDevice::BFFxSpeedMasterTap, "BFFxSpeedMasterTap"},

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
        {Controller::MidiDevice::PlaybackId::PIGm, "gm"},
    })

} // LsMcuRemote

#endif //LS_MCU_REMOTE_CONTROLLER_H
