#ifndef _CONNECT_INDEX_SERVER_
#define _CONNECT_INDEX_SERVER_

#include <pthread.h>

extern pthread_mutex_t lock_servsock;		//mutex lock for the socket that faces the index server
int connect_to_index_server();

#endif
