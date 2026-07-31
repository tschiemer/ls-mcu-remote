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
#ifdef which_mixer_control_type


using which_mixer_command_type = libremidi::remote_control_protocol::which_mixer_command_type;
using which_mixer_command_index = libremidi::remote_control_protocol::which_mixer_command_index;

using which_mixer_control_type = libremidi::remote_control_protocol::which_mixer_control_type;
using which_mixer_control_index = libremidi::remote_control_protocol::which_mixer_control_index;

using relative_midi_to_value = libremidi::remote_control_protocol::relative_midi_to_value;

#else

using mixer_command = libremidi::remote_control_protocol::mixer_command;
using mixer_control = libremidi::remote_control_protocol::mixer_control;

#define type_vpot_click vpot_click_0
#define type_rec rec_0
#define type_solo solo_0
#define type_mute mute_0
#define type_sel sel_0
#define type_assign assign_track
#define type_channel bank_left
#define type_f f1
#define type_page midi_tracks
#define type_meta shift
#define type_control save
#define type_transport markers
#define type_user user_switch_1
#define type_fader_touched fader_touched_0
#define type_leds smpte_led
//#define type_other relay_click

static mixer_command which_mixer_command_type(mixer_command cmd)
{
    if (mixer_command::vpot_click_0 <= cmd && cmd <= mixer_command::vpot_click_7) return mixer_command::type_vpot_click;
    if (mixer_command::rec_0 <= cmd && cmd <= mixer_command::rec_7) return mixer_command::type_rec;
    if (mixer_command::solo_0 <= cmd && cmd <= mixer_command::solo_7) return mixer_command::type_solo;
    if (mixer_command::mute_0 <= cmd && cmd <= mixer_command::mute_7) return mixer_command::type_mute;
    if (mixer_command::sel_0 <= cmd && cmd <= mixer_command::sel_7) return mixer_command::type_sel;
    if (mixer_command::assign_track <= cmd && cmd <= mixer_command::assign_instrument) return mixer_command::type_assign;
    if (mixer_command::bank_left <= cmd && cmd <= mixer_command::global) return mixer_command::type_channel;
    if (mixer_command::f1 <= cmd && cmd <= mixer_command::f8) return mixer_command::type_f;
    if (mixer_command::midi_tracks <= cmd && cmd <= mixer_command::user) return mixer_command::type_page;
    if (mixer_command::shift <= cmd && cmd <= mixer_command::alt) return mixer_command::type_meta;
    if (mixer_command::save <= cmd && cmd <= mixer_command::enter) return mixer_command::type_control;
    if (mixer_command::markers <= cmd && cmd <= mixer_command::scrub) return mixer_command::type_transport;
    if (mixer_command::user_switch_1 <= cmd && cmd <= mixer_command::user_switch_2) return mixer_command::type_user;
    if (mixer_command::fader_touched_0 <= cmd && cmd <= mixer_command::fader_touched_master) return mixer_command::type_fader_touched;
    if (mixer_command::smpte_led <= cmd && cmd <= mixer_command::rude_solo_led) return mixer_command::type_leds;

    return mixer_command::relay_click;
}

inline static mixer_command which_mixer_command_type(uint8_t cmd_byte){
    return which_mixer_command_type(static_cast<mixer_command>(cmd_byte));
}

inline static int which_mixer_command_index(mixer_command type, mixer_command cmd){
    return (uint8_t)cmd - (uint8_t)type;
}
inline static int which_mixer_command_index(mixer_command type, uint8_t cmd_byte){
    return which_mixer_command_index(type, static_cast<mixer_command>(cmd_byte));
}

#define type_vpot_rotation vpot_rotation_0
#define type_vpot_led vpot_led_0
#define type_timecode_digit timecode_digit_0
#define type_assignment_digit assignment_digit_0
//#define type_other external_control

