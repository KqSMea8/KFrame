﻿#include "KFDeployAgentModule.h"
#include "KFDeployAgentConfig.h"
#include "KFProtocol/KFProtocol.h"

#if __KF_SYSTEM__ == __KF_WIN__
    #include <windows.h>
    #include <direct.h>
    #define __MKDIR__( path ) _mkdir( path.c_str() )
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/wait.h>

    #define __MKDIR__( path ) mkdir( path.c_str(), 0755 )
#endif

namespace KFrame
{
    KFDeployAgentModule::KFDeployAgentModule()
    {
        _deploy_server_id = 0;
    }

    KFDeployAgentModule::~KFDeployAgentModule()
    {

    }

    void KFDeployAgentModule::InitModule()
    {
        __KF_ADD_CONFIG__( _kf_deploy_agent_config, true );
    }

    void KFDeployAgentModule::BeforeRun()
    {
        __REGISTER_CLIENT_CONNECTION_FUNCTION__( &KFDeployAgentModule::OnClientConnectServer );
        __REGISTER_LOOP_TIMER__( 0, 10000, &KFDeployAgentModule::OnTimerStartupProcess );
        ////////////////////////////////////////////////////
        __REGISTER_MESSAGE__( KFMsg::S2S_STARTUP_SERVER_TO_AGENT_REQ, &KFDeployAgentModule::HandleStartupServerReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_SHUT_DOWN_SERVER_TO_AGENT_REQ, &KFDeployAgentModule::HandleShutDownServerReq );
        __REGISTER_MESSAGE__( KFMsg::S2S_KILL_SERVER_TO_AGENT_REQ, &KFDeployAgentModule::HandleKillServerReq );
    }

    static std::string _pid_path = "./pid";

    void KFDeployAgentModule::OnceRun()
    {
        __MKDIR__( _pid_path );
        BindServerProcess();
    }

    void KFDeployAgentModule::ShutDown()
    {
        __KF_REMOVE_CONFIG__();
        __UNREGISTER_TIMER__();
        __UNREGISTER_CLIENT_CONNECTION_FUNCTION__();
        //////////////////////////////////////////////////////////
        __UNREGISTER_MESSAGE__( KFMsg::S2S_STARTUP_SERVER_TO_AGENT_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_SHUT_DOWN_SERVER_TO_AGENT_REQ );
        __UNREGISTER_MESSAGE__( KFMsg::S2S_KILL_SERVER_TO_AGENT_REQ );
    }

    /////////////////////////////////////////////////////////////////////////
    __KF_CLIENT_CONNECT_FUNCTION__( KFDeployAgentModule::OnClientConnectServer )
    {
        // 连接成功
        if ( servername == KFField::_deploy && servertype == KFField::_server )
        {
            _deploy_server_id = serverid;

            auto kfglobal = KFGlobal::Instance();

            // 把自己注册到Services
            KFMsg::S2SRegisterAgentToServerReq req;
            req.set_agentid( kfglobal->_app_id );
            req.set_name( kfglobal->_app_name );
            req.set_type( kfglobal->_app_type );
            req.set_port( kfglobal->_listen_port );
            req.set_localip( _kf_ip_address->GetLocalIp() );
            _kf_tcp_client->SendNetMessage( serverid, KFMsg::S2S_REGISTER_AGENT_TO_SERVER_REQ, &req );

            // 更新服务器状态
            for ( auto& iter : _kf_deploy_agent_config->_kf_launch_setting._objects )
            {
                auto kfsetting = iter.second;
                UpdateProcessToServer( kfsetting );
            }
        }
    }

    __KF_TIMER_FUNCTION__( KFDeployAgentModule::OnTimerStartupProcess )
    {
        // 不是自动启动
        if ( !_kf_deploy_agent_config->_is_auto_startup )
        {
            return;
        }

        for ( auto& iter : _kf_deploy_agent_config->_kf_launch_setting._objects )
        {
            auto kfsetting = iter.second;
            if ( !kfsetting->_is_startup )
            {
                continue;
            }

            StartupServerProcess( kfsetting );
        }
    }

