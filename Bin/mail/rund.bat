

echo Starting Mail Cluster Server
start "Mail_Master" "KFStartupd.exe" "appid=100.0.61.1" "log=1" "startup=./setting/master.startup"

echo Starting Mail Proxy Server
start "Mail_Proxy" "KFStartupd.exe" "appid=100.0.62.1" "log=1" "startup=./setting/proxy.startup"

echo Starting Mail Shard Server
start "Mail_Shard" "KFStartupd.exe" "appid=100.0.63.1" "log=1" "startup=./setting/shard.startup"

