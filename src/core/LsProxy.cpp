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

#include "LsProxy.h"

#include <iostream>

#include <arpa/inet.h>

#include <oscpp/client.hpp>

namespace LsMcuRemote {


    void LsProxy::configure(Configuration config){
        // only set config when stopped
        if (state_ != State::Stopped)
            return;

        config_ = config;
   }

    void LsProxy::runloopSync(){

        auto lastSync = std::chrono::system_clock::now();
        syncRequest(config_.synTypes);

        while (state_ == State::Running){

            auto now = std::chrono::system_clock::now();

            if (now - lastSync > config_.syncInterval){
                syncRequest(config_.synTypes);
                lastSync = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds (100));
        }

    }


    void LsProxy::runloopUdpReceiver(){

        state_ = State::Running;

        char buffer[kUdpReceiveBufferSize];
        struct sockaddr_in cliaddr;
        socklen_t len;
        int n;

        while (state_ == State::Running){

            len = sizeof(cliaddr);  //len is value/result

            memset(&cliaddr, 0, sizeof(cliaddr));

            if ((n = recvfrom(sockfd_, buffer, sizeof(buffer), 0, (struct sockaddr *) &cliaddr, &len)) < 0){

                // if timeout, just ignore
                if (errno == EAGAIN){
                    // do nothing
                } else {
                    std::cerr << "unknpown socket error, stopping" << errno << std::endl;

                    // stop loop and end thread
                    state_ = State::Stopping;
                }
            } else {

//                std::cerr << "Rx " << n << std::endl;

                try {
                    OSCPP::Server::Packet packet(buffer,n);

                    handleOscPacket(packet);

                } catch (std::exception e){
                    std::cerr << "couldnt handle OSC packet: " << e.what() << std::endl;
                }
            }

        }
    }

