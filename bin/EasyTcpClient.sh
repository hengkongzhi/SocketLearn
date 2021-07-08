#进入脚本所在目录
cd `dirname $0`
# #服务器地址
# strIP="any"
# #服务器端口
# nPort=4567
# #消息处理线程数量
# nThread=1
# #客户端连接上限
# nClient=2
cmd="strIP=127.0.0.1"
cmd="$cmd nPort=4567"
cmd="$cmd nThread=8"
cmd="$cmd nClient=150"
cmd="$cmd nMsg=1"
cmd="$cmd nSendSleep=1000"
cmd="$cmd nSendBuffSize=81920"
cmd="$cmd nRecvBuffSize=81920"
cmd="$cmd -checkMsgID"
./clientNew ${cmd}
read -p "Press any key to exit.." var