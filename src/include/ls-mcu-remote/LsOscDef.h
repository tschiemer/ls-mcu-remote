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

#ifndef LS_MCU_REMOTE_LSOSCDEF_H
#define LS_MCU_REMOTE_LSOSCDEF_H

#include <cstdint>

namespace LsMcuRemote {

    enum class SyncTypes : uint8_t {
            Grandmaster = 1,
            Submasters  = 2,
            Page        = 4,
            Playbacks   = 8,
            Executors   = 16,
            All         = Grandmaster | Submasters | Page | Playbacks | Executors
    };

    /**
     * LightShark can be controlled remotely via TCP commands. The commands are formed in the same way as OSC commands, but by adding an S in first place.
     */
    constexpr char kLsOscTcpPrefix[] = "S";

    constexpr char kLsOscLsDefaultIpStr[] = "2.0.0.1";
    constexpr int kLsOscLsDefaultIncomingUdpPort = 8000;
    constexpr int kLsOscLsDefaultOutgoingUdpPort = 9000;

    constexpr int kLsOscPageCount = 30;
    constexpr int kLsOscPageMin = 1;
    constexpr int kLsOscPageMax = 30;


    constexpr int kLsOscPlaybackCount = 30;

    constexpr int kLsOscPlaybackIndexMin = 1;
    constexpr int kLsOscPlaybackIndexMax = 30;

    constexpr int kLsOscPlaybackValueMin = 0;
    constexpr int kLsOscPlaybackValueMax = 255;

    typedef uint8_t PlaybackLevel_t;

    constexpr int kLsOscExecutorPageCount = 2;
    constexpr int kLsOscExecutorPageMin = 1;
    constexpr int kLsOscExecutorPageMax = kLsOscExecutorPageCount;

    constexpr int kLsOscExecutorColumnCount = 8;
    constexpr int kLsOscExecutorColumnValueMin = 1;
    constexpr int kLsOscExecutorColumnValueMax = kLsOscExecutorColumnCount;

    constexpr int kLsOscExecutorRowCount = 6;
    constexpr int kLsOscExecutorRowValueMin = 1;
    constexpr int kLsOscExecutorRowValueMax = kLsOscExecutorRowCount;


    enum class Buttons : uint8_t {
        PageUp,
        PageDown,
//        SelectPage,

        DBO,

        Edit,
        Update,
        Delete,
        Copy,
        Move,
        Set,
        Fan,

        Find,

        Clear,
        Rec,

        SelectedPlaybackGo,
        SelectedPlaybackRelease,
        SelectedPlaybackPreviousCue,
        SelectedPlaybackNextCue,
        SelectedPlaybackPause,

        SelectFixture,
        SelectGroup,
        SelectNext,
        SelectPrevious,

        Intensity,
        Position,
        Color,
        Beam,
        Advanced,
        Gobo,
        FX,

        ReleaseAll,

        ChaseSpeedMasterReset,
        ChaseSpeedMasterTap,
        FxSizeMasterReset,
        FxSpeedMasterReset,
        FxSpeedMasterTap
    };

    enum class PlaybackButtons : uint8_t {
        Select,
        Go,
        Flash,
        Release,
        PreviousCue,
        NextCue,
        Pause,
        Tap
    };

    enum class Submasters : uint8_t {
        FxSpeedMaster,
        FxSizeMaster,
        ChaseSpeedMaster
    };

    /**
     * "/LS/Page/Up $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPageUp[]   = "/LS/Page/Up";

    /**
     * "/LS/Page/Down $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPageDown[] = "/LS/Page/Down";

    /**
     * "/LS/Page $page"
     * $page    1 - 30
     */
    constexpr char kLsOscPageSelect[] = "/LS/Page";

    /**
     * "/LS/DBO $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscDBO[] = "/LS/DBO";

    /**
     * "/LS/Edit $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscEdit[] = "/LS/Edit";

    /**
     * "/LS/Update $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscUpdate[] = "/LS/Update";

    /**
     * "/LS/Delete $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscDelete[] = "/Ls/Delete";

    /**
     * "/LS/Copy $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscCopy[] = "/LS/Copy";

    /**
     * "/LS/Move $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscMove[] = "/LS/Move";

    /**
     * "/LS/Move $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSet[] = "/LS/Move";

    /**
     * "/LS/Find $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscFan[] = "/LS/Fan";

    /**
     * "/LS/Find $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscFind[] = "/LS/Find";

    /**
     * "/LS/Clear $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscClear[] = "/LS/Clear";

    /**
     * "/LS/Rec $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscRec[] = "/LS/Rec";

    /**
     * "/LS/Select/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackSelect_Int[] = "/LS/Select/PB/%i";

    /**
     * "/LS/Go/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackGo_Int[] = "/LS/Go/PB/%i";

    /**
     * "/LS/Flash/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackFlash_Int[] = "/LS/Flash/PB/%i";

    /**
     * "/LS/Stop/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackRelease_Int[] = "/LS/Stop/PB/%i";

    /**
     * "/LS/Prev/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackPrevious_Int[] = "/LS/Prev/PB/%i";

    /**
     * "/LS/Next/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackNext_Int[] = "/LS/Next/PB/%i";

    /**
     * "/LS/Level/PB/$i $level"
     * $i       1 - 30
     * $level   0 - 255
     */
    constexpr char kLsOscPlaybackLevel_Int[] = "/LS/Level/PB/%i";

