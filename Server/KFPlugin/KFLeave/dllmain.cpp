﻿#include "KFPlatform.h"
#include "KFEnterPlugin.h"
#include "KFPlugin/KFGlobal.h"
#include "KFPlugin/KFPluginManage.h"

__KF_EXPORT__ __KF_PLUGIN_ENRTY__( KFrame::KFEnterPlugin );
__KF_EXPORT__ __KF_PLUGIN_STOP__( KFrame::KFEnterPlugin );
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#if __KF_PLATFORM__ == __PLATFORM_WIN__
    //////////////////////////////////////////////////////////////////////////////////////////////////
    #ifdef __KF_DEBUG__
        #pragma comment( lib, "KFPlugind.lib" )
        #pragma comment( lib, "KFMemoryd.lib" )
        #pragma comment( lib, "KFUtilityd.lib" )
        #pragma comment( lib, "KFTimed.lib" )
        #pragma comment( lib, "KFLoggerd.lib" )
        #pragma comment( lib, "KFDatad.lib" )
        #pragma comment( lib, "KFThreadd.lib" )
        #pragma comment( lib, "KFProtod.lib" )
        #pragma comment( lib, "libprotobufd.lib" )
        #pragma comment( lib, "KFXmld.lib" )
        #pragma comment( lib, "KFConvertd.lib" )
    #endif
    //////////////////////////////////////////////////////////////////////////////////////////////////
    #ifdef __KF_RELEASE__
        #pragma comment( lib, "KFPlugin.lib" )
        #pragma comment( lib, "KFMemory.lib" )
        #pragma comment( lib, "KFUtility.lib" )
        #pragma comment( lib, "KFTime.lib" )
        #pragma comment( lib, "KFLogger.lib" )
        #pragma comment( lib, "KFData.lib" )
        #pragma comment( lib, "KFThread.lib" )
        #pragma comment( lib, "KFProto.lib" )
        #pragma comment( lib, "libprotobuf.lib" )
        #pragma comment( lib, "KFXml.lib" )
        #pragma comment( lib, "KFConvert.lib" )
    #endif
    //////////////////////////////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#if __KF_PLATFORM__ == __PLATFORM_LINUX__

    //////////////////////////////////////////////////////////////////////////////////////////////////
    #ifdef __KF_DEBUG__


    #endif
    //////////////////////////////////////////////////////////////////////////////////////////////////
    #ifdef __KF_RELEASE__


    #endif
    //////////////////////////////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////