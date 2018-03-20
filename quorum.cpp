#include "quorum.h"
#include <cstdlib>
using std::vector;
using std::mutex;
using std::make_shared;

const char *coordinator_ip = "128.101.37.26";

QuoSer::QuoSer(string ip, int port, string coordinator_ip, int coordinator_port ) {
    this->server_ip = ip;
    this->server_port = port;
    this->coordinator_ip = coordinator_ip;
    this->coordinator_port = coordinator_port;
    now = this;
    char *c_ip = new char[coordinator_ip.length() + 1];
    strcpy(c_ip, coordinator_ip.c_str());
    char *o_ip = new char[ip.length() + 1];
    strcpy(o_ip, ip.c_str());
    
  pclnt = clnt_create(c_ip, COMMUNICATE_PROG, COMMUNICATE_VERSION,
 			       "udp");
   if (!pclnt)    {
       printf("ERROR: Creating clnt to coordinator %s\n", c_ip);
       exit(1);
     }
 }

if (isCoordinator(o_ip)) {
  //num_confirmations_read = 0;
    printf("INFO: Coord starting\n");
    sub_lock.lock();
    num_confirmations_read = 0;
    num_confirmations_write = 0;
    sub.push_back(NULL);
    sub_lock.unlock();
  }
 else {
     int *res = register_1(ip, coord_clnt);
     if (!res) {
	 printf("ERROR: Registering %s to coord %s\n", our_ip, coord_ip);
	 exit(1);
       }

   }

QuoSer::~QuoSer() {
  if (isCoordinator())    {
      clnt_destroy(pclnt);
    }
  for (int i = 0; i < sub.size(); i++) {
      if (sub[i]) {
	  clnt_destroy(sub[i]);
	}
    }
}

//Should queue an article to update in coordinator
int QuoSer::queue_up(int art_id, string content)  {
  //TODO check for article id etc.

  ArticlePool art = ;
  if (!isCoordinator()) {

    int *vote = init_write_vote_1(art, pclnt);

    if (!vote) {
      return -1;
    }
    return *vote;
  }
  else {

    return initWriteVote(art)
  }


}

int QuoSer::read(int id) {

  if (!WriteLock(id)) {
      writelock[id]->lock();
    }

  bool rl = ReadLock(id);
  writelock[id]->unlock();

  if (!rl)  {
      readlock[id]->lock();
    }

  crit.lock();
  int mod_time = getModTime(id);
  crit.unlock();

  ArticlePool art;
  int *ret , status;
  if (!isCoordinator())   {
      ret = init_read_vote_1(id, mod_time, pclnt);
      if (!ret) {
	  status = -1;
	}
      else {
	  status = *ret;
	}
    }
  else {
      status = initReadVote(id, mod_time);
    }
  readlock[id]->unlock();

  crit.lock();
  if (status != -1) {
      art = BaseServer::read(id);
      crit.unlock();
    }
  else /* Return bad art */ {
      printf("ERROR: Could not send read to coordinator\n");
      crit.unlock();
      // art = createBlankArt();
      art.id = -1;
    }

  return art;
}
int QuoSer::insert(Article art)
{
  crit.lock();
  printf("INFO: Inserting %d under %d\n", art.id, art.root);
  art_tree[art.id] = art;
  if (art.id > art_tree[art.root].mod_time)
    {
      art_tree[art.root].mod_time = art.id;
      addLeaf(art.root, art.id);
    }
  crit.unlock();

  if (isWriter(art.root))
    {
      printf("INFO: Unlocking %d\n", art.root);
      writelock[art.root]->unlock();
    }

  return 0;
}

int QuoSer::addSub(const char *ip) {
  if (!isCoordinator())  {
      printf("ERROR: Reg request sent to non-coord\n");
      return -1;
    }

  printf("INFO: Reg req from %s\n", ip);

  CLIENT *sub = clnt_create(ip, BUILITIN_BOARD_PROG,
			    BUILITIN_BOARD_VERS, "udp");
  crit.lock();
  sub.push_back(sub);
  crit.unlock();

  return 0;
}

int QuoSer::WriteVote(Article art) {
  if (!isCoordinator()) {
      printf("ERROR: Init write vote sent to non-coordinator.");
      return -1;
    }

  crit.lock();
  auto it = art_tree.find(art.root);
  if (it == art_tree.end()) {
      crit.unlock();
      printf("ERROR: Article %d DNE\n", art.root);
      return -1;
    }

  num_arts++;
  int next_id = num_arts;
  crit.unlock();

  art.id = next_id;
  art.mod_time = next_id;

  printf("INFO: Appending article %d to updates\n", art.id);

  update_lock.lock();
  updates.push_back(art);
  update_lock.unlock();

  return next_id;
}

int QuoSer::fetchWriteVote(int id, int mod_time) {
  int vote = !isReader(id);
  if (vote) {
      printf("INFO: %s voted for WRITE %d\n", our_ip, id);
    }
  else {
      printf("INFO: %s voted against WRITE %d\n", our_ip, id);
    }

  if (!vote) {
      return 0;
    }

  WriteLock(id);
  return vote;
}

