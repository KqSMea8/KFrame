#ifndef __KF_CLUSTER_SHARD_INTERFACE_H__
#define __KF_CLUSTER_SHARD_INTERFACE_H__

#include "KFrame.h"

namespace KFrame
{
    class KFClusterShardInterface : public KFModule
    {
    public:
        // 发型消息到代理服务器
        virtual void SendMessageToProxy( uint32 msgid, const char* data, uint32 length ) = 0;
        virtual void SendMessageToProxy( uint32 msgid, google::protobuf::Message* message ) = 0;

        // 发型消息到代理服务器
        virtual bool SendMessageToProxy( uint32 handleid, uint32 msgid, const char* data, uint32 length ) = 0;
        virtual bool SendMessageToProxy( uint32 handleid, uint32 msgid, google::protobuf::Message* message ) = 0;

        // 发型消息到代理服务器
        virtual bool SendMessageToClient( const KFGuid& kfguid, uint32 msgid, const char* data, uint32 length ) = 0;
        virtual bool SendMessageToClient( const KFGuid& kfguid, uint32 msgid, google::protobuf::Message* message ) = 0;

        // 发型消息到代理服务器
        virtual bool SendMessageToClient( uint32 clientid, uint32 msgid, const char* data, uint32 length ) = 0;
        virtual bool SendMessageToClient( uint32 clientid, uint32 msgid, google::protobuf::Message* message ) = 0;

        // 发型消息到代理服务器
        virtual bool SendMessageToClient( uint32 proxyid, uint32 clientid, uint32 msgid, const char* data, uint32 length ) = 0;
        virtual bool SendMessageToClient( uint32 proxyid, uint32 clientid, uint32 msgid, google::protobuf::Message* message ) = 0;

        // 发送消息到玩家
        virtual bool SendMessageToPlayer( uint32 clientid, uint32 playerid, uint32 msgid, google::protobuf::Message* message ) = 0;

        /////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////
        // 注册对象映射
        virtual void AddObjectToProxy( uint64 objectid ) = 0;
        virtual void AddObjectToProxy( uint32 proxyid, const std::list< uint64 >& objectlist ) = 0;

        // 删除对象映射
        virtual void RemoveObjectToProxy( uint64 objectid ) = 0;
        virtual void RemoveObjectToProxy( const std::list< uint64 >& objectlist ) = 0;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    __KF_INTERFACE__( _kf_cluster_shard, KFClusterShardInterface );
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
}



#endif