    void KFDeployAgentModule::StartupServerProcess( KFLaunchSetting* kfsetting )
    {
        CheckServerProcess( kfsetting );
        if ( kfsetting->_is_shutdown || kfsetting->_process_id != 0 )
        {
            return;
        }

#if __KF_SYSTEM__ == __KF_WIN__
        auto ok = StartupWinProcess( kfsetting );
#else
        auto ok = StartupLinuxProcess( kfsetting );
#endif
        if ( ok )
        {
            kfsetting->_startup_time = KFGlobal::Instance()->_real_time;
            UpdateProcessToServer( kfsetting );

            // 保存到文件中
            SaveProcessToFile( kfsetting );
        }
    }

    void KFDeployAgentModule::CheckServerProcess( KFLaunchSetting* kfsetting )
    {
        if ( kfsetting->_process_id == 0 )
        {
            return;
        }

#if __KF_SYSTEM__ == __KF_WIN__
        CheckWinProcess( kfsetting );
#else
        CheckLinuxProcess( kfsetting );
#endif
        if ( kfsetting->_process_id != 0 )
        {
            return;
        }

        // 更新状态
        UpdateProcessToServer( kfsetting );

        // 保存到文件中
        SaveProcessToFile( kfsetting );
    }

    void KFDeployAgentModule::BindServerProcess()
    {
        for ( auto& iter : _kf_deploy_agent_config->_kf_launch_setting._objects )
        {
            auto kfsetting = iter.second;

            ReadProcessFromFile( kfsetting );
            if ( kfsetting->_process_id == 0 )
            {
                continue;
            }

            if ( kfsetting->_is_startup )
            {
                UpdateProcessToServer( kfsetting );
            }
            else
            {
                KillServerProcess( kfsetting->_process_id );
            }
        }
    }

    void KFDeployAgentModule::KillServerProcess( uint32 processid )
    {
        if ( processid == 0 )
        {
            return;
        }

#if __KF_SYSTEM__ == __KF_WIN__
        KillWinProcess( processid );
#else
        KillLinuxProcess( processid );
#endif
    }

#if __KF_SYSTEM__ == __KF_WIN__
    bool KFDeployAgentModule::StartupWinProcess( KFLaunchSetting* kfsetting )
    {
        // 启动进程
        STARTUPINFO startupinfo;
        memset( &startupinfo, 0, sizeof( STARTUPINFO ) );
        startupinfo.cb = sizeof( STARTUPINFO );
        startupinfo.wShowWindow = SW_SHOW;
        startupinfo.dwFlags = STARTF_USESHOWWINDOW;

        uint32 createflag = CREATE_NEW_CONSOLE;
        if ( !kfsetting->_is_show_window )
        {
            createflag = CREATE_NO_WINDOW;
        }

        auto apppatch = _kf_deploy_agent_config->_deploy_path + kfsetting->_app_path + "/";
        auto startupfile = apppatch + kfsetting->GetStartupFile();
        auto param = KFUtility::Format( " %u %u %s", kfsetting->_is_pause,
                                        kfsetting->_app_id, kfsetting->_app_config.c_str() );

        // 启动进程
        PROCESS_INFORMATION processinfo;
        BOOL result = CreateProcess( startupfile.c_str(), param, NULL, NULL, FALSE, createflag, NULL,
                                     apppatch.c_str(), &startupinfo, &processinfo );

        kfsetting->_process_id = processinfo.dwProcessId;
        if ( kfsetting->_process_id != _invalid_int )
        {
            KFLogger::LogLogic( KFLogger::Info, "startup [ %s:%s:%u ] ok! process=%u",
                                kfsetting->_app_name.c_str(), kfsetting->_app_type.c_str(),
                                kfsetting->_app_id, kfsetting->_process_id );
        }
        else
        {
            KFLogger::LogLogic( KFLogger::Error, "startup [ %s:%s:%u ] failed!",
                                kfsetting->_app_name.c_str(), kfsetting->_app_type.c_str(),
                                kfsetting->_app_id );
        }

        return kfsetting->_process_id != _invalid_int;
    }

