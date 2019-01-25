﻿#ifndef __KF_GAME_MODULE_H__
#define __KF_GAME_MODULE_H__

/************************************************************************
//    @Module			:    游戏功能模块
//    @Author           :    __凌_痕__
//    @QQ				:    7969936
//    @Mail			    :    lori227@qq.com
//    @Date             :    2017-11-1
************************************************************************/

#include "KFrame.h"
#include "KFGameInterface.h"
#include "KFKernel/KFKernelInterface.h"
#include "KFPlayer/KFPlayerInterface.h"
#include "KFMessage/KFMessageInterface.h"
#include "KFTcpServer/KFTcpServerInterface.h"
#include "KFTcpClient/KFTcpClientInterface.h"
#include "KFRouteClient/KFRouteClientInterface.h"
#include "KFDeployCommand/KFDeployCommandInterface.h"

namespace KFrame
{
    class KFGameModule : public KFGameInterface
    {
    public:
        KFGameModule() = default;
        ~KFGameModule() = default;

        // 刷新
        virtual void BeforeRun();

        // 关闭
        virtual void BeforeShut();
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // 发送消息到世界服务器
        virtual bool SendToWorld( uint32 msgid, ::google::protobuf::Message* message );

        // 发送消息到Gate服务器
        virtual void SendToGate( uint32 msgid, ::google::protobuf::Message* message );
        virtual bool SendToGate( uint64 gateid, uint32 msgid, ::google::protobuf::Message* message );

        // 发送消息到客户端
        virtual bool SendToClient( uint64 gateid, uint64 playerid, uint32 msgid, ::google::protobuf::Message* message );
        virtual bool SendToClient( KFEntity* player, uint32 msgid, ::google::protobuf::Message* message );

        // 发送到玩家
        virtual bool SendToPlayer( uint64 sendid, KFData* kfbasic, uint32 msgid, ::google::protobuf::Message* message );
        virtual bool SendToPlayer( uint64 sendid, uint64 serverid, uint64 playerid, uint32 msgid, ::google::protobuf::Message* message );

        // 广播消息到客户端
        virtual bool BroadcastToGate( uint32 msgid, ::google::protobuf::Message* message );
        virtual bool BroadcastToWorld( uint32 msgid, ::google::protobuf::Message* message );

        // 消息转发
        virtual bool TransmitToPlayer( uint64 playerid, uint32 msgid, ::google::protobuf::Message* message );

        // 转发服务器
        virtual bool TransmitToServer( uint32 msgid, ::google::protobuf::Message* message );
        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // 踢掉角色
        virtual void KickPlayer( uint64 playerid, uint32 type, const char* function, uint32 line );

    protected:
        // 处理消息广播
        __KF_MESSAGE_FUNCTION__( HandleBroadcastMessageReq );

        // 登录token
        __KF_MESSAGE_FUNCTION__( HandleLoginTellTokenToGameReq );

        // 处理登出游戏
        __KF_MESSAGE_FUNCTION__( HandleLoginOutReq );

        // 处理玩家掉线
        __KF_MESSAGE_FUNCTION__( HandlePlayerDisconnectionReq );

        // 处理踢人消息
        __KF_MESSAGE_FUNCTION__( HandleKickGamePlayerReq );

    protected:
        // 连接成功
        __KF_CLIENT_CONNECT_FUNCTION__( OnClientConnectionServer );

        // 断开连接
        __KF_CLIENT_LOST_FUNCTION__( OnClientLostServer );

        // 转发消息到玩家
        __KF_TRANSMIT_MESSAGE_FUNCTION__( TransmitMessageToPlayer );

        // 部署服务器关闭
        __KF_COMMAND_FUNCTION__( OnDeployShutDownServer );

    protected:
        // 进入游戏世界
        void OnEnterGame( KFEntity* player );

        // 离开游戏世界
        void OnLeaveGame( KFEntity* player );

        // 加载玩家数据
        void OnAfterLoadPlayerData( uint32 result, const KFMsg::PBLoginData* pblogin, KFMsg::PBObject* pbplayerdata );

    private:
        // 世界服务器id
        uint64 _world_server_id{ _invalid_int };
    };
}

#endif