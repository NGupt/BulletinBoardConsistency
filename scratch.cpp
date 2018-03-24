#include "quorum.h"

using std::namespace;
using std::vector;
using std::mutex;
using std::make_shared;


int QuoSer::synchronizer(Article art) {
    char *temp_articles = *read_1(pclnt);
    // cout << "empty article pool, output after update:\n" << temp_articles << endl;
    articlePool.releaseAll();  //for synchronization, clearing the bulletin board first
    decode_articles(temp_articles);
    cout << "existing articles were\n" << articlePool.read() << endl;
    return 0;
}

QuoSer::QuoSer(string ip, int port, string coordinator_ip, int coordinator_port) {
    this->server_ip = ip;
    this->server_port = port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    now = this;
    char *c_ip = new char[coordinator_ip.length() + 1];
    strcpy(c_ip, coordinator_ip.c_str());
    char *o_ip = new char[ip.length() + 1];
    strcpy(o_ip, ip.c_str());

    pclnt = clnt_create(c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION, "udp");
    if (!pclnt) {printf("ERROR: Creating clnt to coordinator %s\n", c_ip);
        exit(1);
    }

    if (isCoordinator(o_ip)) {
        cout << "INFO: Coord starting" << endl;
        subscriber_lock.lock();
        num_confirmations_read = 0;
        num_confirmations_write = 0;
        subscribers.push_back(NULL);
        subscriber_lock.unlock();
    } else {
        join_server(o_ip, port);  //ToDo: Our coordinator is not part of client accessible server list in case of quorum consistency
        get_server_list();
    }


        sockaddr_in si_me;
        //printf("Begin listening to requests from the other servers...\n");
        insert_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (insert_listen_fd == -1) {
            perror("Error: creating socket");
            throw;
        }
        int optval = 1;
        setsockopt(insert_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
        memset(&si_me, 0, sizeof(si_me));
        si_me.sin_family = AF_INET;
        si_me.sin_addr.s_addr= htonl(INADDR_ANY);

        if (isCoordinator(o_ip)) {
            si_me.sin_port = htons(coordinator_port);
            cout << "Coordinator started" << endl;
        } else {
            si_me.sin_port = htons(server_port);
            cout << "Backup peer server started" << endl;
        }

        if (bind(insert_listen_fd, (struct sockaddr*)&si_me, sizeof(si_me)) == -1)     {
            close(insert_listen_fd);
            insert_listen_fd = -1;
            perror("binding socket");
            throw;
        }

        if(!isCoordinator(o_ip)) {
            get_server_list();
            //if server joined later, then get the latest article copy
            if(articlePool.read() == ""){
                char *temp_articles = *read_1(pclnt);
                // cout << "empty article pool, output after update:\n" << temp_articles << endl;
                decode_articles(temp_articles);
                cout << "existing articles were\n" << articlePool.read() << endl;
            }
            //create listening udp thread
            insert_listen_thread = thread(listen_from, this, c_ip, server_port);
            insert_listen_thread.detach();
        }
        cout << "Initialization complete" << endl;

        std::cout << ".....Completed Server creation.....\n";
    }
}

QuoSer::~QuoSer() {
    articlePool.releaseAll();
    if (pclnt) {
        clnt_destroy(pclnt);
    }
    if(insert_listen_thread.joinable()){
        insert_listen_thread.join();
    }
    if(update_thread.joinable()){
        update_thread.join();
    }
    close(this->insert_listen_fd);
}

/* Starts a read vote for the identified article */
/* Returns when vote is completed. */
//Has to be integrated with ArticlePool structure
//

//TODO: Passing the entire articlepool here.
int QuoSer::ReadVote(ArticlePool articlePool) {

// initializing the vote to read article

     vector < PeerClient* > voters;
    voters.push_back(-1);
//So here we need to get number of connections before starting the vote. Hence we need to iterate over the server list and push them
   // to our vector
for (int i=0; i< serverlist();i++) {
    voters.push_back(i);
}

    auto it = voters.begin();
    //Should be number of active clients
    int start_size =   serverlist().size() ; //TODO : serverlist.size() ?? How to get ?
    int num_votes = 0;

    while(num_votes <= start_size/2) {

        if (it == voters.end()) {

            it = voters.begin();

        }
        //We need ask our own vote as well !?
        if ((*it) != -1){

            //If not an active connection
            if () //TODO: How to check in udp whether a server is active or not
            {
                it = voters.erase(it);

            }
            else if (udp->send() ) // TODO:We send the request to server
             {

            }

            else {
                char * response ;
                //To check for vote response
                if (udp->receive) {



                }
                else if (response[0] =1 ) {

                    //Response YES received
                    num_votes++;


                }
                else{
                    it++;

                }


            }


        }


        else {
            int vote = getReadVote(id);
            if (vote) {
                it = voters.erase(it);
                num_votes++;
            }
            else {
                ++it;
            }
        }
    }

    //At this point voting is done
    /* Vote done. Push current version to all subs */
    printf("INFO: Read vote %d concluded\n", id);
    crit.lock();
    Article cur_vers = art_tree[id]; //TODO: Sending updated article
    crit.unlock();

    clearReadVote(id); //Clearing all votes



    }


    /* Vote done. Push current version to all subscribers */
    printf("INFO: Read vote %d concluded\n", id);
    crit.lock();
    Article cur_vers = art_tree[id];
    crit.unlock();
    synchronizer(cur_vers);

    return 0;
}

int QuoServer::getReadVote(int id)
{
    printf("INFO: Read vote %d requested\n", id);
    int vote = !isWriter(id);
    if (vote)
    {
        printf("INFO: %s voted for READ %d\n", our_ip, id);
    }
    else
    {
        printf("INFO: %s voted against READ %d\n", our_ip, id);
    }

    if (!vote)
    {
        return 0;
    }

    if (tryReadLock(id))
    {
        printf("INFO: Read-locking %d\n", id);
    }

    printf("INFO: Read vote sent\n");
    return vote;
}

int QuoServer::clearReadVote(int id)
{
    printf("INFO: Clearing READ %d\n", id);
    if (isReader(id))
    {
        readlock[id]->unlock();
    }

    return 0;
}