    void KFDeployAgentModule::CheckWinProcess( KFLaunchSetting* kfsetting )
    {
        DWORD flag = PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION;
        HANDLE handle = OpenProcess( flag, FALSE, kfsetting->_process_id );
        if ( handle != nullptr )
        {
            DWORD code = STILL_ACTIVE;
            GetExitCodeProcess( handle, &code );
            if ( code == STILL_ACTIVE )
            {
                return;
            }

            TerminateProcess( handle, 0 );
        }

        kfsetting->_process_id = 0;
    }

    void KFDeployAgentModule::KillWinProcess( uint32 processid )
    {
        DWORD flag = PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION;
        HANDLE handle = OpenProcess( flag, FALSE, processid );
        if ( handle == nullptr )
        {
            return;
        }

        TerminateProcess( handle, 0 );
    }

#else
    bool KFDeployAgentModule::StartupLinuxProcess( KFLaunchSetting* kfsetting )
    {
        std::vector<char*> args;

        auto apppath = _kf_deploy_agent_config->_deploy_path + kfsetting->_app_path + "/";
        auto startupfile = apppath + kfsetting->GetStartupFile();
        args.push_back( const_cast< char* >( startupfile.c_str() ) );

        auto strpause = __KF_STRING__( kfsetting->_is_pause );
        args.push_back( const_cast< char* >( strpause.c_str() ) );

        auto strappid = __KF_STRING__( kfsetting->_app_id );
        args.push_back( const_cast< char* >( strappid.c_str() ) );

        args.push_back( const_cast< char* >( kfsetting->_app_config.c_str() ) );
        args.push_back( nullptr );

        auto pid = fork();
        if ( pid == 0 )	 // 子进程
        {
            // 新的目录
            if ( chdir( apppath.c_str() ) != 0 )
            {
                _exit( 72 );
            }

            // 关闭资源
            for ( int i = 3; i < sysconf( _SC_OPEN_MAX ); ++i )
            {
                close( i );
            }

            // 启动新进程
            execvp( args[ 0 ], &args[ 0 ] );
            _exit( 72 );
        }

        // 等待返回
        int32 rc;
        do
        {
            int32 status;
            rc = waitpid( pid, &status, 0 );
        } while ( rc < 0 && errno == EINTR );

        kfsetting->_process_id = FindProcessIdByName( kfsetting, startupfile );
        return true;
    }

    uint32 KFDeployAgentModule::FindProcessIdByName( KFLaunchSetting* kfsetting, const std::string& startupfile )
    {
        char strtext[ 128 ] = { 0 };
        sprintf( strtext, "%s %u %u", startupfile.c_str(),
                 kfsetting->_is_pause, kfsetting->_app_id );

        char strshell[ 256 ] = { 0 };
        sprintf( strshell, "ps -ef|grep '%s'|grep -v 'grep'|awk '{print $2}'",
                 strtext );

        FILE* fp = popen( strshell, "r" );
        if ( fp != nullptr )
        {
            // 逐行读取执行结果
            char buff[ 32 ] = { 0 };
            if ( fgets( buff, sizeof( buff ), fp ) != NULL )
            {
                return atoi( buff );
            }

            // 关闭管道指针
            pclose( fp );
        }

        return _invalid_int;
    }

    void KFDeployAgentModule::CheckLinuxProcess( KFLaunchSetting* kfsetting )
    {
        if ( kill( kfsetting->_process_id, 0 ) != 0 )
        {
            kfsetting->_process_id = 0;
        }
    }

    void KFDeployAgentModule::KillLinuxProcess( uint32 processid )
    {
        auto strkill = KFUtility::Format( "kill -s 9 %u", processid );
        FILE* fp = popen( strkill, "r" );
        if ( fp != nullptr )
        {
            pclose( fp );
        }
    }
#endif

    void KFDeployAgentModule::UpdateProcessToServer( KFLaunchSetting* kfsetting )
    {
        if ( _deploy_server_id == 0 )
        {
            return;
        }

        KFMsg::S2SUpdateServerStatusReq req;
        req.set_agentid( KFGlobal::Instance()->_app_id );
        req.set_appid( kfsetting->_app_id );
        req.set_appname( kfsetting->_app_name );
        req.set_apptype( kfsetting->_app_type );
        req.set_process( kfsetting->_process_id );
        req.set_startuptime( kfsetting->_startup_time );
        req.set_isshutdown( kfsetting->_is_shutdown );
        _kf_tcp_client->SendNetMessage( _deploy_server_id, KFMsg::S2S_UPDATE_SERVER_STATUS_REQ, &req );
    }