    void LsProxy::handleOscPacket(OSCPP::Server::Packet packet){

        // only expecting messages
        if (packet.isBundle()){
            OSCPP::Server::Bundle bundle(packet);

            // Get packet stream
            OSCPP::Server::PacketStream packets(bundle.packets());

            // Iterate over all the packets and call handlePacket recursively.
            // Cuidado: Might lead to stack overflow!
            while (!packets.atEnd()) {
                handleOscPacket(packets.next());
            }
        }
        else if (packet.isMessage()){

            // Convert to message
            OSCPP::Server::Message msg(packet);


            OSCPP::Server::ArgStream args(msg.args());

            int i,page,row,col;

            try {
//                std::cerr << "addr " << msg.address() << std::endl;

                if (1 == sscanf(msg.address(), kLsOscSyncPlaybacksResponse_Int, &i)){

                    if (i < kLsOscPlaybackIndexMin || kLsOscPlaybackIndexMax < i) return;

                    int playback = i - kLsOscPlaybackIndexMin;
                    int level = args.int32();
                    int is_active = args.int32();

                    if (lsState_.playbacks[playback].level == level && lsState_.playbacks[playback].is_active == is_active)
                        return;

                    lsState_.playbacks[playback].level = level;
                    lsState_.playbacks[playback].is_active = is_active;

                    // also set the page on which the playback was activated
                    lsState_.playbacks[playback].page = lsState_.page;

                    if (config_.onPlaybackSync)
                        config_.onPlaybackSync(playback,level,is_active);

//                    std::cerr << "PB " << i << " level=" <<level << "active=" << is_active  << std::endl;
                }
                else if (1 == sscanf(msg.address(), kLsOscSyncPlaybacksResponseEcho_Int, &i)){

                    if (i < kLsOscPlaybackIndexMin || kLsOscPlaybackIndexMax < i) return;

                    int playback = i - kLsOscPlaybackIndexMin;
                    int level = args.int32();

                    if (lsState_.playbacks[playback].level == level)
                        return;

                    lsState_.playbacks[playback].level = level;

                    int is_active = lsState_.playbacks[playback].is_active;

                    if (config_.onPlaybackSync)
                        config_.onPlaybackSync(playback,level,is_active);

//                    std::cerr << "PB " << i << " level=" << level  << std::endl;
                }
                else if (3 == sscanf(msg.address(), kLsOscSyncExecutorsResponse_IntIntInt, &page, &col, &row)){

                    if (page < kLsOscExecutorPageMin || kLsOscExecutorPageMax < page) return;
                    if (page < kLsOscExecutorColumnValueMin || kLsOscExecutorColumnValueMax < col) return;
                    if (page < kLsOscExecutorRowValueMin || kLsOscExecutorRowValueMin < row) return;

                    page--;
                    col--;
                    row--;

                    bool is_active = args.int32();

                    if (lsState_.executors[page][col][row] == is_active)
                        return;

                    lsState_.executors[page][col][row] = is_active;

                    if (config_.onExecutorSync)
                        config_.onExecutorSync(page,col,row,is_active);

//                    std::cerr << "Exec " << " page="  << page << " col=" << col << " row=" << row << " active=" << is_active << std::endl;
                }
                else if (strcmp(msg.address(), kLsOscSyncPageResponse) == 0){

                    int page = args.int32();

                    if (lsState_.page == page)
                        return;

                    lsState_.page = page;

                    if (config_.onPageSync)
                        config_.onPageSync(page);

//                    std::cerr << "Page " << page << std::endl;
                }
                else if (strcmp(msg.address(), kLsOscSyncSubmastersResponseChaseSpeedMaster) == 0){

                    int level = args.int32();

                    if (lsState_.fxSpeedMasterLevel == level)
                        return;

                    lsState_.chaseSpeedMasterLevel = level;

                    if (config_.onSubmasterSync)
                        config_.onSubmasterSync(Submasters::ChaseSpeedMaster,level);

//                        std::cerr << "Chase SM level=" << level << std::endl;
                }
                else if (strcmp(msg.address(), kLsOscSyncSubmastersResponseFxSizeMaster) == 0){

                    int level = args.int32();

                    if (lsState_.fxSpeedMasterLevel == level)
                        return;

                    lsState_.fxSizeMasterLevel = level;

                    if (config_.onSubmasterSync)
                        config_.onSubmasterSync(Submasters::FxSizeMaster,level);

//                        std::cerr << "Fx Size level=" << level << std::endl;
                }
                else if (strcmp(msg.address(), kLsOscSyncSubmastersResponseFxSpeedMaster) == 0){

                    int level = args.int32();

                    if (lsState_.fxSpeedMasterLevel == level)
                        return;

                    lsState_.fxSpeedMasterLevel = level;

                    if (config_.onSubmasterSync)
                        config_.onSubmasterSync(Submasters::FxSpeedMaster,level);

//                        std::cerr << "Fx Speed level=" << level << std::endl;
                }
                else if (strcmp(msg.address(), kLsOscSyncGrandmasterResponse) == 0){

                    int level = args.int32();

                    if (lsState_.grandmasterLevel == level)
                        return;

                    lsState_.grandmasterLevel = level;

                    if (config_.onGrandmasterSync)
                        config_.onGrandmasterSync(level);

//                        std::cerr << "Grandmaster level=" << level << std::endl;
                }
                else {

                    std::cerr << "Unhandled OSC msg: " << msg.address() << std::endl;
                }
            } catch(std::exception e){
                // note sometimes Lightshark sends invalid OSC packets, so trying to access the data (arguments) throws another error....
                std::cerr << "Error handling OSC msg addr = " << msg.address() << " with args (" << args.size() << ") / exception: " << e.what() << std::endl;
            }
        }

    }

    void LsProxy::sendUdp(void * data, size_t size){

//        std::cout << "sendto " << config_.lightsharkHostIp << ":" << config_.lightsharkPort << std::endl;

        struct sockaddr_in servaddr;

        // Set lightshark endpoint address
        memset(&servaddr, 0, sizeof(servaddr));

        servaddr.sin_family    = AF_INET; // IPv4
        servaddr.sin_addr.s_addr = inet_addr(config_.lightsharkHostIp.data());
        servaddr.sin_port = htons(config_.lightsharkPort);

        int len = sizeof(servaddr);
        int n = 0;

        if ((n = sendto(sockfd_, data, size, 0, (const struct sockaddr *) &servaddr, len)) < 0){
            std::cerr << "sendto fail errno " << n << " " << errno << std::endl;
        }
    }

