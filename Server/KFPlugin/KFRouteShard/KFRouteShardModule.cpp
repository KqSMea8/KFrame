﻿#include "KFRouteShardModule.h"
#include "KFRouteProxyManage.h"
#include "KFProtocol/KFProtocol.h"

namespace KFrame
{
    KFRouteShardModule::KFRouteShardModule()
    {
    }

    KFRouteShardModule::~KFRouteShardModule()
    {
    }

    void KFRouteShardModule::InitModule()
    {
        ///////////////////////////////////////////////////////////////////////////////
    }

    void KFRouteShardModule::BeforeRun()
    {
        __REGISTER_SERVER_LOST_FUNCTION__( &KFRouteShardModule::OnServerLostRouteProxy );
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __REGISTER_MESSAGE__( KFMsg::S2S_REGISTER_ROUTE_PROXY_REQ, &KFRouteShardModule::HandleRegisterRouteProxyReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_TRANSMIT_ROUTE_PROXY_MESSAGE_REQ, &KFRouteShardModule::HandleTransmitRouteProxyMessageReq );
    }

    void KFRouteShardModule::BeforeShut()
    {
        __UNREGISTER_SERVER_LOST_FUNCTION__();
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __UNREGISTER_MESSAGE__( KFMsg::S2S_REGISTER_ROUTE_PROXY_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_TRANSMIT_ROUTE_PROXY_MESSAGE_REQ );
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    __KF_SERVER_LOST_FUNCTION__( KFRouteShardModule::OnServerLostRouteProxy )
    {
        std::list< KFRouteProxy* > removes;
        for ( auto iter : _kf_proxy_manage->_kf_route_proxy._objects )
        {
            auto kfrouteproxy = iter.second;
            if ( kfrouteproxy->_handle_id == handleid )
            {
                removes.push_back( kfrouteproxy );
            }
        }

        for ( auto kfrouteproxy : removes )
        {
            _kf_proxy_manage->RemoveRouteProxy( kfrouteproxy->_server_id );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFRouteShardModule::HandleRegisterRouteProxyReq )
    {
        __PROTO_PARSE__( KFMsg::S2SRegisterRouteProxyReq );

        auto handleid = __KF_HEAD_ID__( kfguid );

        for ( auto i = 0; i < kfmsg.zonedata_size(); ++i )
        {
            auto pbdata = &kfmsg.zonedata( i );

            _kf_proxy_manage->AddRouteProxy( pbdata->zoneid(), pbdata->serverid(), handleid );
            KFLogger::LogLogic( KFLogger::Info, "[%s] register route server[%u=>%u]!",
                                __FUNCTION__, pbdata->serverid(), handleid );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFRouteShardModule::HandleTransmitRouteProxyMessageReq )
    {
        __PROTO_PARSE__( KFMsg::S2STransmitRouteProxyMessageReq );

        auto transmitdata = &kfmsg.transmitdata();

        auto kfrouteproxy = _kf_proxy_manage->FindRouteProxy( transmitdata->serverid() );
        if ( kfrouteproxy == nullptr )
        {
            return KFLogger::LogLogic( KFLogger::Error, "[%s] can't route server[%u] !",
                                       __FUNCTION__, transmitdata->serverid() );
        }

        KFMsg::S2STransmitRouteProxyMessageAck ack;
        ack.mutable_transmitdata()->CopyFrom( *transmitdata );
        _kf_tcp_server->SendNetMessage( kfrouteproxy->_handle_id, KFMsg::S2S_TRANSMIT_ROUTE_PROXY_MESSAGE_ACK, &ack );
    }
}