static mixer_control which_mixer_control_type(mixer_control ctl)
{
    if (mixer_control::vpot_rotation_0 <= ctl && ctl <= mixer_control::vpot_rotation_7) return mixer_control::type_vpot_rotation;
    if (mixer_control::vpot_led_0 <= ctl && ctl <= mixer_control::vpot_led_7) return mixer_control::type_vpot_led;
    if (mixer_control::timecode_digit_0 <= ctl && ctl <= mixer_control::timecode_digit_9) return mixer_control::type_timecode_digit;
    if (mixer_control::assignment_digit_0 <= ctl && ctl <= mixer_control::assignment_digit_1) return mixer_control::type_assignment_digit;

    return mixer_control::external_control;
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

    void Controller::MidiDevice::init(){

        for(int i = 0; i < EF_COUNT; i++)
            encoderFunctionLUT_[i] = -1;

        for(int i = 0; i < vpots.size(); i++)
            encoderFunctionLUT_[vpots[i]] = i;

    }

    void Controller::MidiDevice::start(){

        // only start when in/out ports are open
        if (!midiInRef_->is_port_open() || !midiOutRef_->is_port_open())
            return;

        mcuRef_->start();

        clearAllButtons();

    }

    void Controller::MidiDevice::stop(){
        // go offline?
//        mcuRef_->
    }

    void Controller::MidiDevice::reset(){

        mcuRef_->reset();



        // reset LCD
//        char lcd[113];
//
//        std::memset(lcd, sizeof(lcd), ' ');
//
//        lcd[sizeof(lcd)-1];
//
//        mcuRef_->update_lcd(lcd, 0);
//
//        mcuRef_->update_lcd(lcd, 0);


        updateTrackColors();


        updateVPotLcd();

        updatePbLcd();
        updatePbLevels();

        updateVPotLeds();

        setActiveButtons();

    }


    void Controller::MidiDevice::updateVPotLed(int i){

        static char cmdLUT[8][13] = {
                "vpot_click_0",
                "vpot_click_1",
                "vpot_click_2",
                "vpot_click_3",
                "vpot_click_4",
                "vpot_click_5",
                "vpot_click_6",
                "vpot_click_7"
        };
//        static MCU::mixer_command cmdLUT[8] = {
//                MCU::mixer_command::vpot_click_0,
//                MCU::mixer_command::vpot_click_1,
//                MCU::mixer_command::vpot_click_2,
//                MCU::mixer_command::vpot_click_3,
//                MCU::mixer_command::vpot_click_4,
//                MCU::mixer_command::vpot_click_5,
//                MCU::mixer_command::vpot_click_6,
//                MCU::mixer_command::vpot_click_7
//        };
        static MCU::mixer_control mcLUT[8] = {
                MCU::mixer_control::vpot_led_0,
                MCU::mixer_control::vpot_led_1,
                MCU::mixer_control::vpot_led_2,
                MCU::mixer_control::vpot_led_3,
                MCU::mixer_control::vpot_led_4,
                MCU::mixer_control::vpot_led_5,
                MCU::mixer_control::vpot_led_6,
                MCU::mixer_control::vpot_led_7,
        };

        ButtonAction action;
        EncoderFunction encoderFunction = vpots[i];

        if (currentButtonLayer().contains(cmdLUT[i]))
            action = currentButtonLayer().at(cmdLUT[i]);


        int ledMode = 0;

        switch(encoderFunction){
            case EFGrandmaster:
            {
                int value = lsStateRef_->grandmasterLevel * 11 / 255;
                ledMode = (int)MCU::led_ring_mode::mode_2 | value; // fill from left to right
            }
                break;

            case EFSmChase:
            {
                int value = lsStateRef_->chaseSpeedMasterLevel * 11 / 255;
                ledMode = (int)MCU::led_ring_mode::mode_2 | value; // fill from left to right
            }
                break;

            case EFSmFxSize:
            {
                int value = lsStateRef_->fxSizeMasterLevel * 11 / 255;
                ledMode = (int)MCU::led_ring_mode::mode_2 | value; // fill from left to right
            }
                break;

            case EFSmFxSpeed:
            {
                int value = lsStateRef_->fxSpeedMasterLevel * 11 / 255;
                ledMode = (int)MCU::led_ring_mode::mode_2 | value; // fill from left to right
            }
                break;

            case EFEncoder1:
            case EFEncoder2:
            case EFEncoder3:
            case EFEncoder4:
                ledMode = (int)MCU::led_ring_mode::mode_0 | 6; // single center led
                break;

            case EFBank:
            case EFPage:
            case EFSelect:

//                    ledMode = (int)MCU::led_ring_mode::mode_3 | 2; // three center leds
                break;

            default:
                // nothing
                break;
        }

//        switch(action.fun){
//            case ButtonFunction::BFNone:
//                // do not light up at all
//                break;
//
//            case ButtonFunction::BFClear:
//            case ButtonFunction::BFDBO:
//                // mark as dangerous
////                    ledMode |= (int)MCU::led_ring_mode::mode_3 | 6;
//                ledMode |= 0b01000000;
//                break;
//
//            default:
//
//        }


//            mixer_control mc = (mixer_control)((int)MCU::mixer_control::vpot_led_0 + i);
        mcuRef_->control(mcLUT[i], ledMode);
    }

    void Controller::MidiDevice::updateVPotLeds(){

        for (int i = 0; i < vpots.size(); i++)
            updateVPotLed(i);
    }

    void Controller::MidiDevice::updateTrackColors(){
        //TODO
    }
    void Controller::MidiDevice::updateTrackColor(int i, int color){
        //TODO
    }

    void Controller::MidiDevice::updateVPotLcd(){

        for(int i = 0; i < vpots.size(); i++)
            updateVPotLcd(i);

    }

    void Controller::MidiDevice::updateVPotLcd(int i){

        char lcd[8] = "       "; // 7 space + EOL

        int l = 0;
        switch(vpots[i]){
            case EFEncoder1:
                l = std::snprintf(lcd, sizeof(lcd), "Enc 1  ");
                break;
            case EFEncoder2:
                l = std::snprintf(lcd, sizeof(lcd), "Enc 2  ");
                break;
            case EFEncoder3:
                l = std::snprintf(lcd, sizeof(lcd), "Enc 3  ");
                break;
            case EFEncoder4:
                l = std::snprintf(lcd, sizeof(lcd), "Enc 4  ");
                break;
            case EFGrandmaster:
                l = std::snprintf(lcd, sizeof(lcd), "GM     ");
                break;
            case EFSmChase:
                l = std::snprintf(lcd, sizeof(lcd), "Chase  ");
                break;
            case EFSmFxSize:
                l = std::snprintf(lcd, sizeof(lcd), "FxSize ");
                break;
            case EFSmFxSpeed:
                l = std::snprintf(lcd, sizeof(lcd), "FxSpeed");
                break;
            case EFSelect:
                l = std::snprintf(lcd, sizeof(lcd), "Select ");
                break;
            case EFButtonLayer:
                l = std::snprintf(lcd, sizeof(lcd), "Layer %i", buttonLayer + 1);
                break;
            case EFBank:
                l = std::snprintf(lcd, sizeof(lcd), "Bank %i ", banks.current_ + 1);
                break;
            case EFPage:
                l = std::snprintf(lcd, sizeof(lcd), "Page %i ", lsStateRef_->page);
                break;
            case EFNone:
                break;
        }

//        std::cerr << "VPot " << i << " LCD: " << lcd << std::endl;

        mcuRef_->update_lcd(lcd, i*7);
    }


    void Controller::MidiDevice::updatePbLcd(){

        for(int i = 0; i < 8; i++)
            updatePbLcd(i);

    }

    void Controller::MidiDevice::updatePbLcd(int i){

        char lcd[8]= {0,0,0,0,0,0,0,0};

        BankLayout & bank = currentBank();

        int l=0;

        switch(bank[i]){
            case PlaybackId::PINone:
                std::memcpy(lcd, "       ", sizeof(lcd));
                l = 7;
                break;
            case PlaybackId::PIGm:
                l = std::snprintf(lcd, sizeof(lcd), "GM");
                break;
            case PlaybackId::PISmChase:
                l = std::snprintf(lcd, sizeof(lcd), "Chase");
                break;
            case PlaybackId::PISmFxSize:
                l = std::snprintf(lcd, sizeof(lcd), "FxSize");
                break;
            case PlaybackId::PISmFxSpeed:
                l = std::snprintf(lcd, sizeof(lcd), "FxSpeed");
                break;
            default:
            {

                int pi = (int)bank[i];

                // Id -> pretty index
                int index = pi + 1;

                char activeOnPage[5] = "";

                if (lsStateRef_->playbacks[pi].is_active)
                    std::memcpy(activeOnPage, "*", 2);

                // well, LS does not provide this info, so this is rather guessed...
//                        std::snprintf(activeOnPage, sizeof(activeOnPage), "%2d", lsStateRef_->playbacks[pi].page+1);

                l = std::snprintf(lcd, sizeof(lcd), "Pb%2i %2s", index, activeOnPage);
            }

        }

//        std::cerr << "Pb " << i << " LCD: " << lcd << std::endl;

        mcuRef_->update_lcd(lcd, 56 + i*7);
    }


    void Controller::MidiDevice::updatePbLevels(){

        BankLayout & bank = currentBank();

        for(int i = 0; i < bank.size(); i++){

            PlaybackId pid = bank[i];


            int level = 0;

            if (pid == PlaybackId::PIGm) {
                level = lsStateRef_->grandmasterLevel;
            } else if (pid == PlaybackId::PISmChase) {
                level = lsStateRef_->chaseSpeedMasterLevel;
            } else if (pid == PlaybackId::PISmFxSize) {
                level = lsStateRef_->fxSizeMasterLevel;
            } else if (pid == PlaybackId::PISmFxSpeed) {
                level = lsStateRef_->fxSpeedMasterLevel;
            }
            else if (pid != PlaybackId::PINone){
                level = lsStateRef_->playbacks[static_cast<int>(pid)].level;
            }

//            std::cerr << "fader[" << i << "] " << (int)pid << " = " << level << std::endl;

            // 8-bit -> 14-bit
            level <<= 6;

            mcuRef_->fader(static_cast<libremidi::remote_control_protocol::fader>(i), level);
        }

        // possibly there is a level encoder
        updateVPotLeds();
    }

    void Controller::MidiDevice::updatePbLevel(PlaybackId pid, int level){



        int i = lookupFaderIndex(pid);

        if (i > -1){

            //8-bit -> 14-bit
            int faderLevel = level << 6;

            mcuRef_->fader((MCU::fader)i, faderLevel);

            // possibly is now active
            updatePbLcd(i);
        }


        i = lookupVPotIndex(pid);

        if (i > -1){
            updateVPotLed(i);
        }

//        std::cerr << "pid " << (int)pid << std::endl;

    }
    void Controller::MidiDevice::updateExecutorState(int page, int col, int row, bool is_active){

        int target = ButtonAction::executorTarget(page, col, row);
        auto lut = currentExecutorLUT();

//        if (is_active){
//            std::cerr << "executor " << target << std::endl;
//        }

        if (!lut.contains(target))
            return;

        MCU::mixer_command cmd = lut[target];

        mcuRef_->command(cmd, is_active);
    }

    void Controller::MidiDevice::gotoBank(int bank){
        // ignore command if not supposed to change
        if (banks.fixed)
            return;

        // range sanity check
        if (bank < 0 | sharedData_->banks.size() <= bank)
            return;

        // if going to current bank, go to last instead
        // this allows for popping to a specific bank and going back again ;)
        if (banks.current_ == bank){
            banks.current_ = banks.previous_;
            banks.previous_ = bank;
        } else {
            banks.previous_ = banks.current_;
            banks.current_ = bank;
        }


        int i = lookupVPotIndex(EFBank);

        if (i > -1) {
            updateVPotLed(i);
            updateVPotLcd(i);
        }

        updateTrackColors();
        updatePbLcd();
        updatePbLevels();
    }

    void Controller::MidiDevice::changeBank(int upOrDown){
        // ignore command if not supposed to change
//        if (banks.fixed)
//            return;

        int target = banks.current_ + upOrDown;

        gotoBank(target);
    }

    void Controller::MidiDevice::clearAllButtons(){

        static MCU::mixer_command buttons[] = {

                MCU::mixer_command::vpot_click_0,
                MCU::mixer_command::vpot_click_1,
                MCU::mixer_command::vpot_click_2,
                MCU::mixer_command::vpot_click_3,
                MCU::mixer_command::vpot_click_4,
                MCU::mixer_command::vpot_click_5,
                MCU::mixer_command::vpot_click_6,
                MCU::mixer_command::vpot_click_7,

                MCU::mixer_command::rec_0,
                MCU::mixer_command::rec_1,
                MCU::mixer_command::rec_2,
                MCU::mixer_command::rec_3,
                MCU::mixer_command::rec_4,
                MCU::mixer_command::rec_5,
                MCU::mixer_command::rec_6,
                MCU::mixer_command::rec_7,

                MCU::mixer_command::solo_0,
                MCU::mixer_command::solo_1,
                MCU::mixer_command::solo_2,
                MCU::mixer_command::solo_3,
                MCU::mixer_command::solo_4,
                MCU::mixer_command::solo_5,
                MCU::mixer_command::solo_6,
                MCU::mixer_command::solo_7,

                MCU::mixer_command::mute_0,
                MCU::mixer_command::mute_1,
                MCU::mixer_command::mute_2,
                MCU::mixer_command::mute_3,
                MCU::mixer_command::mute_4,
                MCU::mixer_command::mute_5,
                MCU::mixer_command::mute_6,
                MCU::mixer_command::mute_7,

                MCU::mixer_command::sel_0,
                MCU::mixer_command::sel_1,
                MCU::mixer_command::sel_2,
                MCU::mixer_command::sel_3,
                MCU::mixer_command::sel_4,
                MCU::mixer_command::sel_5,
                MCU::mixer_command::sel_6,
                MCU::mixer_command::sel_7,

                MCU::mixer_command::assign_track,
                MCU::mixer_command::assign_send,
                MCU::mixer_command::assign_pan,
                MCU::mixer_command::assign_plugin,
                MCU::mixer_command::assign_eq,
                MCU::mixer_command::assign_instrument,

                MCU::mixer_command::bank_left,
                MCU::mixer_command::bank_right,
                MCU::mixer_command::channel_left,
                MCU::mixer_command::channel_right,
                MCU::mixer_command::flip,
                MCU::mixer_command::global,

                MCU::mixer_command::name_value_button,
                MCU::mixer_command::smpte_beats_button,

                MCU::mixer_command::f1,
                MCU::mixer_command::f2,
                MCU::mixer_command::f3,
                MCU::mixer_command::f4,
                MCU::mixer_command::f5,
                MCU::mixer_command::f6,
                MCU::mixer_command::f7,
                MCU::mixer_command::f8,

                MCU::mixer_command::midi_tracks,
                MCU::mixer_command::inputs,
                MCU::mixer_command::audio_tracks,
                MCU::mixer_command::audio_instruments,
                MCU::mixer_command::aux,
                MCU::mixer_command::busses,
                MCU::mixer_command::outputs,
                MCU::mixer_command::user,

                MCU::mixer_command::shift,
                MCU::mixer_command::option,
                MCU::mixer_command::control,
                MCU::mixer_command::alt,

                MCU::mixer_command::save,
                MCU::mixer_command::undo,
                MCU::mixer_command::cancel,
                MCU::mixer_command::enter,

                MCU::mixer_command::markers,
                MCU::mixer_command::nudge,
                MCU::mixer_command::cycle,
                MCU::mixer_command::drop,
                MCU::mixer_command::replace,
                MCU::mixer_command::click,
                MCU::mixer_command::solo,

                MCU::mixer_command::rewind,
                MCU::mixer_command::forward,
                MCU::mixer_command::stop,
                MCU::mixer_command::play,
                MCU::mixer_command::record,

                MCU::mixer_command::up,
                MCU::mixer_command::down,
                MCU::mixer_command::left,
                MCU::mixer_command::right,
                MCU::mixer_command::zoom,
                MCU::mixer_command::scrub,

                MCU::mixer_command::user_switch_1,
                MCU::mixer_command::user_switch_2,

                MCU::mixer_command::fader_touched_0,
                MCU::mixer_command::fader_touched_1,
                MCU::mixer_command::fader_touched_2,
                MCU::mixer_command::fader_touched_3,
                MCU::mixer_command::fader_touched_4,
                MCU::mixer_command::fader_touched_5,
                MCU::mixer_command::fader_touched_6,
                MCU::mixer_command::fader_touched_7,
                MCU::mixer_command::fader_touched_master,

                MCU::mixer_command::smpte_led,
                MCU::mixer_command::beats_led,
                MCU::mixer_command::rude_solo_led,

                MCU::mixer_command::relay_click
        };

        for(MCU::mixer_command cmd : buttons)
            mcuRef_->command(cmd, false);
    }

    void Controller::MidiDevice::clearUsedButtons(){
        ButtonLayer layer = currentButtonLayer();

        for(auto & [key,action] : layer){
            MCU::mixer_command cmd = buttonKey2MixerCommandLUT(key);
            mcuRef_->command(cmd, false);
        }

    }

    void Controller::MidiDevice::setActiveButtons(){
        auto lut = currentExecutorLUT();
        for (auto [target, cmd] : lut){
            int page, col, row;
            ButtonAction::executorTarget(target, page, col, row);
            bool is_active = lsStateRef_->executors[page][col][row];
            mcuRef_->command(cmd, is_active);
        }
    }

    void Controller::MidiDevice::gotoButtonLayer(int layer){

        // sanity check
        if (layer < 0 || sharedData_->buttonLayers.size() <= layer)
            return;

        clearUsedButtons();

        buttonLayer = layer;

        updateVPotLcd();
        setActiveButtons();
    }

    void Controller::MidiDevice::changeButtonLayer(int upOrDown){
        if (upOrDown  == 0)
            return;

        int target = buttonLayer + upOrDown;

        gotoButtonLayer(target);
    }


    int Controller::MidiDevice::lookupFaderIndex(PlaybackId pid){

        BankLayout & bank = currentBank();

        for(int i = 0; i < bank.size(); i++){
            if (bank[i] == pid)
                return i;
        }

        return -1;
    }
    int Controller::MidiDevice::lookupVPotIndex(PlaybackId pid){

        switch(pid){
            case PlaybackId::PIGm:
                return lookupVPotIndex(EFGrandmaster);

            case PlaybackId::PISmChase:
                return lookupVPotIndex(EFSmChase);

            case PlaybackId::PISmFxSize:
                return lookupVPotIndex(EFSmFxSize);

            case PlaybackId::PISmFxSpeed:
                return lookupVPotIndex(EFSmFxSpeed);

            default:
                return -1;
        }
    }

//    void Controller::MidiDevice::remapButtonActionLUT(){
//
//        buttonActionLUT_.clear();
//
//        ButtonLayer layer = currentButtonLayer();
//
//        for( auto & [mixerCommand, buttonAction] : layer){
//
//        }
//    }
//
//    void Controller::MidiDevice::resteExecutorLUT(){
//        executorLUT.clear();
//
//        ButtonLayer layer = currentButtonLayer();
//
//        for( ButtonAction &action : layer){
//
//        }
//    }


    const char * Controller::MidiDevice::mixerCommand2ButtonKeyLUT(MCU::mixer_command cmd){

        static bool to_be_initialized = true;
        static const char * LUT[127];

        if (to_be_initialized){
            to_be_initialized = false;

            // default fill with nullptr
            std::memset(LUT, 0, sizeof(LUT));


            // let's make life a bit easier with these enum key -> string lookups

#define SET_LUT(key) LUT[libremidi::to_underlying(MCU::mixer_command::key)] = #key;
#define SET_LUT_0_7(key) \
    SET_LUT(key ## _0)       \
    SET_LUT(key ## _1)       \
    SET_LUT(key ## _2)       \
    SET_LUT(key ## _3)       \
    SET_LUT(key ## _4)       \
    SET_LUT(key ## _5)       \
    SET_LUT(key ## _6)       \
    SET_LUT(key ## _7)


                SET_LUT_0_7(sel)
                SET_LUT_0_7(mute)
                SET_LUT_0_7(rec)
                SET_LUT_0_7(solo)
                SET_LUT_0_7(vpot_click)

                SET_LUT(assign_track)
                SET_LUT(assign_send)
                SET_LUT(assign_pan)
                SET_LUT(assign_plugin)
                SET_LUT(assign_eq)
                SET_LUT(assign_instrument)

                SET_LUT(bank_left)
                SET_LUT(bank_right)
                SET_LUT(channel_left)
                SET_LUT(channel_right)
                SET_LUT(flip)
                SET_LUT(global)

                SET_LUT(name_value_button)
                SET_LUT(smpte_beats_button)

                SET_LUT(f1)
                SET_LUT(f2)
                SET_LUT(f3)
                SET_LUT(f4)
                SET_LUT(f5)
                SET_LUT(f6)
                SET_LUT(f7)
                SET_LUT(f8)

                SET_LUT(midi_tracks)
                SET_LUT(inputs)
                SET_LUT(audio_tracks)
                SET_LUT(audio_instruments)
                SET_LUT(aux)
                SET_LUT(busses)
                SET_LUT(outputs)
                SET_LUT(user)

                SET_LUT(shift)
                SET_LUT(option)
                SET_LUT(control)
                SET_LUT(alt)

                SET_LUT(save)
                SET_LUT(undo)
                SET_LUT(cancel)
                SET_LUT(enter)

                SET_LUT(markers)
                SET_LUT(nudge)
                SET_LUT(cycle)
                SET_LUT(drop)
                SET_LUT(replace)
                SET_LUT(click)
                SET_LUT(solo)

                SET_LUT(rewind)
                SET_LUT(forward)
                SET_LUT(stop)
                SET_LUT(play)
                SET_LUT(record)

                SET_LUT(up)
                SET_LUT(down)
                SET_LUT(left)
                SET_LUT(right)
                SET_LUT(zoom)
                SET_LUT(scrub)

                SET_LUT(user_switch_1)
                SET_LUT(user_switch_2)


#undef SET_LUT_0_7
#undef SET_LUT
        }

        return LUT[libremidi::to_underlying(cmd)];
    }

    libremidi::remote_control_protocol::mixer_command Controller::MidiDevice::buttonKey2MixerCommandLUT(std::string key){

        static bool to_be_initialized = true;
        static std::map<std::string, MCU::mixer_command> LUT;

        if (to_be_initialized){
            to_be_initialized = false;

            // let's make life a bit easier ..

#define SET_LUT(key) LUT[#key] = MCU::mixer_command::key;
#define SET_LUT_0_7(key) \
    SET_LUT(key ## _0)       \
    SET_LUT(key ## _1)       \
    SET_LUT(key ## _2)       \
    SET_LUT(key ## _3)       \
    SET_LUT(key ## _4)       \
    SET_LUT(key ## _5)       \
    SET_LUT(key ## _6)       \
    SET_LUT(key ## _7)


            SET_LUT_0_7(sel)
            SET_LUT_0_7(mute)
            SET_LUT_0_7(rec)
            SET_LUT_0_7(solo)
            SET_LUT_0_7(vpot_click)

            SET_LUT(assign_track)
            SET_LUT(assign_send)
            SET_LUT(assign_pan)
            SET_LUT(assign_plugin)
            SET_LUT(assign_eq)
            SET_LUT(assign_instrument)

            SET_LUT(bank_left)
            SET_LUT(bank_right)
            SET_LUT(channel_left)
            SET_LUT(channel_right)
            SET_LUT(flip)
            SET_LUT(global)

            SET_LUT(name_value_button)
            SET_LUT(smpte_beats_button)

            SET_LUT(f1)
            SET_LUT(f2)
            SET_LUT(f3)
            SET_LUT(f4)
            SET_LUT(f5)
            SET_LUT(f6)
            SET_LUT(f7)
            SET_LUT(f8)

            SET_LUT(midi_tracks)
            SET_LUT(inputs)
            SET_LUT(audio_tracks)
            SET_LUT(audio_instruments)
            SET_LUT(aux)
            SET_LUT(busses)
            SET_LUT(outputs)
            SET_LUT(user)

            SET_LUT(shift)
            SET_LUT(option)
            SET_LUT(control)
            SET_LUT(alt)

            SET_LUT(save)
            SET_LUT(undo)
            SET_LUT(cancel)
            SET_LUT(enter)

            SET_LUT(markers)
            SET_LUT(nudge)
            SET_LUT(cycle)
            SET_LUT(drop)
            SET_LUT(replace)
            SET_LUT(click)
            SET_LUT(solo)

            SET_LUT(rewind)
            SET_LUT(forward)
            SET_LUT(stop)
            SET_LUT(play)
            SET_LUT(record)

            SET_LUT(up)
            SET_LUT(down)
            SET_LUT(left)
            SET_LUT(right)
            SET_LUT(zoom)
            SET_LUT(scrub)

            SET_LUT(user_switch_1)
            SET_LUT(user_switch_2)


#undef SET_LUT_0_7
#undef SET_LUT
        }

        return LUT[key];
    }

    Controller::MidiDevice::ButtonAction & Controller::MidiDevice::lookupButtonAction(MCU::mixer_command cmd){

        static ButtonAction NoAction{.fun = BFNone};

        const char * lookup = mixerCommand2ButtonKeyLUT(cmd);

        if (currentButtonLayer().contains(lookup))
            return currentButtonLayer().at(lookup);

        return NoAction;
    }


    void Controller::Data::init(){
        initButtonLayerLUTs();
    }

    void Controller::Data::initButtonLayerLUTs(){
        executorsPerLayerLUT_.clear();
        tapPerLayerLUT_.clear();

        for (MidiDevice::ButtonLayer & layer : buttonLayers){

            std::map<int, MCU::mixer_command> executorLUT;
            TapAssignment tapLUT;

            for(auto & [cmdKey, buttonAction] : layer){
                switch(buttonAction.fun){
                    case MidiDevice::BFExecutor:
                        executorLUT[buttonAction.target] = MidiDevice::buttonKey2MixerCommandLUT(cmdKey);
                        break;
                    case MidiDevice::BFChaseSpeedMasterTap:
                        tapLUT.chaseSpeed = MidiDevice::buttonKey2MixerCommandLUT(cmdKey);
                        break;
                    case MidiDevice::BFFxSpeedMasterTap:
                        tapLUT.fxSpeed = MidiDevice::buttonKey2MixerCommandLUT(cmdKey);
                        break;
                    default:
                        break;
                }
            }

            executorsPerLayerLUT_.push_back(executorLUT);
        }
    }

    void Controller::initLsProxy(){

        lsProxy_.configure({
           .lightsharkHost = data_.lightshark.host,
           .lightsharkPort = data_.lightshark.port,
           .localPort = data_.lightshark.remotePort,
            .syncInterval = std::chrono::milliseconds(data_.lightshark.syncIntervalMs),
           .onGrandmasterSync = [&](PlaybackLevel_t level){
               if (state_ != State::Running)
                   return;

               lsDeviceFader(MidiDevice::PlaybackId::PIGm, level);
           },
           .onSubmasterSync = [&](Submasters sm, PlaybackLevel_t level){

               if (state_ != State::Running)
                   return;


               MidiDevice::PlaybackId pid = MidiDevice::PlaybackId::PINone;

               switch(sm){
                   case Submasters::ChaseSpeedMaster:
                       pid = MidiDevice::PlaybackId::PISmChase;
                       break;
                   case Submasters::FxSpeedMaster:
                       pid = MidiDevice::PlaybackId::PISmFxSpeed;
                       break;
                   case Submasters::FxSizeMaster:
                       pid = MidiDevice::PlaybackId::PISmFxSize;
                       break;
               }

               lsDeviceFader(pid, level);
           },
           .onPageSync = [&](int page){

               if (state_ != State::Running)
                   return;

                lsPageChange();
           },
           .onPlaybackSync = [&](int pb, PlaybackLevel_t level, bool is_active){

               if (state_ != State::Running)
                   return;

               MidiDevice::PlaybackId pid = (MidiDevice::PlaybackId)(pb);

//               std::cout << "pb sync " << pb << " " << (int)level << " " << is_active << std::endl;

               lsDeviceFader(pid, level);

           },
           .onExecutorSync = [&](int page, int col, int row, bool is_active){

               if (state_ != State::Running)
                   return;

               lsExecutor(page, col, row, is_active);
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

        for(auto & [portName, midiDevice] : data_.devices){

            midiDevice.init();

            // set needed references for devices
            midiDevice.lsStateRef_ = &lsProxy_.lsState();
            midiDevice.sharedData_ = &data_;

            midiDevice.banks.current_ = midiDevice.banks.offset;


            // midi in port
            midiDevice.midiInRef_ = new libremidi::midi_in{{
                 .on_message = [&](const libremidi::message& message) {

//                     std::cerr << "in " << message.size() << std::endl;

                         if (state_ == State::Running) {
                             midiDevice.mcuRef_->on_midi(message);
                         }
                     }
             }};

            // midi out port
            midiDevice.midiOutRef_ = new libremidi::midi_out();

            // midi MCU processor
            midiDevice.mcuRef_ = new libremidi::remote_control_processor{
                    {
//                device_type = midiDevice.deviceType;
                            .midi_out = [&](libremidi::message &&msg) {
                                if (state_ == State::Running && midiDevice.midiOutRef_->is_port_open())
                                    midiDevice.midiOutRef_->send_message(msg);
                            },
                            .on_command = [&](libremidi::remote_control_protocol::mixer_command cmd, bool pressed) {

//                                std::cerr << "command: " <<(int)cmd << " -> " << (pressed ? "pressed" : "released") << "\n";

                                if (state_ != State::Running)
                                    return;


                                auto type = which_mixer_command_type(cmd);
                                auto index = which_mixer_command_index(type, cmd);

                                switch (type){
                                    case mixer_command::type_fader_touched:
                                    {
                                        // only interested in fader release
                                        if (pressed)
                                            return;

                                        MidiDevice::PlaybackId pid = midiDevice.lookupPlaybackId(index);

                                        // ignore if fader not assigned
                                        if (pid == MidiDevice::PlaybackId::PINone)
                                            return;

                                        MCU::fader fader = (MCU::fader)((int)libremidi::remote_control_protocol::fader::fader_0 + index);

                                        midiDevice.mcuRef_->fader(fader, midiDevice.faderLevels_[index]);
                                    }
                                        break;

                                    default:
                                    {
                                        MidiDevice::ButtonAction btn = midiDevice.lookupButtonAction(cmd);

                                        bool feedback = midiDeviceButton(midiDevice, btn, pressed);

                                        if (feedback)
                                            midiDevice.mcuRef_->command(cmd, pressed);
                                    }
                                        break;
                                }
                            },
                            .on_control = [&](libremidi::remote_control_protocol::mixer_control ctl, int v) {
                                if (state_ != State::Running)
                                    return;

//                                std::cerr << "control: " << (int)ctl << " -> " << v << "\n";

                                auto type = which_mixer_control_type(ctl);
                                auto index = which_mixer_control_index(type, ctl);

                                switch(type){
                                    case mixer_control::type_vpot_rotation:
                                        {
                                            int rvalue = relative_midi_to_value(v);
                                            midiDeviceEncoder(midiDevice, midiDevice.vpots[index], rvalue);
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

//                                std::cerr << "fader: " << (int) fader << " -> " << level << "\n";

                                MidiDevice::PlaybackId pid = midiDevice.lookupPlaybackId((int)fader);

                                // ignore if fader not used
                                if (pid == MidiDevice::PlaybackId::PINone)
                                    return;

                                midiDevice.faderLevels_[(int)fader] = level;

                                midiDeviceFader(midiDevice, pid, level);
                            }
                    }
            };
        }

    }

    void Controller::deinitMidiDevices(){

        for(auto &[portName,midiDevice] : data_.devices){

            delete midiDevice.mcuRef_;
            delete midiDevice.midiOutRef_;
            delete midiDevice.midiInRef_;
            delete midiDevice.lsStateRef_;
        }
    }

    void Controller::initMidiObserver(){

        midiObserver_ = std::make_shared<libremidi::observer>(libremidi::observer({
            .input_added = [&](const libremidi::input_port& p){
                std::cerr << "MIDI input found " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                //Q what happens when multiple devices have the same port name, is that possible even in libremidi?

                if (device.midiInRef_->is_port_open())
                    device.midiInRef_->close_port();

                device.midiInRef_->open_port(p);

                device.start();
            },
            .input_removed = [&](const libremidi::input_port& p){
                std::cerr << "MIDI input lost " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                if (device.midiInRef_->is_port_open())
                    device.midiInRef_->close_port();
            },
            .output_added = [&](const libremidi::output_port& p){
                std::cerr << "MIDI output found " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                //Q what happens when multiple devices have the same port name, is that possible even in libremidi?

                if (device.midiOutRef_->is_port_open())
                    device.midiOutRef_->close_port();

                device.midiOutRef_->open_port(p);

                device.start();
            },
            .output_removed = [&](const libremidi::output_port& p){
                std::cerr << "MIDI output lost " << p.port_name << std::endl;

                if (!data_.devices.contains(p.port_name))
                    return;

                MidiDevice &device = data_.devices.at(p.port_name);

                if (device.midiOutRef_->is_port_open())
                    device.midiOutRef_->close_port();
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

        if (data_.lightshark.syncIntervalMs < 0)
            throw new std::invalid_argument("Invalid sync interval, must be >= 0");

        if (data_.devices.size() == 0)
            throw new std::invalid_argument("at least one device needed in configuration");

        for(auto & [portName, midiDevice] : data_.devices){
            if (midiDevice.vpots.size() != 8)
                throw new std::invalid_argument("configured vpot array must have 8 elements");

            if (midiDevice.banks.offset >= data_.banks.size())
                throw new std::invalid_argument("pb bank offset higher than actual bank count");
        }

//        j = data_;
//        std::cout << j << std::endl;

    }


    void Controller::start(){
        if (state_ != State::Stopped)
            return;

        state_ = State::Starting;

        initLsProxy();

        data_.init();

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

//        std::thread test([&]{
//            while(1){
//                std::this_thread::sleep_for(std::chrono::seconds(2));
//
//                std::cout << "> page " << lsProxy_.lsState().page << std::endl;
////                std::cout << "pb 1 level " << (int)lsProxy_.lsState().playbacks[0].level << std::endl;
//
////                for(auto & [key, dev] : data_.devices){
////
////                    std::cout << key << "-pb 1 level " << (int)dev.lsStateRef_->playbacks[0].level << std::endl;
////                    std::cout << key << "-pb 1 level " << (int)dev.sharedLsState_->playbacks[0].level << std::endl;
////                }
//
//            }
//        });

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


    void Controller::gotoBank(int bank){
        if (state_ != State::Running)
            return;

        for(auto & [key, midiDevice] : data_.devices){
            midiDevice.gotoBank(bank);
        }
    }

    void Controller::changeBank(int bankUpDown){
        if (state_ != State::Running)
            return;

        // ignore nonsense
        if (bankUpDown == 0)
            return;

        for(auto & [key, midiDevice] : data_.devices){
            midiDevice.changeBank(bankUpDown);
        }
//        int bank = data_.;
//
//        std::cerr << "current page " << lsProxy_.lsState().page << std::endl;
//        std::cerr << "target page " << page << std::endl;
//
//        gotoPage(page);
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

        // ignore nonsense
        if (pageUpDown == 0)
            return;

        int page = lsProxy_.lsState().page + pageUpDown;

//        std::cerr << "current page " << lsProxy_.lsState().page << std::endl;
//        std::cerr << "target page " << page << std::endl;

        gotoPage(page);
    }

    void Controller::midiDeviceEncoder(MidiDevice &midiDevice, MidiDevice::EncoderFunction encoderFunction, int relativeValue){

        if (relativeValue == 0)
            return;

        using EF = MidiDevice::EncoderFunction;
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
            case EF::EFGrandmaster:
            {
                int level = (lsProxy_.lsState().grandmasterLevel + relativeValue * kEncoderPbLevelStepsize) << 6;
                midiDeviceFader(midiDevice, MidiDevice::PlaybackId::PIGm, level);
            }
                break;
            case EF::EFSmChase:
            {
                int level = (lsProxy_.lsState().chaseSpeedMasterLevel + relativeValue * kEncoderPbLevelStepsize) << 6;
                midiDeviceFader(midiDevice, MidiDevice::PlaybackId::PISmChase, level);
            }
                break;
            case EF::EFSmFxSize:
            {
                int level = (lsProxy_.lsState().fxSizeMasterLevel + relativeValue * kEncoderPbLevelStepsize) << 6;
                midiDeviceFader(midiDevice, MidiDevice::PlaybackId::PISmFxSize, level);
            }
                break;
            case EF::EFSmFxSpeed:
            {
                int level = (lsProxy_.lsState().fxSpeedMasterLevel + relativeValue * kEncoderPbLevelStepsize) << 6;
                midiDeviceFader(midiDevice, MidiDevice::PlaybackId::PISmFxSpeed, level);
            }
                break;
            case EF::EFSelect:
                if (relativeValue < 0)
                    lsProxy_.pressButton(Buttons::SelectPrevious, false);
                else
                    lsProxy_.pressButton(Buttons::SelectNext, false);
                break;
            case EF::EFBank:
                changeBank(relativeValue);
                break;
            case EF::EFPage:
                changePage(relativeValue);
                break;
            case EF::EFButtonLayer:
                midiDevice.changeButtonLayer(relativeValue);
                break;
            case EF::EFNone:
                break;
        }
    }

    bool Controller::midiDeviceButton(MidiDevice &midiDevice, MidiDevice::ButtonAction & action, bool pressed){

        switch(action.fun){

            case MidiDevice::BFPageUp:
                lsProxy_.pressButton(Buttons::Clear, pressed);
                return true;
            case MidiDevice::BFPageDown:
                lsProxy_.pressButton(Buttons::Clear, pressed);
                return true;

            case MidiDevice::BFDBO:
                lsProxy_.pressButton(Buttons::DBO, pressed);
                return true;

            case MidiDevice::BFEdit:
                lsProxy_.pressButton(Buttons::Edit, pressed);
                return true;
            case MidiDevice::BFUpdate:
                lsProxy_.pressButton(Buttons::Update, pressed);
                return true;
            case MidiDevice::BFDelete:
                lsProxy_.pressButton(Buttons::Delete, pressed);
                return true;
            case MidiDevice::BFCopy:
                lsProxy_.pressButton(Buttons::Copy, pressed);
                return true;
            case MidiDevice::BFMove:
                lsProxy_.pressButton(Buttons::Move, pressed);
                return true;
            case MidiDevice::BFSet:
                lsProxy_.pressButton(Buttons::Set, pressed);
                return true;
            case MidiDevice::BFFan:
                lsProxy_.pressButton(Buttons::Fan, pressed);
                return true;

            case MidiDevice::BFClear:
                lsProxy_.pressButton(Buttons::Clear, pressed);
                return true;
            case MidiDevice::BFRec:
                lsProxy_.pressButton(Buttons::Rec, pressed);
                return true;

            case MidiDevice::BFFind:
                lsProxy_.pressButton(Buttons::Find, pressed);
                return true;

            case MidiDevice::BFSelectPlayback:
                lsProxy_.pressPlaybackButton(PlaybackButtons::Select, action.target,pressed);
                return true;

            case MidiDevice::BFGo:
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    lsProxy_.pressButton(Buttons::SelectedPlaybackGo, pressed);
                else
                    lsProxy_.pressPlaybackButton(PlaybackButtons::Go, action.target, pressed);
                return true;
            case MidiDevice::BFRelease:
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    lsProxy_.pressButton(Buttons::SelectedPlaybackRelease, pressed);
                else
                    lsProxy_.pressPlaybackButton(PlaybackButtons::Release, action.target, pressed);
                return true;
            case MidiDevice::BFPause:
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    lsProxy_.pressButton(Buttons::SelectedPlaybackPause, pressed);
                else
                    lsProxy_.pressPlaybackButton(PlaybackButtons::Pause, action.target, pressed);
                return true;
            case MidiDevice::BFNextCue:
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    lsProxy_.pressButton(Buttons::SelectedPlaybackNextCue, pressed);
                else
                    lsProxy_.pressPlaybackButton(PlaybackButtons::NextCue, action.target, pressed);
                return true;
            case MidiDevice::BFPreviousCue:
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    lsProxy_.pressButton(Buttons::SelectedPlaybackPreviousCue, pressed);
                else
                    lsProxy_.pressPlaybackButton(PlaybackButtons::PreviousCue, action.target, pressed);
                return true;
            case MidiDevice::BFTap:
                // this function requires a target
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    return false;
                lsProxy_.pressPlaybackButton(PlaybackButtons::Tap, action.target, pressed);
                return true;


            case MidiDevice::BFSelectFixture:
                lsProxy_.pressButton(Buttons::SelectFixture, pressed);
                return true;
            case MidiDevice::BFSelectGroup:
                lsProxy_.pressButton(Buttons::SelectGroup, pressed);
                return true;
            case MidiDevice::BFSelectNext:
                lsProxy_.pressButton(Buttons::SelectNext, pressed);
                return true;
            case MidiDevice::BFSelectPrevious:
                lsProxy_.pressButton(Buttons::SelectPrevious, pressed);
                return true;


            case MidiDevice::BFIntensity:
                lsProxy_.pressButton(Buttons::Intensity, pressed);
                return true;
            case MidiDevice::BFPosition:
                lsProxy_.pressButton(Buttons::Position, pressed);
                return true;
            case MidiDevice::BFColor:
                lsProxy_.pressButton(Buttons::Color, pressed);
                return true;
            case MidiDevice::BFBeam:
                lsProxy_.pressButton(Buttons::Beam, pressed);
                return true;
            case MidiDevice::BFAdvanced:
                lsProxy_.pressButton(Buttons::Advanced, pressed);
                return true;
            case MidiDevice::BFGobo:
                lsProxy_.pressButton(Buttons::Gobo, pressed);
                return true;
            case MidiDevice::BFFx:
                lsProxy_.pressButton(Buttons::FX, pressed);
                return true;


            case MidiDevice::BFChaseSpeedMasterReset:
                lsProxy_.pressButton(Buttons::ChaseSpeedMasterReset, pressed);
                return true;
            case MidiDevice::BFChaseSpeedMasterTap:
                lsProxy_.pressButton(Buttons::ChaseSpeedMasterTap, pressed);
                return true;
            case MidiDevice::BFFxSizeMasterReset:
                lsProxy_.pressButton(Buttons::FxSizeMasterReset, pressed);
                return true;
            case MidiDevice::BFFxSpeedMasterReset:
                lsProxy_.pressButton(Buttons::FxSpeedMasterReset, pressed);
                return true;
            case MidiDevice::BFFxSpeedMasterTap:
                lsProxy_.pressButton(Buttons::FxSpeedMasterTap, pressed);
                return true;

            case MidiDevice::BFExecutor:
            {
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    return false;

                int page, col, row;

                MidiDevice::ButtonAction::executorTarget(action.target, page, col, row);

                try {
                    lsProxy_.pressExecutor(page, col, row, pressed);
                } catch (std::exception e){
                    std::cerr << "Error pressing executor: " << e.what() << std::endl;
                }

                // executor state is shown otherwise
                return false;
            }
            case MidiDevice::BFExecutorRow:
            {
                if (action.target == MidiDevice::ButtonAction::kNoTarget)
                    return false;

                int page, row;

                MidiDevice::ButtonAction::executorRowTarget(action.target, page, row);

                lsProxy_.pressExecutor(page, row, pressed);
                return true;
            }

            case MidiDevice::BFNextBank:
                // on release only
                if (!pressed)
                    changeBank(1);
                return true;
            case MidiDevice::BFPreviousBank:
                // on release only
                if (!pressed)
                    changeBank(-1);
                return true;
            case MidiDevice::BFSelectBank:
//                if (action.target == MidiDevice::ButtonAction::kNoTarget)
//                    throw new std::invalid_argument("selectBank action must have a valid target");
                // on release only
                if (!pressed)
                    gotoBank(action.target);
                return true;

            case MidiDevice::BFSync:
                lsProxy_.syncRequest();
                return true;
            case MidiDevice::BFNone:
            default:
                break;
        }

        return false;
    }

    void Controller::midiDeviceFader(MidiDevice &device, MidiDevice::PlaybackId pid, int level){

        if (pid == MidiDevice::PlaybackId::PINone)
            return;

        // 14-bit -> 8-bit
        level >>= 6;

        // sanity check
        if (level < 0 || 255 < level)
            return;

//        std::cerr << "level = " << level << std::endl;

        switch(pid){
            case MidiDevice::PlaybackId::PIGm:
                if (lsProxy_.lsState().grandmasterLevel != level)
                    lsProxy_.setGrandmaster(level);
                break;
            case MidiDevice::PlaybackId::PISmChase:
                if (lsProxy_.lsState().chaseSpeedMasterLevel != level)
                    lsProxy_.setSubmaster(Submasters::ChaseSpeedMaster,level);
                break;
            case MidiDevice::PlaybackId::PISmFxSpeed:
                if (lsProxy_.lsState().fxSpeedMasterLevel != level)
                    lsProxy_.setSubmaster(Submasters::FxSpeedMaster,level);
                break;
            case MidiDevice::PlaybackId::PISmFxSize:
                if (lsProxy_.lsState().fxSizeMasterLevel != level)
                    lsProxy_.setSubmaster(Submasters::FxSizeMaster,level);
                break;
            default:
                if (lsProxy_.lsState().playbacks->level != level)
                    lsProxy_.setPlaybackLevel((int)pid, level);
        }

    }

    void Controller::lsPageChange(){
        for(auto & [key,midiDevice] : data_.devices){
            midiDevice.updateVPotLcd();
            midiDevice.updatePbLcd();
            midiDevice.updateTrackColors();
        }
    }

    void Controller::lsDeviceFader(MidiDevice::PlaybackId pid, int level){

        for(auto & [key,midiDevice] : data_.devices){
            midiDevice.updatePbLevel(pid, level);
        }

    }
    void Controller::lsExecutor(int page, int col, int row, bool is_active){
        for(auto & [key, midiDevice] : data_.devices){
            midiDevice.updateExecutorState(page, col, row, is_active);
        }
    }

} // LsMcuRemote