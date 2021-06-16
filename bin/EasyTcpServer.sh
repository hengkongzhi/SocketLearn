#进入脚本所在目录
cd `dirname $0`
#服务器地址
strIP="any"
#服务器端口
nPort=4567
#消息处理线程数量
nThread=1
#客户端连接上限
nClient=2
./serverNew ${strIP} ${nPort} ${nThread} ${nClient}
read -p "..press any key to exit.." var