    void LsProxy::start() {
        if (state_ != State::Stopped)
            return;

        state_ = State::Starting;

        try {

            // Creating socket file descriptor
            if ( (sockfd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
                throw new std::runtime_error("socket creation failed");
            }

            //// set some socket options
            // reuse port
            int optval = 1;
//            setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

//            struct ip_mreq m;
//            m.imr_interface.s_addr = inet_addr("10.0.0.148");
//            m.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
//            setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m, sizeof(m));

            // set recv timeout
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 500000;
            setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));


            struct sockaddr_in servaddr;
            memset(&servaddr, 0, sizeof(servaddr));

            // Filling server information
            servaddr.sin_family    = AF_INET; // IPv4
            servaddr.sin_addr.s_addr = INADDR_ANY;
            servaddr.sin_port = htons(config_.localPort);

            // Bind the socket with the server address
            if (bind(sockfd_, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
                throw new std::runtime_error("bind failed");
            }

            udpReceiveThread_ = new std::thread(&LsProxy::runloopUdpReceiver, this);

        } catch (std::exception e) {
            state_ = State::Stopped;

            if (sockfd_ != -1){
                close(sockfd_);
                sockfd_ = -1;
            }

            std::cerr << "Error while starting: " << e.what() << "(Errno " << errno << ")" << std::endl;

            return;
        }

        // iff sync interval 0 -> require external sync request
        if (config_.syncInterval > NoAutoSync){
            try {

                //wait for reading thread to start to also start sync request thread
                while(state_ == State::Starting)
                    std::this_thread::sleep_for(std::chrono::microseconds(10));

                syncTimerThread_ = new std::thread(&LsProxy::runloopSync, this);
            } catch (std::exception e){
                stop();

                std::cerr << "Error while staring sync thread: " << e.what() << std::endl;
                return;
            }
        }
    }

    void LsProxy::stop() {
        if (state_ != State::Running)
            return;

        state_ = State::Stopping;

        if (udpReceiveThread_ != nullptr){

            if (syncTimerThread_ != nullptr){
                syncTimerThread_->join();
                delete syncTimerThread_;
            }

            udpReceiveThread_->join();
            delete udpReceiveThread_;
        }

        state_ = State::Stopped;
    }

