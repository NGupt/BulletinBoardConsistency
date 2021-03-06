CXX=g++
CXXFLAGS=-std=c++0x -g -fpermissive
LIBS= -lnsl  -lpthread
RPCGEN_FILE=communicate.x

SVC=communicate_svc.cpp
CLI=communicate_clnt.cpp
XDR=communicate_xdr.cpp

SVC_SRC_SEQ= $(XDR) $(SVC) communicate_server.cpp base_peer_client.cpp seq_peer_client.cpp article.cpp
SVC_SRC_RW= $(XDR) $(SVC) communicate_server.cpp base_peer_client.cpp rw_peer_client.cpp article.cpp
SVC_SRC_QUO= $(XDR) $(SVC) communicate_server.cpp quorum_peer_client.cpp base_peer_client.cpp article.cpp

CLI_SRC=$(XDR) $(CLI) communicate_client.cpp

CLIENT=clientside
SERVER_SEQ=serverside_seq
SERVER_RW=serverside_rw
SERVER_QUO=serverside_quo

all: $(CLIENT) $(SERVER_SEQ) $(SERVER_RW) $(SERVER_QUO)
#
# rpc:
# 	rpcgen -N $(RPCGEN_FILE)
# 	rename 's/\.c/.cpp/' communicate_svc.c communicate_clnt.c communicate_xdr.c

$(CLIENT): $(CLI_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(SERVER_SEQ): $(SVC_SRC_SEQ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(SERVER_RW): $(SVC_SRC_RW)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(SERVER_QUO): $(SVC_SRC_QUO)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(CLIENT) $(SERVER_SEQ) $(SERVER_RW) $(SERVER_QUO)
