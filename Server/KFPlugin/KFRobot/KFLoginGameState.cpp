﻿#include "KFLoginGameState.h"
#include "KFRobot.h"

namespace KFrame
{
    void KFLoginGameState::EnterState( KFRobot* kfrobot )
    {
        // 启动定时器, 10秒认证一次
        kfrobot->StartTimer( _kf_robot_config->_state_rep_time );
    }

    void KFLoginGameState::LeaveState( KFRobot* kfrobot )
    {
        kfrobot->StopTimer();
    }

    void KFLoginGameState::CheckState( KFRobot* kfrobot )
    {

    }

    void KFLoginGameState::RunState( KFRobot* kfrobot )
    {
        if ( !kfrobot->DoneTimer() )
        {
            return;
        }


        kfrobot->ConnectServer( kfrobot->_gate_server_type, kfrobot->_robot_id,
                                "robot", kfrobot->_connect_ip, kfrobot->_connect_port );
    }
}