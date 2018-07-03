﻿#include "KFMailClientModule.h"
#include "KFMailConfig.h"
#include "KFUtility/KFUtility.h"

namespace KFrame
{
    KFMailClientModule::KFMailClientModule()
    {
    }

    KFMailClientModule::~KFMailClientModule()
    {

    }

    void KFMailClientModule::InitModule()
    {
        __KF_ADD_CONFIG__( _kf_mail_config, true );
    }

    void KFMailClientModule::BeforeRun()
    {

        _kf_player->RegisterEnterFunction( this, &KFMailClientModule::OnEnterQueryMail );
        _kf_player->RegisterLeaveFunction( this, &KFMailClientModule::OnLeaveQueryMail );
        _kf_player->RegisterNewPlayerFunction( this, &KFMailClientModule::OnNewPlayerHandleGMMail );
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __REGISTER_MESSAGE__( KFMsg::MSG_QUERY_MAIL_REQ, &KFMailClientModule::HandleQueryMailReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_QUERY_MAIL_ACK, &KFMailClientModule::HandleQueryMailAck );
        __REGISTER_MESSAGE__( KFMsg::MSG_VIEW_MAIL_REQ, &KFMailClientModule::HandleViewMailReq );
        __REGISTER_MESSAGE__( KFMsg::MSG_DELETE_MAIL_REQ, &KFMailClientModule::HandleDeleteMailReq );
        __REGISTER_MESSAGE__( KFMsg::MSG_RECEIVE_MAIL_REWARD_REQ, &KFMailClientModule::HandleReceiveMailReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_UPDATE_MAIL_FLAG_ACK, &KFMailClientModule::HandleUpdateMailFlagAck );
        __REGISTER_MESSAGE__( KFMsg::MSG_PLAYER_TOAST_REQ, &KFMailClientModule::HandlePlayerToastReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_PLAYER_TOAST_ACK, &KFMailClientModule::HandlePlayerToastAck );
        __REGISTER_MESSAGE__( KFMsg::S2S_NOTICE_NEW_MAIL_REQ, &KFMailClientModule::HandleNoticeNewMailReq );
    }

