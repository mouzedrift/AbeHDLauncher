#pragma once

#include <QUrl>
#include <QString>

enum class eFileId
{
    eRelive,
    eDove,
    eBirdPortal,
    eAbeFMV,
    eAbeOriginal,
    eBoomMachine,
    eCrawlingSlig,
    eDoorLock,
    eDoors,
    eDrill,
    eElectricWall,
    eFallingItems,
    eFleech,
    eFlyingSlig,
    eFootSwitches,
    eGibs,
    eGlukkons,
    eGreeter,
    eIcons,
    eMenuGlukkonSpeak,
    eMineCar,
    eMotionDetector,
    eParamite,
    ePullRingRope,
    eRockSack,
    eRopes,
    eScrab,
    eSecurityDoor,
    eSecurityOrb,
    eSlamDoors,
    eSligAENoTubes,
    eSligAETubes,
    eSligAONoTubes,
    eSligAOTubes,
    eSligMONoTubes,
    eSligMOTubes,
    eSlog,
    eSlurg,
    eThrowables,
    eTimedMine,
    eTorturedMudokon,
    eUngrouped,
    eUXB,
    eWorkWheel,
    eTrapDoors,
    eLifts,
    eLever,
    eSlapLock,
    eBoneBag,
    eCams,
};

struct AssetFile
{
    QUrl downloadUrl;
    QString fileName;
    eFileId fileId;
};
