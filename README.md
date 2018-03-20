# BulletinBoardConsistency
A multithreaded Bulletin Board (BB) system in which clients can post, reply, and read articles stored in the BB. The BB is maintained by a group of replicated servers that offer sequential consistency, Quorum consistency and Read-your-Write consistency. <br/>
<br />

To compile:
```
make clean
make all
```

How to use:
````
Sequential consistency: Launch the RPC server with: ./serverside_seq   “local ip” “local port” "coordinator ip" "coordinator port"
Read yur Write consistency: Launch the RPC server with: ./serverside_rw   “local ip” “local port” "coordinator ip" "coordinator port"
Client application: Launch in another process with: ./clientside “local ip” “server ip”
````
NOTE: If coordinator_ip and local_ip are same, it is a coordinator

Example of launching:
````
Coordinator:   `./serverside 128.101.37.27 1234 128.101.37.27 3456`
Peer Server1:   `./serverside 128.101.37.11 2345 128.101.37.27 3456`
Peer Server2:   `./serverside 128.101.37.07 4444 128.101.37.27 3456`
Client     :   `./clientside 128.101.37.25 128.101.37.11`
````
Options on client side (post | read | choose | reply | get_server_list) where each has its function as follows -

1) Post will return 0 if post fails, otherwise, it will return the id of posted article.
2) Read will read all articles from the server.
3) Choose will read an article with the given id.
4) Reply will post a reply article to an existing article with a certain id.
5) Get server list will return all the servers currently there in the network. Client can connect to any one of them.
