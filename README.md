# BulletinBoardConsistency
A multithreaded Bulletin Board (BB) system in which clients can post, reply, and read articles stored in the BB. The BB is maintained by a group of replicated servers that offer sequential consistency, Quorum consistency and Read-your-Write consistency. <br/>
<br />

To compile:
```
make clean
make rpc
make all
```

How to use:

Launch the RPC server with ./serverside

Launch a client application in another process. Its usage is as follows;
./clientside “local ip” “local port” “server ip”

Options on client side (post | read | choose | reply) where each has its function as follows -

1) Post will return 0 if post fails, otherwise, it will return the id of posted article.
2) Read will read all articles from the server.
3) Choose will read an article with the given id.
4) Reply will post a reply article to an existing article with a certain id.
