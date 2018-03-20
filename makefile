CXX=g++
CXXFLAGS=-std=c++0x -g
LIBS= -lnsl  -lpthread
RPCGEN_FILE=communicate.x

SVC=communicate_svc.cpp
CLI=communicate_clnt.cpp
XDR=communicate_xdr.cpp

#SVC_SRC= $(XDR) $(SVC) communicate_server.cpp base_peer_client.cpp rw_peer_client.cpp article.cpp
SVC_SRC= $(XDR) $(SVC) communicate_server.cpp base_peer_client.cpp seq_peer_client.cpp article.cpp

CLI_SRC=$(XDR) $(CLI) communicate_client.cpp

CLIENT=clientside
SERVER=serverside

all: $(CLIENT) $(SERVER)
#
# rpc:
# 	rpcgen -N $(RPCGEN_FILE)
# 	rename 's/\.c/.cpp/' communicate_svc.c communicate_clnt.c communicate_xdr.c

$(CLIENT): $(CLI_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(SERVER): $(SVC_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(CLIENT) \
	    $(SERVER)
