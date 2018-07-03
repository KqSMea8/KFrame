@echo off

SETLOCAL ENABLEDELAYEDEXPANSION

rem ===========================================================================
rem ===========================================================================
rem ===========================================================================
rem ===========================================================================
rem ===========================================================================

if not exist plugin (mkdir plugin)
if not exist setting (mkdir setting)
if not exist config (mkdir config)
rem ===========================================================================
rem ===========================================================================

rem ����Ŀ¼
set path1=..\..\Source\_bin\win64
set path2=.

call :copyfile 1 KFStartupd.exe
call :copyfile 1 KFStartup.exe
call :copyfile 1 log4cxxd.dll
call :copyfile 1 log4cxx.dll
call :copyfile 1 libcrypto-1_1-x64.dll
call :copyfile 1 libssl-1_1-x64.dll
rem ===========================================================================
rem ===========================================================================
rem ȫ������
set path1=..\..\Source\_bin\setting
set path2=setting
call :copyfile 0 connection.network
call :copyfile 1 initapp.log4cxx
call :copyfile 1 templateapp.log4cxx
rem ===========================================================================

rem ����Ŀ¼
set path1=..\..\Source\_bin\setting\auth
set path2=setting
call :copyfile 1 auth.network
call :copyfile 1 auth.startup

rem config
set path1=..\..\Resource\config
set path2=config
call :copyfile 0 redis.config
call :copyfile 1 option.config
call :copyfile 1 channel.config
rem ===========================================================================
rem ===========================================================================

rem ���Ŀ¼
set path1=..\..\Source\_bin\win64\plugin
set path2=plugin

call :copyfile 1 KFHttpServerd.dll
call :copyfile 1 KFHttpServer.dll

call :copyfile 1 KFHttpClientd.dll
call :copyfile 1 KFHttpClient.dll

call :copyfile 1 KFConfigd.dll
call :copyfile 1 KFConfig.dll

call :copyfile 1 KFConnection.dll
call :copyfile 1 KFConnectiond.dll

call :copyfile 1 KFAuthd.dll
call :copyfile 1 KFAuth.dll

call :copyfile 1 KFChanneld.dll
call :copyfile 1 KFChannel.dll

call :copyfile 1 KFRedisd.dll
call :copyfile 1 KFRedis.dll


rem pause
rem ===========================================================================
rem ===========================================================================
rem ===========================================================================
rem �ж�ʱ��
:check_time
set need_copy=0

if not exist %file2% (
	set need_copy=1&exit/b:check_time
)

set file1_t=""
set file2_t=""

if %1==1 (
call:get_time file1
call:get_time file2 
)

if "%file1_t%" gtr "%file2_t%" (
	set need_copy=1
)
goto:eof

rem ���ʱ��
:get_time
for /f "skip=4 tokens=1,2" %%a in ('dir /tw "!!%~1!!"')do (
set %~1_t=%%a %%b&exit/b:get_time)
goto:eof

rem �����ļ�
:copyfile 
set file1=%path1%\%2
set file2=%path2%\%2

call :check_time %1
if %need_copy%==1 (
echo update %file2%
copy /y %file1% %file2%
) 
goto:eof