﻿#ifndef __KF_ZONE_CONFIG_H__
#define __KF_ZONE_CONFIG_H__

#include "KFrame.h"
#include "KFConHash/KFConHash.h"
#include "KFZoneInterface.h"
#include "KFConfig/KFConfigInterface.h"

namespace KFrame
{
    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    class KFZoneConfig : public KFConfig, public KFSingleton< KFZoneConfig >
    {
    public:
        KFZoneConfig();
        ~KFZoneConfig();

        bool LoadConfig( const char* file );


    public:
        // 分区信息
        KFZone _kf_zone;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////
    static auto _kf_zone_config = KFZoneConfig::Instance();
    //////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif