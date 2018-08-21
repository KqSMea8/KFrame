﻿#include "KFMatchShardModule.h"

namespace KFrame
{
    KFMatchShardModule::KFMatchShardModule()
    {
    }

    KFMatchShardModule::~KFMatchShardModule()
    {
    }

    void KFMatchShardModule::InitModule()
    {
        __KF_ADD_CONFIG__( _kf_match_config, false );
    }

    void KFMatchShardModule::BeforeRun()
    {
        __REGISTER_CLIENT_CONNECTION_FUNCTION__( &KFMatchShardModule::OnClientConnectMatchMaster );

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __REGISTER_MESSAGE__( KFMsg::S2S_MATCH_TO_SHARD_REQ, &KFMatchShardModule::HandleMatchToShardReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_CANCEL_MATCH_TO_SHARD_REQ, &KFMatchShardModule::HandleCancelMatchToShardReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_CREATE_ROOM_TO_MATCH_SHARD_ACK, &KFMatchShardModule::HandleCreateRoomToMatchShardAck );
        __REGISTER_MESSAGE__( KFMsg::S2S_TELL_ROOM_START_TO_MATCH_SHARD_REQ, &KFMatchShardModule::HandleTellRoomStartToMatchShardReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_ADD_CAMP_TO_MATCH_SHARD_ACK, &KFMatchShardModule::HandleAddCampToMatchShardAck );
        __REGISTER_MESSAGE__( KFMsg::S2S_PLAYER_LEAVE_ROOM_TO_MATCH_SHARD_REQ, &KFMatchShardModule::HandlePlayerLeaveRoomToMatchShardReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_QUERY_ROOM_TO_MATCH_SHARD_REQ, &KFMatchShardModule::HandleQueryRoomToMatchShardReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_OPEN_ROOM_TO_MATCH_SHARD_REQ, &KFMatchShardModule::HandleOpenRoomToMatchShardReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_CANCEL_MATCH_TO_MATCH_SHARD_ACK, &KFMatchShardModule::HandleCancelMatchToMatchShardAck );
        __REGISTER_MESSAGE__( KFMsg::S2S_TELL_ROOM_CLOSE_TO_MATCH_SHARD_REQ, &KFMatchShardModule::HandleTellRoomCloseToMatchShardReq );
    }

    void KFMatchShardModule::BeforeShut()
    {
        __KF_REMOVE_CONFIG__();
        __UNREGISTER_SERVER_DISCOVER_FUNCTION__();
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __UNREGISTER_MESSAGE__( KFMsg::S2S_MATCH_TO_SHARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_CANCEL_MATCH_TO_SHARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_CREATE_ROOM_TO_MATCH_SHARD_ACK );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_TELL_ROOM_START_TO_MATCH_SHARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_ADD_CAMP_TO_MATCH_SHARD_ACK );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_PLAYER_LEAVE_ROOM_TO_MATCH_SHARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_QUERY_ROOM_TO_MATCH_SHARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_OPEN_ROOM_TO_MATCH_SHARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_CANCEL_MATCH_TO_MATCH_SHARD_ACK );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_TELL_ROOM_CLOSE_TO_MATCH_SHARD_REQ );
    }

    void KFMatchShardModule::Run()
    {
        for ( auto& iter : _kf_match_queue._objects )
        {
            auto kfmatchqueue = iter.second;

            kfmatchqueue->RunMatch();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    __KF_CLIENT_CONNECT_FUNCTION__( KFMatchShardModule::OnClientConnectMatchMaster )
    {
        if ( servername == KFGlobal::Instance()->_app_name && servertype == __KF_STRING__( master ) )
        {
            std::list< uint64 > objectlist;

            for ( auto& iter : _kf_match_config->_kf_match_setting._objects )
            {
                auto kfsetting = iter.second;
                objectlist.push_back( kfsetting->_match_id );
            }

            _kf_cluster_shard->AllocObjectToMaster( objectlist );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleMatchToShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2SMatchToShardReq );

        auto kfsetting = _kf_match_config->FindMatchSetting( kfmsg.matchid() );
        if ( kfsetting == nullptr )
        {
            __LOG_ERROR__( KFLogEnum::System, "can't find matchsetting[{}]!", kfmsg.matchid() );
            return;
        }

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            kfmatchqueue = _kf_match_queue.Create( kfmsg.matchid() );
            kfmatchqueue->_match_id = kfmsg.matchid();
            kfmatchqueue->_kf_setting = kfsetting;
            kfmatchqueue->_kf_match_module = this;
        }

        auto pbgroup = &kfmsg.pbgroup();
        kfmatchqueue->StartMatch( pbgroup, kfmsg.allowgroup() );

        KFMsg::S2SMatchToClientAck ack;
        ack.set_matchid( kfmsg.matchid() );
        ack.set_playerid( kfmsg.playerid() );
        ack.set_result( KFMsg::MatchRequestSuccess );
        _kf_cluster_shard->SendMessageToClient( kfmsg.serverid(), KFMsg::S2S_MATCH_TO_CLIENT_ACK, &ack );

        __LOG_DEBUG__( KFLogEnum::Logic, "group[{}] match allowgroup[{}]!", pbgroup->groupid(), kfmsg.allowgroup() ? 1 : 0 );
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleQueryRoomToMatchShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2SQueryRoomToMatchShardReq );
        __LOG_DEBUG__( KFLogEnum::Logic, "player[{}] query match[{}] req!", kfmsg.playerid(), kfmsg.matchid() );

        auto proxyid = __KF_HEAD_ID__( kfguid );

        auto matchid = _invalid_int;
        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue != nullptr )
        {
            if ( kfmatchqueue->QueryBattleRoom( kfmsg.playerid(), kfmsg.serverid() ) )
            {
                matchid = kfmsg.matchid();
            }
        }

        // 查询匹配
        KFMsg::S2SQueryMatchRoomAck ack;
        ack.set_matchid( matchid );
        ack.set_playerid( kfmsg.playerid() );
        auto ok = _kf_cluster_shard->SendMessageToClient( proxyid, kfmsg.serverid(), KFMsg::S2S_QUERY_MATCH_ROOM_ACK, &ack );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Logic, "player[{}] query match[{}] ok!", kfmsg.playerid(), kfmsg.matchid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Logic, "player[{}] query match[{}] failed!", kfmsg.playerid(), kfmsg.matchid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleCancelMatchToShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2SCancelMatchToShardReq );

        __LOG_DEBUG__( KFLogEnum::Logic, "player[{}] cancel match[{}] req!", kfmsg.playerid(), kfmsg.matchid() );
        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->CancelMatchReq( kfmsg.playerid() );
        if ( !ok )
        {
            __LOG_ERROR__( KFLogEnum::Logic, "player[{}] cancel match[{}] failed!", kfmsg.playerid(), kfmsg.matchid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleCancelMatchToMatchShardAck )
    {
        __PROTO_PARSE__( KFMsg::S2SCancelMatchToMatchShardAck );
        __LOG_DEBUG__( KFLogEnum::Logic, "room[{}:{}] player[{}:{}] cancel ack!", kfmsg.matchid(), kfmsg.roomid(), kfmsg.campid(), kfmsg.playerid() );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->CancelMatchAck( kfmsg.roomid(), kfmsg.campid(), kfmsg.playerid(), kfmsg.isroomopen() );
        if ( !ok )
        {
            __LOG_ERROR__( KFLogEnum::Logic, "room[{}:{}] player[{}:{}] cancel failed!", kfmsg.matchid(), kfmsg.roomid(), kfmsg.campid(), kfmsg.playerid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleCreateRoomToMatchShardAck )
    {
        __PROTO_PARSE__( KFMsg::S2SCreateRoomToMatchShardAck );
        __LOG_DEBUG__( KFLogEnum::Logic, "create room[{}:{}] ack!", kfmsg.matchid(), kfmsg.roomid() );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto result = kfmatchqueue->CreateBattleRoom( kfmsg.roomid(), kfmsg.battleshardid() );
        if ( !result )
        {
            __LOG_ERROR__( KFLogEnum::Logic, "create battle room[{}:{}] failed!", kfmsg.matchid(), kfmsg.roomid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleAddCampToMatchShardAck )
    {
        __PROTO_PARSE__( KFMsg::S2SAddCampToMatchShardAck );

        __LOG_DEBUG__( KFLogEnum::Logic, "room[{}:{}] add camp[{}] result[{}] ack!",
                       kfmsg.matchid(), kfmsg.roomid(), kfmsg.campid(), ( kfmsg.addok() ? 1 : 0 ) );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->EnterBattleRoom( kfmsg.roomid(), kfmsg.campid(), kfmsg.addok() );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Logic, "room[{}:{}] add camp[{}] ok!", kfmsg.matchid(), kfmsg.roomid(), kfmsg.campid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Logic, "room[{}:{}] add camp[{}] failed!", kfmsg.matchid(), kfmsg.roomid(), kfmsg.campid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleOpenRoomToMatchShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2SOpenRoomToMatchShardReq );
        __LOG_DEBUG__( KFLogEnum::Logic, "room[{}:{}] open req!", kfmsg.matchid(), kfmsg.roomid() );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "room[{}] can't find match queue[{}]!", kfmsg.roomid(), kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->OpenBattleRoom( kfmsg.roomid(), kfmsg.waittime() );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Logic, "room[{}:{}] open ok!", kfmsg.matchid(), kfmsg.roomid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Logic, "room[{}:{}] open failed!", kfmsg.matchid(), kfmsg.roomid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandlePlayerLeaveRoomToMatchShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2SPlayerLeaveRoomToMatchShardReq );
        __LOG_DEBUG__( KFLogEnum::Logic, "[{}:{}] leave room[{}:{}] req!", kfmsg.campid(), kfmsg.playerid(), kfmsg.matchid(), kfmsg.roomid() );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->LeaveBattleRoom( kfmsg.roomid(), kfmsg.campid(), kfmsg.playerid() );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Logic, "[{}:{}] leave room[{}:{}] ok!", kfmsg.campid(), kfmsg.playerid(), kfmsg.matchid(), kfmsg.roomid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Logic, "[{}:{}] leave room[{}:{}] failed!", kfmsg.campid(), kfmsg.playerid(), kfmsg.matchid(), kfmsg.roomid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleTellRoomStartToMatchShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2STellRoomStartToMatchShardReq );
        __LOG_DEBUG__( KFLogEnum::Logic, "start room[{}:{}] req!", kfmsg.matchid(), kfmsg.roomid() );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->StartBattleRoom( kfmsg.roomid() );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Logic, "start room[{}:{}] ok!", kfmsg.matchid(), kfmsg.roomid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Logic, "start room[{}:{}] failed!", kfmsg.matchid(), kfmsg.roomid() );
        }
    }

    __KF_MESSAGE_FUNCTION__( KFMatchShardModule::HandleTellRoomCloseToMatchShardReq )
    {
        __PROTO_PARSE__( KFMsg::S2STellRoomCloseToMatchShardReq );
        __LOG_DEBUG__( KFLogEnum::Logic, "close room[{}:{}] req!", kfmsg.matchid(), kfmsg.roomid() );

        auto kfmatchqueue = _kf_match_queue.Find( kfmsg.matchid() );
        if ( kfmatchqueue == nullptr )
        {
            return __LOG_ERROR__( KFLogEnum::Logic, "can't find match queue[{}]!", kfmsg.matchid() );
        }

        auto ok = kfmatchqueue->CloseBattleRoom( kfmsg.roomid() );
        if ( ok )
        {
            __LOG_DEBUG__( KFLogEnum::Logic, "close room[{}:{}] ok!", kfmsg.matchid(), kfmsg.roomid() );
        }
        else
        {
            __LOG_ERROR__( KFLogEnum::Logic, "start room[{}:{}] failed!", kfmsg.matchid(), kfmsg.roomid() );
        }
    }

}