    char* KFDeployAgentModule::FormatPidFileName( KFLaunchSetting* kfsetting )
    {
        static char file[ 128 ] = "";

        // 文件名字
        sprintf( file, "%s/%s/%s-%s-%u", _pid_path.c_str(), kfsetting->_app_path.c_str(),
                 kfsetting->_app_name.c_str(), kfsetting->_app_type.c_str(), kfsetting->_app_id );

        return file;
    }

    void KFDeployAgentModule::SaveProcessToFile( KFLaunchSetting* kfsetting )
    {
        auto file = FormatPidFileName( kfsetting );

        auto pidpath = _pid_path + "/" + kfsetting->_app_path;
        __MKDIR__( pidpath );

        std::ofstream offile( file );
        if ( !offile )
        {
            return;
        }

        offile << kfsetting->_process_id;
        offile << DEFAULT_SPLIT_STRING;
        offile << kfsetting->_startup_time;

        offile.flush();
        offile.close();
    }

    void KFDeployAgentModule::ReadProcessFromFile( KFLaunchSetting* kfsetting )
    {
        auto file = FormatPidFileName( kfsetting );

        std::ifstream infile( file );
        if ( !infile )
        {
            return;
        }

        std::ostringstream os;
        infile >> os.rdbuf();
        infile.close();

        std::string strdata = os.str();
        kfsetting->_process_id = KFUtility::SplitValue< uint32 >( strdata, DEFAULT_SPLIT_STRING );
        kfsetting->_startup_time = KFUtility::SplitValue< uint64 >( strdata, DEFAULT_SPLIT_STRING );
    }

    __KF_MESSAGE_FUNCTION__( KFDeployAgentModule::HandleStartupServerReq )
    {
        __PROTO_PARSE__( KFMsg::S2SStartupServerToAgentReq );

        for ( auto& iter : _kf_deploy_agent_config->_kf_launch_setting._objects )
        {
            auto kfsetting = iter.second;

            auto isserver = kfsetting->IsAppServer( kfmsg.appname(), kfmsg.apptype(), kfmsg.appid() );
            if ( isserver )
            {
                kfsetting->_is_shutdown = false;
            }
        }
    }

    __KF_MESSAGE_FUNCTION__( KFDeployAgentModule::HandleShutDownServerReq )
    {
        __PROTO_PARSE__( KFMsg::S2SShutDownServerToAgentReq );

        for ( auto& iter : _kf_deploy_agent_config->_kf_launch_setting._objects )
        {
            auto kfsetting = iter.second;

            auto isserver = kfsetting->IsAppServer( kfmsg.appname(), kfmsg.apptype(), kfmsg.appid() );
            if ( isserver )
            {
                kfsetting->_is_shutdown = true;
            }
        }

        KFMsg::S2SShutDownServerToMasterReq req;
        req.set_appname( kfmsg.appname() );
        req.set_apptype( kfmsg.apptype() );
        req.set_appid( kfmsg.appid() );
        req.set_delaytime( kfmsg.delaytime() );
        _kf_tcp_server->SendNetMessage( KFMsg::S2S_SHUT_DOWN_SERVER_TO_MASTER_REQ, &req );
    }

    __KF_MESSAGE_FUNCTION__( KFDeployAgentModule::HandleKillServerReq )
    {
        __PROTO_PARSE__( KFMsg::S2SKillServerToAgentReq );

        for ( auto& iter : _kf_deploy_agent_config->_kf_launch_setting._objects )
        {
            auto kfsetting = iter.second;

            auto isserver = kfsetting->IsAppServer( kfmsg.appname(), kfmsg.apptype(), kfmsg.appid() );
            if ( isserver )
            {
                kfsetting->_is_shutdown = true;
                KillServerProcess( kfsetting->_process_id );
            }
        }
    }
}