    void KFMailClientModule::BeforeShut()
    {
        __KF_REMOVE_CONFIG__();

        _kf_player->UnRegisterEnterFunction( this );
        _kf_player->UnRegisterLeaveFunction( this );
        _kf_player->UnRegisterNewPlayerFunction( this );

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        __UNREGISTER_MESSAGE__( KFMsg::MSG_QUERY_MAIL_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_QUERY_MAIL_ACK );
        __UNREGISTER_MESSAGE__( KFMsg::MSG_VIEW_MAIL_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::MSG_DELETE_MAIL_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::MSG_RECEIVE_MAIL_REWARD_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_UPDATE_MAIL_FLAG_ACK );
        __UNREGISTER_MESSAGE__( KFMsg::MSG_PLAYER_TOAST_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_PLAYER_TOAST_ACK );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_NOTICE_NEW_MAIL_REQ );
    }

    bool KFMailClientModule::SendMessageToMail( uint32 msgid, ::google::protobuf::Message* message )
    {
        return _kf_cluster->SendMessageToShard( KFField::_mail, msgid, message );
    }

    void KFMailClientModule::OnEnterQueryMail( KFEntity* player )
    {
        auto playerid = player->GetKeyID();
        _kf_timer->RegisterLoopTimer( playerid, TimeEnum::OneMinuteMicSecond * 5, this, &KFMailClientModule::OnTimerQueryMail );
    }

    void KFMailClientModule::OnLeaveQueryMail( KFEntity* player )
    {
        auto playerid = player->GetKeyID();
        _kf_timer->UnRegisterTimer( this, playerid );
    }

    void KFMailClientModule::OnNewPlayerHandleGMMail( KFEntity* player )
    {
        KFMsg::S2SNewPlayerLoginMailReq req;
        req.set_playerid( player->GetKeyID() );
        SendMessageToMail( KFMsg::S2S_NEW_PLAYER_LOGIN_MAIL_REQ, &req );
    }

    __KF_TIMER_FUNCTION__( KFMailClientModule::OnTimerQueryMail )
    {
        auto playerid = static_cast< uint32 >( objectid );
        auto player = _kf_player->FindPlayer( playerid, __FUNCTION_LINE__ );
        if ( player == nullptr )
        {
            return;
        }

        // 统一逻辑, 分多次查询不同类型邮件
        for ( uint32 i = KFMsg::MailEnum_MIN; i <= KFMsg::MailEnum_MAX; ++i )
        {
            // 获得该类型最大的邮件id
            auto maxmailid = FindMaxMailId( player, i );
            SendQueryMailMessage( playerid, i, maxmailid );
        }
    }

    void KFMailClientModule::SendQueryMailMessage( uint32 playerid, uint32 mailtype, uint64 maxmailid )
    {
        KFMsg::S2SQueryMailReq req;
        req.set_playerid( playerid );
        req.set_mailtype( mailtype );
        req.set_maxmailid( maxmailid );
        SendMessageToMail( KFMsg::S2S_QUERY_MAIL_REQ, &req );
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleQueryMailReq )
    {
        __CLIENT_PROTO_PARSE__( KFMsg::MsgQueryMailReq );
        if ( KFMsg::MailEnum_MIN > kfmsg.mailtype() || KFMsg::MailEnum_MAX < kfmsg.mailtype() )
        {
            return;
        }
        auto maxmailid = FindMaxMailId( player, kfmsg.mailtype() );
        SendQueryMailMessage( playerid, kfmsg.mailtype(), maxmailid );
    }

    uint64 KFMailClientModule::FindMaxMailId( KFEntity* player, uint32 mailtype )
    {
        auto kfobject = player->GetData();
        auto kfmailrecord = kfobject->FindData( KFField::_mail );

        auto maxmailid = _invalid_int;

        auto kfmail = kfmailrecord->FirstData();
        while ( kfmail != nullptr )
        {
            auto type = kfmail->GetValue< uint32 >( KFField::_type );
            if ( type == mailtype )
            {
                auto mailid = kfmail->GetValue< uint64 >( KFField::_id );
                if ( mailid > maxmailid )
                {
                    maxmailid = mailid;
                }
            }

            kfmail = kfmailrecord->NextData();
        }

        return maxmailid;
    }

    uint64 KFMailClientModule::FindMinMailId( KFEntity* player, uint32 mailtype )
    {
        auto kfobject = player->GetData();
        auto kfmailrecord = kfobject->FindData( KFField::_mail );

        auto minmailid = std::numeric_limits<uint64>::max();;

        auto kfmail = kfmailrecord->FirstData();
        while ( kfmail != nullptr )
        {
            auto type = kfmail->GetValue< uint32 >( KFField::_type );
            if ( type == mailtype )
            {
                auto mailid = kfmail->GetValue< uint64 >( KFField::_id );
                if ( mailid < minmailid )
                {
                    minmailid = mailid;
                }
            }

            kfmail = kfmailrecord->NextData();
        }

        return minmailid;
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleQueryMailAck )
    {
        __SERVER_PROTO_PARSE__( KFMsg::S2SQueryMailAck );

        auto kfobject = player->GetData();
        auto kfmailrecord = kfobject->FindData( KFField::_mail );

        // 将邮件保存到玩家属性中
        auto pbmails = &kfmsg.mails();
        for ( auto i = 0; i < pbmails->mail_size(); ++i )
        {
            auto pbmail = &pbmails->mail( i );
            auto kfmail = ParsePBMailToData( pbmail, kfmailrecord->GetDataSetting() );
            if ( kfmail == nullptr )
            {
                continue;
            }

            player->AddData( kfmailrecord, kfmail );
        }
    }

    KFData* KFMailClientModule::ParsePBMailToData( const KFMsg::PBMail* pbmail, const KFDataSetting* kfsetting )
    {
        auto kfmail = _kf_kernel->CreateObject( kfsetting );
        if ( kfmail == nullptr )
        {
            return nullptr;
        }

        // 设置邮件属性
        for ( auto i = 0; i < pbmail->data_size(); ++i )
        {
            auto pbdata = &pbmail->data( i );
            kfmail->SetValue< std::string >( pbdata->name(), pbdata->value() );
        }

        return kfmail;
    }

    bool KFMailClientModule::CheckMailTimeOut( KFData* kfmail )
    {
        auto validtime = kfmail->GetValue<uint32>( KFField::_valid_time );
        auto sendtime = kfmail->GetValue<uint64>( KFField::_send_time );

        return sendtime + validtime < KFGlobal::Instance()->_real_time;
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleViewMailReq )
    {
        __CLIENT_PROTO_PARSE__( KFMsg::MsgViewMailReq );

        auto kfobject = player->GetData();
        auto kfmail = kfobject->FindData( KFField::_mail, kfmsg.mailid() );
        if ( kfmail == nullptr )
        {
            return _kf_display->SendToClient( player, KFMsg::MailNotExist );
        }

        if ( CheckMailTimeOut( kfmail ) )
        {
            return _kf_display->SendToClient( player, KFMsg::MailTimeOut );
        }

        auto flag = kfmail->GetValue< uint32>( KFField::_flag );
        if ( flag != KFMsg::FlagEnum::Init )
        {
            return;
        }

        // 更新状态
        UpdateFlagToMail( playerid, kfmail, KFMsg::FlagEnum::Done );
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleDeleteMailReq )
    {
        __CLIENT_PROTO_PARSE__( KFMsg::MsgDeleteMailReq );

        auto kfobject = player->GetData();
        auto kfmail = kfobject->FindData( KFField::_mail, kfmsg.mailid() );
        if ( kfmail == nullptr )
        {
            return _kf_display->SendToClient( player, KFMsg::MailNotExist );
        }

        // 如果有奖励, 并且没有领取
        auto flag = kfmail->GetValue< uint32 >( KFField::_flag );
        auto reward = kfmail->GetValue< std::string >( KFField::_reward );
        if ( flag != KFMsg::FlagEnum::Received && !reward.empty() )
        {
            return _kf_display->SendToClient( player, KFMsg::MailDeleteFailed );
        }

        // 更新到邮件集群
        UpdateFlagToMail( playerid, kfmail, KFMsg::FlagEnum::Remove );
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleReceiveMailReq )
    {
        __CLIENT_PROTO_PARSE__( KFMsg::MsgReceiveMailRewardReq );

        auto kfobject = player->GetData();
        auto kfmail = kfobject->FindData( KFField::_mail, kfmsg.mailid() );
        if ( kfmail == nullptr )
        {
            return _kf_display->SendToClient( player, KFMsg::MailNotExist );
        }

        auto flag = kfmail->GetValue< uint32 >( KFField::_flag );
        if ( flag == KFMsg::FlagEnum::Received )
        {
            return _kf_display->SendToClient( player, KFMsg::MailAlreadyReceived );
        }

        auto reward = kfmail->GetValue< std::string >( KFField::_reward );
        if ( reward.empty() )
        {
            return _kf_display->SendToClient( player, KFMsg::MailNotHaveReward );
        }

        if ( CheckMailTimeOut( kfmail ) )
        {
            return _kf_display->SendToClient( player, KFMsg::MailTimeOut );
        }

        auto mailtype = kfmail->GetValue< uint32 >( KFField::_type );

        auto deltype = _kf_option->GetValue<uint32>( "delNow", mailtype );
        if ( deltype == 1 )
        {
            UpdateFlagToMail( playerid, kfmail, KFMsg::FlagEnum::ReceiveRemove );
        }
        else
        {
            UpdateFlagToMail( playerid, kfmail, KFMsg::FlagEnum::Received );
        }
    }

    bool KFMailClientModule::UpdateFlagToMail( uint32 playerid, KFData* kfmail, uint32 flag )
    {
        auto mailid = kfmail->GetKeyID();
        auto mailtype = kfmail->GetValue< uint32 >( KFField::_type );

        KFMsg::S2SUpdateMailFlagReq req;
        req.set_playerid( playerid );
        req.set_mailid( mailid );
        req.set_mailtype( mailtype );
        req.set_flag( flag );
        auto ok = SendMessageToMail( KFMsg::S2S_UPDATE_MAIL_FLAG_REQ, &req );
        if ( !ok )
        {
            auto strmailid = __KF_STRING__( mailid );
            KFLogger::LogLogic( KFLogger::Error, "[%s] player[%u] update mail[%u:%s] flag[%u] failed!",
                                __FUNCTION__, playerid, mailtype, strmailid.c_str(), flag );
        }

        return ok;
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleUpdateMailFlagAck )
    {
        __SERVER_PROTO_PARSE__( KFMsg::S2SUpdateMailFlagAck );

        switch ( kfmsg.flag() )
        {
        case KFMsg::FlagEnum::Done:
        {
            player->UpdateData( KFField::_mail, kfmsg.mailid(), KFField::_flag, KFOperateEnum::Set, KFMsg::FlagEnum::Done );
        }
        break;
        case KFMsg::FlagEnum::Remove:
        {
            player->RemoveData( KFField::_mail, kfmsg.mailid() );
        }
        break;
        case KFMsg::FlagEnum::Received:
        {
            ReceiveMailReward( player, kfmsg.mailid() );
        }
        break;
        case KFMsg::FlagEnum::ReceiveRemove:
        {
            ReceiveMailReward( player, kfmsg.mailid() );
            player->RemoveData( KFField::_mail, kfmsg.mailid() );
        }
        break;
        default:
            break;
        }
    }

    void KFMailClientModule::ReceiveMailReward( KFEntity* player, uint64 mailid )
    {
        auto strmailid = __KF_STRING__( mailid );

        auto kfobject = player->GetData();
        auto kfmail = kfobject->FindData( KFField::_mail, mailid );
        if ( kfmail == nullptr )
        {
            return KFLogger::LogLogic( KFLogger::Error, "[%s:%u] player[%s] can't find mail[%s]!",
                                       __FUNCTION_LINE__, player->GetKeyString(), strmailid.c_str() );
        }

        auto reward = kfmail->GetValue< std::string >( KFField::_reward );
        if ( reward.empty() )
        {
            return KFLogger::LogLogic( KFLogger::Error, "[%s:%u] player[%s] mail[%s] no reward!",
                                       __FUNCTION_LINE__, player->GetKeyString(), strmailid.c_str() );
        }

        KFAgents kfagents;
        auto ok = kfagents.ParseFromString( reward, __FUNCTION_LINE__ );
        if ( !ok )
        {
            return KFLogger::LogLogic( KFLogger::Error, "[%s:%u] player[%s] mail[%s] reward[%s] error!",
                                       __FUNCTION_LINE__, player->GetKeyString(), strmailid.c_str(), reward.c_str() );
        }

        player->UpdateData( kfmail, KFField::_flag, KFOperateEnum::Set, KFMsg::FlagEnum::Received );
        player->AddAgentData( &kfagents, 1.0f, true, __FUNCTION_LINE__ );

        // 如果有配置回复邮件id, 回复邮件
        auto configid = kfmail->GetValue< uint32 >( KFField::_config_id );
        auto kfsetting = _kf_mail_config->FindMailSetting( configid );
        if ( kfsetting != nullptr && kfsetting->_reply_id != _invalid_int )
        {
            auto senderid = kfmail->GetValue< uint32 >( KFField::_sender_id );
            auto serverid = kfmail->GetValue< uint32 >( KFField::_server_id );
            SendMail( player, KFGuid( serverid, senderid ), kfsetting->_reply_id, nullptr, reward );
        }
    }

    void KFMailClientModule::FormatMailData( const KFMailSetting* kfsetting, MapString& maildata, const KFAgents* kfagents, KFEntity* sender /* = nullptr */, const std::string& extend/* = ""*/ )
    {
        // 有优化的空间, 服务器只发configid到客户端, 客户端根据邮件配置表来获得邮件的基础数据
        // 暂时先发给客户端, 省去客户端读取邮件配置表

        // 配置id
        maildata.insert( std::make_pair( KFField::_config_id, __KF_STRING__( kfsetting->_config_id ) ) );

        // 类型
        maildata.insert( std::make_pair( KFField::_type, __KF_STRING__( kfsetting->_type ) ) );

        // 标识
        maildata.insert( std::make_pair( KFField::_flag, "0" ) );

        // 标题
        maildata.insert( std::make_pair( KFField::_title, kfsetting->_title ) );

        // 内容
        maildata.insert( std::make_pair( KFField::_content, kfsetting->_content ) );

        // 有效时间
        maildata.insert( std::make_pair( KFField::_valid_time, __KF_STRING__( kfsetting->_valid_time ) ) );

        // 附加信息
        if ( !extend.empty() )
        {
            maildata.insert( std::make_pair( KFField::_extend, extend ) );
        }
        else
        {
            maildata.insert( std::make_pair( KFField::_extend, kfsetting->_extend ) );
        }

        // 奖励
        if ( kfagents != nullptr )
        {
            maildata.insert( std::make_pair( KFField::_reward, kfagents->_string ) );
        }

        // 发送者信息
        if ( sender != nullptr )
        {
            auto kfobject = sender->GetData();
            auto kfbasic = kfobject->FindData( KFField::_basic );

            // 发送者名字
            maildata.insert( std::make_pair( KFField::_sender_name, kfbasic->GetValue< std::string >( KFField::_name ) ) );

            // 发送者id
            maildata.insert( std::make_pair( KFField::_sender_id, __KF_STRING__( kfobject->GetKeyID() ) ) );

            // 发送者服务器id
            maildata.insert( std::make_pair( KFField::_server_id, __KF_STRING__( kfbasic->GetValue< uint32 >( KFField::_server_id ) ) ) );

            // 头像
            maildata.insert( std::make_pair( KFField::_icon, kfbasic->GetValue< std::string >( KFField::_icon ) ) );

            // 头像
            maildata.insert( std::make_pair( KFField::_icon_box, kfbasic->GetValue< std::string >( KFField::_icon_box ) ) );

            // 性别
            maildata.insert( std::make_pair( KFField::_sex, __KF_STRING__( kfbasic->GetValue< uint32 >( KFField::_sex ) ) ) );
        }
    }

    bool KFMailClientModule::SendAddMailToCluster( uint32 serverid, uint32 playerid, uint32 mailtype, const MapString& maildata )
    {
        KFMsg::S2SAddMailReq req;
        req.set_mailtype( mailtype );
        req.set_playerid( playerid );

        auto pbmail = req.mutable_pbmail();
        for ( auto& iter : maildata )
        {
            auto pbdata = pbmail->add_data();
            pbdata->set_name( iter.first );
            pbdata->set_value( iter.second );
        }

        auto ok = SendMessageToMail( KFMsg::S2S_ADD_MAIL_REQ, &req );
        if ( ok )
        {
            KFMsg::S2SNoticeNewMailReq notice;
            notice.set_playerid( playerid );
            notice.set_mailtype( mailtype );
            _kf_route->SendMessageToRoute( serverid, playerid, KFMsg::S2S_NOTICE_NEW_MAIL_REQ, &notice );
        }

        return ok;
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandleNoticeNewMailReq )
    {
        __SERVER_PROTO_PARSE__( KFMsg::S2SNoticeNewMailReq );

        auto maxmailid = FindMaxMailId( player, kfmsg.mailtype() );
        SendQueryMailMessage( player->GetKeyID(), kfmsg.mailtype(), maxmailid );

    }

    bool KFMailClientModule::SendMail( KFEntity* receiverplayer, uint32 mailconfigid, const KFAgents* kfagents /* = nullptr */ )
    {
        auto kfsetting = _kf_mail_config->FindMailSetting( mailconfigid );
        if ( kfsetting == nullptr )
        {
            return false;
        }

        MapString maildata;
        FormatMailData( kfsetting, maildata, kfagents );
        return SendAddMailToCluster( KFGlobal::Instance()->_app_id, receiverplayer->GetKeyID(), kfsetting->_type, maildata );
    }

    bool KFMailClientModule::SendMail( KFEntity* senderplayer, const KFGuid& receiverguid, uint32 mailconfigid, const KFAgents* kfagents /* = nullptr */, const std::string& extend/* = ""*/ )
    {
        auto kfsetting = _kf_mail_config->FindMailSetting( mailconfigid );
        if ( kfsetting == nullptr )
        {
            return false;
        }

        MapString maildata;
        FormatMailData( kfsetting, maildata, kfagents, senderplayer, extend );

        auto receiverid = __KF_DATA_ID__( receiverguid );
        auto receiverserverid = __KF_HEAD_ID__( receiverguid );
        return SendAddMailToCluster( receiverserverid, receiverid, kfsetting->_type, maildata );
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandlePlayerToastReq )
    {
        __CLIENT_PROTO_PARSE__( KFMsg::MsgPlayerToastReq );

        auto kfsetting = _kf_mail_config->FindMailSetting( KFMsg::MailConfigEnum::ToastMail );
        if ( kfsetting == nullptr )
        {
            return KFLogger::LogLogic( KFLogger::Error, "%s can't find ToastMail config!", __FUNCTION__ );
        }

        auto kfobject = player->GetData();
        auto toastrecord = kfobject->FindData( KFField::_toast );

        //判断每日敬酒次数
        auto toastcount = toastrecord->Size();
        auto toastlimit = kfsetting->_daily_send_limit;
        if ( toastcount >= toastlimit )
        {
            return _kf_display->SendToClient( player, KFMsg::ToastSendCountOver );
        }

        //判断是否重复敬酒
        auto toastplayer = toastrecord->FindData( kfmsg.playerid() );
        if ( toastplayer != nullptr )
        {
            return _kf_display->SendToClient( player, KFMsg::ToastCanNotRepeat );
        }

        //发送到public判断被敬酒次数
        KFMsg::S2SPlayerToastReq req;
        req.set_selfplayerid( __KF_DATA_ID__( kfguid ) );
        req.set_targetplayerid( kfmsg.playerid() );
        req.set_dailygetlimit( kfsetting->_daily_get_limit );
        _kf_cluster->SendMessageToShard( KFField::_public, KFMsg::S2S_PLAYER_TOAST_REQ, &req );
    }

    __KF_MESSAGE_FUNCTION__( KFMailClientModule::HandlePlayerToastAck )
    {
        __SERVER_PROTO_PARSE__( KFMsg::S2SPlayerToastAck );

        _kf_display->SendToClient( player, kfmsg.result() );

        // 敬酒成功
        if ( kfmsg.result() == KFMsg::ToastOK )
        {
            player->UpdateData( KFField::_toast, kfmsg.targetplayerid(), KFField::_id, KFOperateEnum::Set, kfmsg.targetplayerid() );

            //发送敬酒邮件
            const KFAgents* kfagents = nullptr;
            auto kfsetting = _kf_mail_config->FindMailSetting( KFMsg::MailConfigEnum::ToastMail );
            if ( kfsetting != nullptr )
            {
                kfagents = &kfsetting->_rewards;
            }

            KFGuid targetguid;
            targetguid._head_id = kfmsg.targetserverid();
            targetguid._data_id = kfmsg.targetplayerid();
            SendMail( player, targetguid, KFMsg::MailConfigEnum::ToastMail, kfagents );
        }
    }
}