    void LsProxy::syncRequest(SyncTypes syncType){

        if (state_ != State::Running){
            return;
        }

        char buf[128];
        OSCPP::Client::Packet packet(buf, sizeof(buf));

        try {

            switch (syncType) {
                case SyncTypes::Grandmaster:
                    packet.openMessage(kLsOscSyncGrandmaster, 0).closeMessage();
                    break;
                case SyncTypes::Submasters:
                    packet.openMessage(kLsOscSyncSubmasters, 0).closeMessage();
                    break;
                case SyncTypes::Page:
                    packet.openMessage(kLsOscSyncPage, 0).closeMessage();
                    break;
                case SyncTypes::Playbacks:
                    packet.openMessage(kLsOscSyncPlaybacks, 0).closeMessage();
                    break;
                case SyncTypes::Executors:
                    packet.openMessage(kLsOscSyncExecutors, 0).closeMessage();
                    break;
                case SyncTypes::All:
                    packet.openMessage(kLsOscSyncAll, 0).closeMessage();
                    break;
            }
        } catch (std::exception e){
            std::cerr << "asdf"<< e.what() <<std::endl;
        }

        try {
            sendUdp(packet.data(), packet.size());
        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }


    void LsProxy::setGrandmaster(int level){

        if (state_ != State::Running){
            return;
        }

        if (level < 0 || 255 < level){
            throw new std::out_of_range("Level must be in[0,255]");
        }

        try {
            char buf[128];
            OSCPP::Client::Packet packet(buf, sizeof(buf));

            packet.openMessage(kLsOscGrandmasterLevel,1).int32(level).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }

    }

    void LsProxy::setSubmaster(Submasters submaster, int level){

        if (state_ != State::Running){
            return;
        }

        if (level < 0 || 255 < level){
            throw new std::out_of_range("Level must be in[0,255]");
        }

        try {
            char buf[128];
            OSCPP::Client::Packet packet(buf, sizeof(buf));

            switch(submaster){

                case Submasters::ChaseSpeedMaster:
                    packet.openMessage(kLsOscChaseSpeedMasterSetLevel,1).int32(level).closeMessage();
                    break;

                case Submasters::FxSizeMaster:
                    packet.openMessage(kLsOscFxSizeMasterSetLevel,1).int32(level).closeMessage();
                    break;

                case Submasters::FxSpeedMaster:
                    packet.openMessage(kLsOscFxSpeedMasterSetLevel,1).int32(level).closeMessage();
                    break;
            }


            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

    void LsProxy::selectPage(int page){

        if (state_ != State::Running)
            return;


        if (page < kLsOscPageMin || kLsOscPageMax < page)
            throw new std::out_of_range("page must be in [1,30], starting at 1!");

        try {
            char buf[128];
            OSCPP::Client::Packet packet(buf, sizeof(buf));

            packet.openMessage(kLsOscPageSelect,1).int32(page).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

    void LsProxy::pressButton(Buttons btn, bool pressed){

        if (state_ != State::Running){
            return;
        }

        try {
            char * addr;
            char buf[128];

            OSCPP::Client::Packet packet(buf, sizeof(buf));

            switch(btn){
                case Buttons::PageUp:
                    addr = const_cast<char*>(kLsOscPageUp);
                    break;
                case Buttons::PageDown:
                    addr = const_cast<char*>(kLsOscPageDown);
                    break;

                case Buttons::DBO:
                    addr = const_cast<char*>(kLsOscDBO);
                    break;

                case Buttons::Edit:
                    addr = const_cast<char*>(kLsOscEdit);
                    break;
                case Buttons::Update:
                    addr = const_cast<char*>(kLsOscUpdate);
                    break;
                case Buttons::Delete:
                    addr = const_cast<char*>(kLsOscDelete);
                    break;
                case Buttons::Copy:
                    addr = const_cast<char*>(kLsOscCopy);
                    break;
                case Buttons::Move:
                    addr = const_cast<char*>(kLsOscMove);
                    break;
                case Buttons::Set:
                    addr = const_cast<char*>(kLsOscSet);
                    break;
                case Buttons::Fan:
                    addr = const_cast<char*>(kLsOscFan);
                    break;

                case Buttons::Find:
                    addr = const_cast<char*>(kLsOscFind);
                    break;

                case Buttons::Clear:
                    addr = const_cast<char*>(kLsOscClear);
                    break;
                case Buttons::Rec:
                    addr = const_cast<char*>(kLsOscRec);
                    break;

                case Buttons::SelectedPlaybackGo:
                    addr = const_cast<char*>(kLsOscSelectedPlaybackGo);
                    break;
                case Buttons::SelectedPlaybackRelease:
                    addr = const_cast<char*>(kLsOscSelectedPlaybackRelease);
                    break;
                case Buttons::SelectedPlaybackPreviousCue:
                    addr = const_cast<char*>(kLsOscSelectedPlaybackPrevious);
                    break;
                case Buttons::SelectedPlaybackNextCue:
                    addr = const_cast<char*>(kLsOscSelectedPlaybackNext);
                    break;
                case Buttons::SelectedPlaybackPause:
                    addr = const_cast<char*>(kLsOscSelectedPlaybackPause);
                    break;

                case Buttons::SelectFixture:
                    addr = const_cast<char*>(kLsOscSelectFixture);
                    break;
                case Buttons::SelectGroup:
                    addr = const_cast<char*>(kLsOscSelectGroup);
                    break;
                case Buttons::SelectNext:
                    addr = const_cast<char*>(kLsOscSelectNext);
                    break;
                case Buttons::SelectPrevious:
                    addr = const_cast<char*>(kLsOscSelectPrevious);
                    break;

                case Buttons::Intensity:
                    addr = const_cast<char*>(kLsOscIntensity);
                    break;
                case Buttons::Position:
                    addr = const_cast<char*>(kLsOscPosition);
                    break;
                case Buttons::Color:
                    addr = const_cast<char*>(kLsOscColor);
                    break;
                case Buttons::Beam:
                    addr = const_cast<char*>(kLsOscBeam);
                    break;
                case Buttons::Advanced:
                    addr = const_cast<char*>(kLsOscAdvanced);
                    break;
                case Buttons::Gobo:
                    addr = const_cast<char*>(kLsOscGobo);
                    break;
                case Buttons::FX:
                    addr = const_cast<char*>(kLsOscFx);
                    break;

                case Buttons::ReleaseAll:
                    addr = const_cast<char*>(kLsOscReleaseAll);
                    break;

                case Buttons::ChaseSpeedMasterReset:
                    addr = const_cast<char*>(kLsOscChaseSpeedMasterReset);
                    break;
                case Buttons::ChaseSpeedMasterTap:
                    addr = const_cast<char*>(kLsOscChaseSpeedMasterTap);
                    break;
                case Buttons::FxSizeMasterReset:
                    addr = const_cast<char*>(kLsOscFxSizeMasterReset);
                    break;
                case Buttons::FxSpeedMasterReset:
                    addr = const_cast<char*>(kLsOscFxSpeedMasterReset);
                    break;
                case Buttons::FxSpeedMasterTap:
                    addr = const_cast<char*>(kLsOscFxSpeedMasterTap);
                    break;

//                default:
//                    throw new std::runtime_error("oopsy, forgot that button");
            }

            packet.openMessage(addr,1).int32(pressed).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }


    void LsProxy::pressPlaybackButton(PlaybackButtons btn, int playback, bool pressed){

        if (state_ != State::Running){
            return;
        }

        if (playback < 0 || kLsOscPlaybackCount < playback){
            throw new std::out_of_range("Level must be in [0,29], starting at 0!");
        }

        try {
            char * cmd;
            char addr[128],buf[128];

            OSCPP::Client::Packet packet(buf, sizeof(buf));

            switch(btn){
                case PlaybackButtons::Select:
                    cmd = const_cast<char*>(kLsOscPlaybackSelect_Int);
                    break;
                case PlaybackButtons::Go:
                    cmd = const_cast<char*>(kLsOscPlaybackGo_Int);
                    break;
                case PlaybackButtons::Flash:
                    cmd = const_cast<char*>(kLsOscPlaybackFlash_Int);
                    break;
                case PlaybackButtons::Release:
                    cmd = const_cast<char*>(kLsOscPlaybackRelease_Int);
                    break;
                case PlaybackButtons::PreviousCue:
                    cmd = const_cast<char*>(kLsOscPlaybackPrevious_Int);
                    break;
                case PlaybackButtons::NextCue:
                    cmd = const_cast<char*>(kLsOscPlaybackNext_Int);
                    break;
                case PlaybackButtons::Pause:
                    cmd = const_cast<char*>(kLsOscPlaybackPause_Int);
                    break;
            }

            std::snprintf(addr, sizeof(addr), cmd,
                          playback+kLsOscPlaybackIndexMin);

            packet.openMessage(addr, 1).int32(pressed).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

    void LsProxy::setPlaybackLevel(int playback, PlaybackLevel_t  level){

        if (state_ != State::Running){
            return;
        }

        if (playback < 0 || kLsOscPlaybackCount < playback){
            throw new std::out_of_range("Level must be in [0,29], starting at 0!");
        }


        try {
            char addr[128], buf[128];

            std::snprintf(addr, sizeof(addr), kLsOscPlaybackLevel_Int, playback+kLsOscPlaybackIndexMin);

            OSCPP::Client::Packet packet(buf, sizeof(buf));

            packet.openMessage(addr,1).int32(level).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

    void LsProxy::pressExecutor(int page, int column, int row, bool pressed){

        if (state_ != State::Running){
            return;
        }

        if (page < 0 || kLsOscExecutorPageCount < page){
            throw new std::out_of_range("page must be in [0,1], starting at 0!");
        }
        if (column < 0 || kLsOscExecutorColumnCount < column){
            throw new std::out_of_range("column must be in [0,7], starting at 0!");
        }
        if (row < 0 || kLsOscExecutorRowCount < row){
            throw new std::out_of_range("column must be in [0,5], starting at 0!");
        }


        try {
            char addr[128], buf[128];

            std::snprintf(addr, sizeof(addr), kLsOscExecutorTriggerOne_IntIntInt,
                          page+kLsOscExecutorPageMin,
                          column+kLsOscExecutorColumnValueMin,
                          row+kLsOscExecutorRowValueMin);

            OSCPP::Client::Packet packet(buf, sizeof(buf));

            packet.openMessage(addr,1).int32(pressed).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

    void LsProxy::pressExecutorRow(int page, int row, bool pressed){

        if (state_ != State::Running){
            return;
        }

        if (page < 0 || kLsOscExecutorPageCount < page){
            throw new std::out_of_range("page must be in [0,1], starting at 0!");
        }
        if (row < 0 || kLsOscExecutorRowCount < row){
            throw new std::out_of_range("column must be in [0,5], starting at 0!");
        }


        try {
            char addr[128], buf[128];

            std::snprintf(addr, sizeof(addr), kLsOscExecutorTriggerRow_IntInt,
                          page+kLsOscExecutorPageMin,
                          row+kLsOscExecutorRowValueMin);

            OSCPP::Client::Packet packet(buf, sizeof(buf));

            packet.openMessage(addr,1).int32(pressed).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

    void LsProxy::encoderChange(int encoder, int relativeValue){

        if (state_ != State::Running){
            return;
        }

        std::cerr << "encoder " << encoder << " -> " << relativeValue << std::endl;

        if (encoder < 0 || kLsOscEncoderCount <= encoder)
            throw new std::out_of_range("encoder must be in [0,3] starting at 0!");


        try {
            char addr[128], buf[128];

            std::snprintf(addr, sizeof(addr), kLsOscEncoder_Int,
                          encoder+kLsOscEncoderIndexMin);

            OSCPP::Client::Packet packet(buf, sizeof(buf));

            packet.openMessage(addr,1).int32(relativeValue).closeMessage();

            sendUdp(packet.data(), packet.size());

        } catch(std::exception e) {
            std::cerr << "Error sending: " << e.what() << std::endl;
        }
    }

} // LsMcuRemote