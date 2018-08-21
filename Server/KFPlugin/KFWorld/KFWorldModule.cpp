﻿#include "KFWorldModule.h"
#include "KFOnlineEx.h"
#include "KFJson.h"
#include "KFProtocol/KFProtocol.h"

namespace KFrame
{
    KFWorldModule::KFWorldModule()
    {
    }

    KFWorldModule::~KFWorldModule()
    {

    }

    void KFWorldModule::InitModule()
    {

    }

    void KFWorldModule::BeforeRun()
    {
        __REGISTER_SERVER_DISCOVER_FUNCTION__( &KFWorldModule::OnServerDisCoverGame );
        __REGISTER_SERVER_LOST_FUNCTION__( &KFWorldModule::OnServerLostGame );
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __REGISTER_MESSAGE__( KFMsg::S2S_LOGIN_WORLD_VERIFY_REQ, &KFWorldModule::HandleLoginWorldVerifyReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_GAME_SYNC_ONLINE_REQ, &KFWorldModule::HandleGameSyncOnlineReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_TRANSMIT_MESSAGE_REQ, &KFWorldModule::HandleTransmitMessageReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_BROADCAST_MESSAGE_REQ, &KFWorldModule::HandleBroadcastMessageReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_PLAYER_ENTER_WORLD_REQ, &KFWorldModule::HandlePlayerEnterWorldReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_PLAYER_LEAVE_WORLD_REQ, &KFWorldModule::HandlePlayerLeaveWorldReq );
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __REGISTER_HTTP_FUNCTION__( __KF_STRING__( kickonline ), true, &KFWorldModule::HandleHttpKickOnline );
    }

    void KFWorldModule::BeforeShut()
    {
        __UNREGISTER_SERVER_DISCOVER_FUNCTION__();
        __UNREGISTER_SERVER_LOST_FUNCTION__();
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __UNREGISTER_MESSAGE__( KFMsg::S2S_LOGIN_GAME_UPDATE_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_GAME_SYNC_ONLINE_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_TRANSMIT_MESSAGE_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_BROADCAST_MESSAGE_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_PLAYER_ENTER_WORLD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_PLAYER_LEAVE_WORLD_REQ );
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __UNREGISTER_HTTP_FUNCTION__( __KF_STRING__( kickonline ) );
    }

    KFOnline* KFWorldModule::CreateOnline( uint32 playerid )
    {
        auto kfonline = _kf_online_list.Create( playerid );
        kfonline->_player_id = playerid;
        return kfonline;
    }

    KFOnline* KFWorldModule::FindOnline( uint32 playerid )
    {
        return _kf_online_list.Find( playerid );
    }

    bool KFWorldModule::RemoveOnline( uint32 playerid )
    {
        return _kf_online_list.Remove( playerid );
    }

    uint32 KFWorldModule::GetOnlineCount()
    {
        return _kf_online_list.Size();
    }

    bool KFWorldModule::SendMessageToOnline( uint32 playerid, uint32 msgid, ::google::protobuf::Message* message )
    {
        auto kfonline = FindOnline( playerid );
        if ( kfonline == nullptr )
        {
            return false;
        }

        kfonline->SendMessageToOnline( msgid, message );
        return true;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////
    __KF_SERVER_DISCOVER_FUNCTION__( KFWorldModule::OnServerDisCoverGame )
    {
        if ( handletype == __KF_STRING__( game ) )
        {
            _kf_game_conhash.AddHashNode( __KF_STRING__( game ), handleid, 100 );
        }
    }

    __KF_SERVER_LOST_FUNCTION__( KFWorldModule::OnServerLostGame )
    {
        if ( handletype == __KF_STRING__( game ) )
        {
            _kf_game_conhash.RemoveHashNode( handleid );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFWorldModule::HandleLoginWorldVerifyReq )
    {
        __PROTO_PARSE__( KFMsg::S2SLoginWorldVerifyReq );
        auto loginid = __KF_HEAD_ID__( kfguid );
        auto& token = kfmsg.token();
        auto gateid = kfmsg.gateid();
        auto playerid = kfmsg.playerid();
        auto accountid = kfmsg.accountid();
        auto sessionid = kfmsg.sessionid();

        __LOG_DEBUG__( KFLogEnum::Login, "accountid[{}] player[{}] verify req!", accountid, playerid );

        // 踢掉已经在线的玩家, 只有踢下线以后才能登陆
        if ( KickOnline( playerid, __FUNC_LINE__ ) )
        {
            return SendVerifyFailedToLogin( KFMsg::LoginAlreadyOnline, loginid, gateid, accountid, sessionid );
        }

        // 选择Game服务器
        auto gameid = _kf_game_conhash.FindHashNode( playerid );
        if ( gameid == _invalid_int )
        {
            return SendVerifyFailedToLogin( KFMsg::CanNotFindGame, loginid, gateid, accountid, sessionid );
        }

        // 通知游戏服务器 保存验证信息
        KFMsg::S2SLoginTellTokenToGameReq req;
        req.set_gateid( gateid );
        req.set_token( kfmsg.token() );
        req.set_channel( kfmsg.channel() );
        req.set_accountid( accountid );
        req.set_sessionid( sessionid );
        req.set_playerid( kfmsg.playerid() );
        req.mutable_channeldata()->CopyFrom( kfmsg.channeldata() );
        auto ok = _kf_tcp_server->SendNetMessage( gameid, KFMsg::S2S_LOGIN_TELL_TOKEN_TO_GAME_REQ, &req );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Login, "player[{}:{}] login to game ok!", kfmsg.accountid(), kfmsg.playerid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Login, "player[{}:{}] login to game failed!", kfmsg.accountid(), kfmsg.playerid() );
        }
    }