int QuoSer::ReadVote(int id, int mod_time) {
  crit.lock();
  auto it = art_tree.find(id);
  if (it == art_tree.end()) {
      printf("ERROR: Article %d does not exist\n", id);
      crit.unlock();
      return -1;
    }
  crit.unlock();

  printf("INFO: Init vote to read article %d\n", id);

  vector<CLIENT *> voters = sub;
  int start_size = voters.size();

  for (auto it = voters.begin(); voters.size() > start_size / 2;) {
      if (it == voters.end()) {
	  it = voters.begin();
	}

      if ((*it) != NULL)
	{
	  int *vote = fetch_read_vote_1(id, mod_time, (*it));
	  if (!vote || *vote == 1)
	    {
	      it = voters.erase(it);
	    }
	  else
	    {
	      ++it;
	    }
	}
      else
	{
	  int vote = fetchReadVote(id, mod_time);
	  if (vote)
	    {
	      it = voters.erase(it);
	    }
	  else
	    {
	      ++it;
	    }
	}
    }

  /* Vote done. Push current version to all sub */
  printf("INFO: Read vote %d concluded\n", id);
  crit.lock();
  Article cur_vers = art_tree[id];
  crit.unlock();
  synchronizer(cur_vers);

  return 0;
}
int QuoSer::fetchReadVote(int id, int mod_time)
{
  if (isWriter(id))
    {
      return 0;
    }

  crit.lock();
  int my_mod_time = art_tree[id].mod_time;
  crit.unlock();

  int vote = my_mod_time == mod_time;

  if (vote)
    {
      printf("INFO: %s voted for READ %d\n", our_ip, id);
      ReadLock(id);
    }
  else
    {
      printf("INFO: %s voted against READ %d\n", our_ip, id);
    }

  return vote;
}

bool QuoSer::isCoordinator() const
{
  return pclnt == NULL;
}

int QuoSer::synchronizer(Article art)
{
  printf("INFO: Synchronizerhing\n");
  for (auto it = sub.begin(); it != sub.end();)  {
      /* If this is not coord */
      if ((*it) != NULL)
	{
	  int *res = insert_1(art, (*it));
	  if (!res)
	    {
	      it = sub.erase(it);
	    }
	  else
	    {
	      ++it;
	    }
	}
      else
	{
	  insert(art);
	  ++it;
	}
    }

  printf("INFO: Synchronizer complete\n");

  return 0;
}

/* Creates rlock for desired article if it does not exist, then try lock. */
bool QuoSer::ReadLock(int id)
{
  if (readlock.find(id) == readlock.end())
    {
      readlock[id] = make_shared<mutex>();
    }

  return readlock[id]->try_lock();
}

/* Creates wlock for desired article if it does not exist, then try lock. */
bool QuoSer::WriteLock(int id)
{
  if (writelock.find(id) == writelock.end())
    {
      writelock[id] = make_shared<mutex>();
    }

  return writelock[id]->try_lock();
}

/* Returns whether or not an article was locked for reading */
bool QuoSer::isReader(int id)
{
  bool ret = ReadLock(id);
  if (ret)
    {
      readlock[id]->unlock();
    }

  return !ret;
}

/* Returns whether or not an article was locked for writing */
bool QuoSer::isWriter(int id)
{
  bool ret = WriteLock(id);
  if (ret)
    {
      writelock[id]->unlock();
    }

  return !ret;
}

int QuoSer::Loop()
{
  while (1)
    {
      Article update;
      update_lock.lock();
      if (updates.size() > 0)
	{
	  auto it = updates.begin();
	  update = (*it);
	  it = updates.erase(it);
	}
      else
	{
	  update_lock.unlock();
	  continue;
	}
      update_lock.unlock();

      /* Hold write election for update */
      printf("INFO: Beginning election to reply to %d with %d\n",
	     update.root, update.id);
      vector<CLIENT *> voters = sub;
      int start_size = voters.size();

      for (auto it = voters.begin(); voters.size() > start_size / 2;) {
	  if (it == voters.end()) {
	      it = voters.begin();
	    }

	  if ((*it) != NULL) {
	      int *vote = fetch_write_vote_1(update.root, update.id, (*it));
	      if (!vote || *vote == 1) {
		  it = voters.erase(it);
		}
	      else {
		  ++it;
		}
	    }
	  else
	    {
	      int vote = fetchWriteVote(update.root, update.id);
	      if (vote) {
		  it = voters.erase(it);
		  printf("Start: %d, Have: %d, Need %d.\n",
			 start_size, voters.size(), start_size / 2);
		}
	      else {
		  ++it;
		}
	    }
	}

      printf("Write vote to add %d to %d concluded.\n", update.id,
	     update.root);

      /* Vote done. Push current version to all sub */
      synchronizer(update);
    }

  return 0;
}