    /**
     * "/LS/Pause/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackPause_Int[] = "/LS/Pause/PB/%i";

    /**
     * "/LS/TAP/PB/$i $pressed"
     * $i       1 - 30
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPlaybackTap_Int[] = "/LS/TAP/PB/%i";

    /**
     * "/LS/Go/Main $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectedPlaybackGo[] = "/LS/Go/Main";

    /**
     * "/LS/Stop/Main $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectedPlaybackRelease[] = "/LS/Stop/Main";

    /**
     * "/LS/Prev/Main $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectedPlaybackPrevious[] = "/LS/Prev/Main";

    /**
     * "/LS/Next/Main $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectedPlaybackNext[] = "/LS/Next/Main";

    /**
     * "/LS/Pause/Main $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectedPlaybackPause[] = "/LS/Pause/Main";

    /**
     * "/LS/Level/GM $level"
     * $level   0 - 255
     */
    constexpr char kLsOscGrandmasterLevel[] = "/LS/Level/GM";

    /**
     * "/LS/Encoder/$i $level"
     * $i       1 - 4
     * $level   -1 - 1
     */
    constexpr char kLsOscEncoder_Int[] = "/LS/Encoder/%i";

    constexpr int kLsOscEncoderCount = 4;
    constexpr int kLsOscEncoderIndexMin = 1;
    constexpr int kLsOscEncoderIndexMax = 4;

    /**
     * "/LS/SelectFixture $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectFixture[] = "/LS/SelectFixture";

    /**
     * "/LS/SelectGroup $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectGroup[] = "/LS/SelectGroup";

    /**
     * "/LS/SelectionNext $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectNext[] = "/LS/SelectionNext";

    /**
     * "/LS/SelectionPrevious $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscSelectPrevious[] = "/LS/SelectionPrevious";

    /**
     * "/LS/Intensity $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscIntensity[] = "/LS/Intensity";

    /**
     * "/LS/Position $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscPosition[] = "/LS/Position";

    /**
     * "/LS/Color $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscColor[] = "/LS/Color";

    /**
     * "/LS/Beam $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscBeam[] = "/LS/Beam";

    /**
     * "/LS/Advanced $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscAdvanced[] = "/LS/Advanced";

    /**
     * "/LS/Gobo $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscGobo[] = "/LS/Gobo";

    /**
     * "/LS/Fx $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscFx[] = "/LS/Fx";

    /** Executor Push
     *
     * "/LS/Executor/$page/$column/$row $pressed"
     * $page    1 - 2
     * $column  1 - 8
     * $row     1 - 7?
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscExecutorTriggerOne_IntIntInt[] = "/LS/Executor/%i/%i/%i";

    /** Trigger Executor Row
     *
     * "/LS/Executor/Line/$page/$row $pressed"
     * $column  1 - 8
     * $row     1 - 6?
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscExecutorTriggerRow_IntInt[] = "/LS/Executor/Line/%i/%i";

    /** Sync All
     *
     * "/LS/Sync"
     *
     * Active like sending all the different sync requests.
     */
    constexpr char kLsOscSyncAll[] = "/LS/SyncAdv";

    /** Sync all playbacks only
     *
     * "/LS/Sync/Playbacks"
     *
     * Responses (30):
     * "/LS/Level/PB/$playback (float)$level"
     *
     * This format is an assumed bug, because sync all requests returns a different format for PBs:
     * "/LS/Active/PB/$playback (float)$level (int)cue-list-active
     */
    constexpr char kLsOscSyncPlaybacks[] = "/LS/Sync/Playbacks";
    constexpr char kLsOscSyncPlaybacksResponse_Int[] = "/LS/Active/PB/%i";
    constexpr char kLsOscSyncPlaybacksResponseEcho_Int[] = "/LS/Level/PB/%i";

    /** Sync all executors only
     *
     * "/LS/Sync/Executors"
     *
     * Responses (2 pages x 8 columns x 6 rows = 96):
     * "/LS/Active/Executor/$page/$column/$row $active"
     */
    constexpr char kLsOscSyncExecutors[] = "/LS/Sync/Executors";
    constexpr char kLsOscSyncExecutorsResponse_IntIntInt[] = "/LS/Active/Executor/%i/%i/%i";