    void KFWorldModule::SendVerifyFailedToLogin( uint32 result, uint32 loginid, uint32 gateid, uint32 accountid, uint32 sessionid )
    {
        KFMsg::S2SLoginFailedToLoginAck ack;
        ack.set_result( result );
        ack.set_gateid( gateid );
        ack.set_accountid( accountid );
        ack.set_sessionid( sessionid );
        auto ok = _kf_tcp_server->SendNetMessage( loginid, KFMsg::S2S_LOGIN_FAILED_TO_LOGIN_ACK, &ack );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Login, "player[{}] world verify result[{}] ok!", accountid, result );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Login, "aplayer[{}] world verify result[{}] failed!", accountid, result );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFWorldModule::HandleGameSyncOnlineReq )
    {
        __PROTO_PARSE__( KFMsg::S2SGameSyncOnlineReq );

        auto serverid = __KF_HEAD_ID__( kfguid );
        for ( auto i = 0; i < kfmsg.playerid_size(); ++i )
        {
            auto playerid = kfmsg.playerid( i );

            auto kfonline = CreateOnline( playerid );
            kfonline->_game_id = serverid;
        }
    }

    __KF_MESSAGE_FUNCTION__( KFWorldModule::HandleTransmitMessageReq )
    {
        __PROTO_PARSE__( KFMsg::S2STransmitMessageReq );

        auto kfonline = FindOnline( kfmsg.playerid() );
        if ( kfonline == nullptr )
        {
            return;
        }

        auto& msgdata = kfmsg.msgdata();
        kfonline->SendMessageToOnline( kfmsg.msgid(), msgdata.data(), static_cast< uint32 >( msgdata.size() ) );
    }

    __KF_MESSAGE_FUNCTION__( KFWorldModule::HandleBroadcastMessageReq )
    {
        __PROTO_PARSE__( KFMsg::S2SBroadcastMessageReq );

        _kf_tcp_server->SendMessageToType( __KF_STRING__( game ), KFMsg::S2S_BROADCAST_MESSAGE_REQ, &kfmsg );
    }

    __KF_MESSAGE_FUNCTION__( KFWorldModule::HandlePlayerEnterWorldReq )
    {
        __PROTO_PARSE__( KFMsg::S2SPlayerEnterWorldReq );

        auto kfonline = CreateOnline( kfmsg.playerid() );
        kfonline->_player_id = kfmsg.playerid();
        kfonline->_game_id = __KF_HEAD_ID__( kfguid );
        kfonline->_account_id = kfmsg.accountid();

        // 更新到认证服务器
        UpdateOnlineToAuth( kfmsg.accountid(), kfmsg.playerid(), true );
    }

    __KF_MESSAGE_FUNCTION__( KFWorldModule::HandlePlayerLeaveWorldReq )
    {
        __PROTO_PARSE__( KFMsg::S2SPlayerLeaveWorldReq );

        RemoveOnline( kfmsg.playerid() );

        // 更新到认证服务器
        UpdateOnlineToAuth( kfmsg.accountid(), kfmsg.playerid(), false );
    }

    void KFWorldModule::UpdateOnlineToAuth( uint32 accountid, uint32 playerid, bool online )
    {
        static auto _url = _kf_ip_address->FindAuthAddress() + __KF_STRING__( onlinezone );
        static auto _world_url = _kf_http_server->GetHttpUrl();

        // 在线服务器
        KFJson sendjson;
        sendjson.SetValue( __KF_STRING__( zoneid ), _kf_zone->GetZone()->_id );
        sendjson.SetValue( __KF_STRING__( playerid ), playerid );
        sendjson.SetValue( __KF_STRING__( accountid ), accountid );
        if ( online )
        {
            sendjson.SetValue( __KF_STRING__( online ), 1 );
            sendjson.SetValue( __KF_STRING__( zonehttp ), _world_url );
        }
        else
        {
            sendjson.SetValue( __KF_STRING__( online ), 0 );
        }

        _kf_http_client->StartMTHttpClient( _url, sendjson, false );
    }


    bool KFWorldModule::KickOnline( uint32 playerid, const char* function, uint32 line )
    {
        auto kfonline = FindOnline( playerid );
        if ( kfonline == nullptr )
        {
            return false;
        }

        __LOG_DEBUG__( KFLogEnum::Logic, "kick player[{}]!", playerid );

        // 发送踢出消息
        KFMsg::S2SKickGamePlayerReq req;
        req.set_playerid( playerid );
        req.set_type( KFMsg::KickEnum::LoginBeKick );
        auto ok = _kf_tcp_server->SendNetMessage( kfonline->_game_id, KFMsg::S2S_KICK_GAME_PLAYER_REQ, &req );
        if ( ok )
        {
            // 发送成功, 先删除在线信息, 避免gameserver掉线以后,登录不正常的问题
            RemoveOnline( playerid );
        }
        return ok;
    }

    bool KFWorldModule::SendMessageToGame( uint32 gameid, uint32 msgid, ::google::protobuf::Message* message )
    {
        return _kf_tcp_server->SendNetMessage( gameid, msgid, message );
    }

    __KF_HTTP_FUNCTION__( KFWorldModule::HandleHttpKickOnline )
    {
        KFJson kfjson( data );

        auto playerid = kfjson.GetUInt32( __KF_STRING__( playerid ) );
        KickOnline( playerid, __FUNC_LINE__ );
        return _invalid_str;
    }

}
