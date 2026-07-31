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

#ifndef LS_MCU_REMOTE_LSPROXY_H
#define LS_MCU_REMOTE_LSPROXY_H

#include <cstdint>
#include <functional>
#include <chrono>
#include <thread>

#include <unistd.h>

#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include <oscpp/server.hpp>

#include "LsOscDef.h"


namespace LsMcuRemote {

    class LsProxy {


        private:

            const int kUdpReceiveBufferSize = 2000;

            /**
             *  Type declarations
             */
        public:

            enum class State : uint8_t {
                Stopped, Starting, Running, Stopping
            };


            typedef struct LsState_st {
                PlaybackLevel_t grandmasterLevel = 0;
                PlaybackLevel_t  chaseSpeedMasterLevel = 0;
                PlaybackLevel_t  fxSizeMasterLevel = 0;
                PlaybackLevel_t  fxSpeedMasterLevel = 0;
                int page = 0;
                struct Playback_st {
                    uint8_t level = 0;
                    bool is_active = false;
                    int page = 0;
                } playbacks[kLsOscPlaybackCount];
                bool executors[kLsOscExecutorPageCount][kLsOscExecutorColumnCount][kLsOscExecutorRowCount];
            } LsState;

            struct Configuration {

                std::string_view lightsharkHost = kLsOscLsDefaultIpStr;
                uint16_t lightsharkPort = kLsOscLsDefaultIncomingUdpPort;
                uint16_t localPort = kLsOscLsDefaultOutgoingUdpPort;

                std::chrono::milliseconds syncInterval = std::chrono::milliseconds(250);

                LsMcuRemote::SyncTypes synTypes = SyncTypes::All;

                /**
                 * onGrandmasterSync(int level)
                 */
                std::function<void(PlaybackLevel_t)> onGrandmasterSync = nullptr;

                /**
                 * onSubmasterSync(Submasters sm, int level)
                 */
                std::function<void(Submasters, PlaybackLevel_t)> onSubmasterSync = nullptr;

                /**
                 * onPageSync(int page)
                 */
                std::function<void(int)> onPageSync = nullptr;

                /**
                 * onPlaybackSync(int playbackIndex, int level, bool active)
                 */
                std::function<void(int, PlaybackLevel_t, bool)> onPlaybackSync = nullptr;

                /**
                 * onExecutorSync(int page, int column, int row, bool active)
                 */
                std::function<void(int,int,int,bool)> onExecutorSync = nullptr;

            } ;

        /**
         * Constants
         */
        public:

            static constexpr std::chrono::milliseconds NoAutoSync = std::chrono::milliseconds (0);


        private:

            State state_ = State::Stopped;

            Configuration config_;

            LsState lsState_;

            std::thread * runloopUdpReceiverThread_ = nullptr;
            std::thread * runloopSyncThread_ = nullptr;

            struct {
                asio::io_context io_context;

                asio::ip::udp::socket * udp_socket = nullptr;
                asio::ip::udp::endpoint udp_ls_endpoint;

                asio::ip::tcp::socket * tcp_socket;
            } net_;

        public:

            LsProxy(){

            }

        private:

            void runloopSync();

            void runloopUdpReceiver();

            void initUdp();
            void deinitUdp();

            void sendUdp(void * data, size_t len);

            void handleOscPacket(OSCPP::Server::Packet packet);


        public:

            void configure(Configuration config);

            inline State state() { return state_; }

            inline LsState & lsState(){ return lsState_; }

            void start();
            void stop();

        public:

            void syncRequest(SyncTypes syncType = SyncTypes::All);

            void setGrandmaster(int level);
            void setSubmaster(Submasters submaster, int level);

            void selectPage(int page);

            void pressButton(Buttons btn, bool pressed = false);

            void pressPlaybackButton(PlaybackButtons btn, int playback, bool pressed = false);
            void setPlaybackLevel(int playback, PlaybackLevel_t level);

            void pressExecutor(int page, int column, int row, bool pressed = false);
            void pressExecutorRow(int page, int row, bool pressed = false);

            void encoderChange(int encoder, int value);
    };

} // LsMcuRemote

#endif //LS_MCU_REMOTE_LSPROXY_H