    /** Sync submasters only
     *
     * "/LS/Sync/Submasters"
     *
     * Responses (3):
     *  "/LS/Level/SmSize (float)$level"
     *  "/LS/Level/SmSpeed (float)$level"
     *  "/LS/Level/SmChase (float)$level"
     */
    constexpr char kLsOscSyncSubmasters[] = "/LS/Sync/Submasters";
    constexpr char kLsOscSyncSubmastersResponseFxSizeMaster[] = "/LS/Level/SmSize";
    constexpr char kLsOscSyncSubmastersResponseFxSpeedMaster[] = "/LS/Level/SmSpeed";
    constexpr char kLsOscSyncSubmastersResponseChaseSpeedMaster[] = "/LS/Level/SmChase";


    /** Sync grandmaster
     *
     * "/LS/Sync/Master"
     *
     * Response:
     *  "/LS/Level/GM (float)$level"
     */
    constexpr char kLsOscSyncGrandmaster[] = "/LS/Sync/Master";
    constexpr char kLsOscSyncGrandmasterResponse[] = "/LS/Level/GM";

    /** Sync current page
     *
     * "/LS/Sync/Page"
     *
     * Response:
     *  "/LS/Page/$page"
     */
    constexpr char kLsOscSyncPage[] = "/LS/Sync/Page";
    constexpr char kLsOscSyncPageResponse[] = "/LS/Page";

    /**
     * "/LS/StopAll $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscReleaseAll[] = "/LS/StopAll";

    /** Playback TAP
     *
     * "/LS/ChaseValue/PB/$playback $speed"
     * $playback    1 - 30
     * $speed       5 - 200
     */
    constexpr char kLsOscPlaybackChaseSpeed_Int[] = "/LS/ChaseValue/PB/%i";
    constexpr int kLsOscPlaybackChaseSpeedMin = 5;
    constexpr int kLsOscPlaybackChaseSpeedMax = 200;

    /** Chase (?) Speed Master Tap
     *
     * "/LS/TAP $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscChaseSpeedMasterTap[] = "/LS/TAP";

    /** Reset Chase (?) Speed Master
     *
     * "/LS/Reset/SmChase $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscChaseSpeedMasterReset[] = "/LS/Reset/SmChase";

    /** Set Chase (?) Speed Master Level
     *
     * "/LS/Level/SmChase $level"
     * $level   0 - 255
     */
    constexpr char kLsOscChaseSpeedMasterSetLevel[] = "/LS/Level/SmChase";

    /** Reset Fx Size Master
     *
     * "/LS/Reset/SmSize $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscFxSizeMasterReset[] = "/LS/Reset/SmSize";

    /** Set Fx Size Master Level
     *
     * "/LS/Level/SmSize $level"
     * $level   0 - 255
     */
    constexpr char kLsOscFxSizeMasterSetLevel[] = "/LS/Level/SmSize";

    /** Reset Fx Speed Master
     *
     * "/LS/Reset/SmSpeed $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscFxSpeedMasterReset[] = "/LS/Reset/SmSpeed";

    /** Fx Speed Master Tap
     *
     * "/LS/FxTap $pressed"
     * $pressed 1 (pressed), 0 (released)
     */
    constexpr char kLsOscFxSpeedMasterTap[] = "/LS/FxTap";

    /** Set Fx Speed Master Level
     *
     * "/LS/Level/SmSpeed $level"
     * $level   0 - 255
     */
    constexpr char kLsOscFxSpeedMasterSetLevel[] = "/LS/Level/SmSpeed";

    /** Go to Cue of Playback
     *
     * "/LS/GotoCue/PB/$playback $cue"
     * $playback    1 - 30
     * $cue         cue number divided by 100 (ex 310 for cue 3.1)
     */
    constexpr char kLsOscCueGoTo_Int[] = "/LS/GotoCue/PB/%i";

    /** Arm/preload next GO Cue of Playback (preload cue)
     *
     * "/LS/PreloadCue/PB/$playback $cue"
     * $playback    1 - 30
     * $cue         cue number divided by 100 (ex 310 for cue 3.1)
     */
    constexpr char kLsOscCueArm_Int[] = "/LS/PreloadCue/PB/%i";

    /** Apply palette to fixture
     *
     * "/LS/ApplyUserPalette/Patch/$fixture $palette"
     * $fixture     Fixture ID (as shown in upper left corner of selection box)
     * $palette     Palette ID (hold down palette box, will be shown in upper left corner of window)
     */
    constexpr char kLsOscFixtureApplyPalette_Int[] = "/LS/ApplyUserPalette/Patch/$i";

    /** Apply palette to group
     *
     * "/LS/ApplyUserPalette/Group/$group $palette"
     * $group       Group ID (as shown in upper left corner of selection box)
     * $palette     Palette ID (hold down palette box, will be shown in upper left corner of window)
     */
    constexpr char kLsOscGroupApplyPalette_Int[] = "/LS/ApplyUserPalette/Group/$i";

}

#endif //LS_MCU_REMOTE_LSOSCDEF_H
