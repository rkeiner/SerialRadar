#pragma once
#define HAVE_STRUCT_TIMESPEC
#include "pthread.h"
#include "Config.h"
using namespace std;
#define BUFCNT 3
#define BUFSIZE sizeof(vector<uint8_t>)
#include <vector>

typedef struct {
	vector<uint8_t> msgBufs[BUFCNT][BUFSIZE];
	uint8_t nextadd, nextrem, occ;
	pthread_mutex_t buflock;
	pthread_mutex_t donelock;
	pthread_cond_t adddata;
	pthread_cond_t remdata;
	pthread_mutex_t webgo;
	bool available;
	int argc;
	char **args;
} Buf_t;

void *SPIProducer(void  *buf);
void *serialConsumer(void  *buf);
void *config_server(void  *buf);
extern Config *cfg;


