﻿#ifndef __NET_CONNECTOR_H__
#define __NET_CONNECTOR_H__

#include "KFNetSession.h"

namespace KFrame
{
    class KFNetServices;
    class KFNetConnector : public KFNetSession
    {
    public:

        KFNetConnector();
        ~KFNetConnector();

        // 初始化
        void InitConnector( uint32 id, KFNetServices* netservices );

        // 弹出一个收到消息
        KFNetMessage* PopNetMessage();

        // 发送消息
        bool SendNetMessage( uint32 msgid, const char* data, uint32 length );
        bool SendNetMessage( uint32 objectid, uint32 msgid, const char* data, uint32 length );

    protected:
        // 弹出一个消息
        KFNetMessage* PopMessage();

        // 发送单一消息
        bool SendSingleMessage( uint32 objectid, uint32 msgid, const char* data, uint32 length );

        // 发送拆包消息
        bool SendMultiMessage( uint32 objectid, uint32 msgid, const char* data, uint32 length );
        //////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////

    protected:
        // 网络服务
        KFNetServices* _net_services;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

#endif