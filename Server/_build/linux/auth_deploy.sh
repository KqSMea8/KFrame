#!/bin/sh

function copyfile()
{
	file1=$path1/$2
	file2=$path2/$2
	
	needcopyfile=0
	if [ ! -f "$file1" ];then
		needcopyfile=0
	elif [ ! -f "$file2" ];then
		needcopyfile=1	
	elif [ $1 == "1" ];then
		filetime1=`stat -c %Y $file1`
        	filetime2=`stat -c %Y $file2`

        	if [ $filetime1 -gt $filetime2 ];then
                	needcopyfile=1
        	fi
	fi
	
	if [ $needcopyfile == 1 ];then
		echo "copy $file1 $file2"
		\cp $file1 $file2
	fi
}


binpath=../../_bin/linux
settingpath=../../_bin/setting
respath=../../../Resource
deploypath=/data/auth


mkdir -p $deploypath
mkdir -p $deploypath/setting
mkdir -p $deploypath/plugin
mkdir -p $deploypath/config

#setting
path1=$settingpath
path2=$deploypath/setting
copyfile 0 connection.network
copyfile 1 initapp.log4cxx
copyfile 1 templateapp.log4cxx

path1=$settingpath/auth
path2=$deploypath/setting
copyfile 1 auth.network
copyfile 1 auth.startup

#config
path1=$respath/config
path2=$deploypath/config
copyfile 0 redis.config
copyfile 1 channel.config

#plugin
path1=$binpath/
path2=$deploypath
copyfile 1 KFStartup
copyfile 1 KFStartupd

path2=$deploypath/plugin

copyfile 1 KFHttpServer.so
copyfile 1 KFHttpServerd.so
copyfile 1 KFHttpClient.so
copyfile 1 KFHttpClientd.so
copyfile 1 KFConfig.so
copyfile 1 KFConfigd.so
copyfile 1 KFAuth.so
copyfile 1 KFAuthd.so
copyfile 1 KFChannel.so
copyfile 1 KFChanneld.so
copyfile 1 KFRedis.so
copyfile 1 KFRedisd.so
copyfile 1 KFConnectiond.so
copyfile 1 KFConnection.so


