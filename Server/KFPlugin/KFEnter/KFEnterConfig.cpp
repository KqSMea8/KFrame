﻿#include "KFEnterConfig.h"

namespace KFrame
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    KFEnterConfig::KFEnterConfig()
    {

    }

    KFEnterConfig::~KFEnterConfig()
    {

    }


    bool KFEnterConfig::LoadConfig( const char* file )
    {
        _kf_enter_setting.clear();

        try
        {
            KFXml kfxml( file );
            auto config = kfxml.RootNode();
            //////////////////////////////////////////////////////////////////
            auto node = config.FindNode( "Setting" );
            while ( node.IsValid() )
            {
                KFEnterSetting setting;
                setting._note_id = node.GetUInt32( "NoteId" );
                setting._lua_file = node.GetString( "LuaFile" );
                setting._lua_function = node.GetString( "LuaFunction" );

                _kf_enter_setting.push_back( setting );
                node.NextNode();
            }
        }
        catch ( ... )
        {
            return false;
        }

        return true;
    }
}