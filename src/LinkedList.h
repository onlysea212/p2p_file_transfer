#ifndef _DATA_STRUCTURE_
#define _DATA_STRUCTURE_

#include <stdint.h>

#define INSERT_NODE_SUCCESS 1
#define INSERT_NODE_NOT_IN_LL -2 
#define INSERT_NODE_ARGS_NULL -1

#define REMOVE_NODE_ARGS_NULL -1
#define REMOVE_NODE_EMPTY_LL -2
#define REMOVE_NODE_SUCCESS 1

#define SEGMENT_TYPE	0
#define INT_TYPE		1 
#define FILE_OWNER_TYPE 2
#define DATA_HOST_TYPE 3

struct FileOwner{
	char filename[256];
	uint32_t filesize;
	struct LinkedList *host_list;	// LinkedList with data type DataHost
};


struct Segment{
	uint32_t offset;
	uint32_t n_bytes;
};


/* info for each peer that own the data */
struct DataHost{
	uint32_t ip_addr;
	uint16_t port;			//equal to data_port from the struct net_info
};

struct Node{
	void *data;
	uint8_t type;			//data type
	struct Node *next;
	struct Node *prev;
};

struct LinkedList{
	struct Node *head;
	struct Node *tail;
	unsigned int n_nodes;	//number of nodes in the LL
};

struct Node* newNode(void *data, int data_type);

struct LinkedList* newLinkedList();

void destructLinkedList(struct LinkedList *ll);

int llistContain(struct LinkedList ll, struct Node *node);

int insertNode(struct LinkedList *ll, struct Node *node, struct Node *offset);

int removeNode(struct LinkedList *ll, struct Node *node);

struct Node* pop(struct LinkedList *ll);

void push(struct LinkedList *ll, struct Node *node);

#endif