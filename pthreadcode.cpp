#define HAVE_STRUCT_TIMESPEC

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#include "pthread.h"

#include "Config.h"
#include "serial_radarAPI_if.h"
#include "serial_radar.h"
#include "pthreaddefs.h"
#include "json.hpp"
#include "ConfigPageHandler.h"

using namespace nlohmann;
int startSerialRadar(int argc, char *argv[]);
int startSPIUSB();
int startSerialConsumer();
int startWebServer();
void processParms(int argc, char *args[]);
int comport=0;
char *mqtt_server="";
char *mqtt_client = "";
char *mqtt_password = "";
char *mqtt_topic = "";

Config *cfg = nullptr;


//Buf_t bufparm;



int startThreads() {
	// initRadar();
#define NUM_THREADS 4

	pthread_t thr[NUM_THREADS];
	int i, rc;
	/* create a thread_data_t argument array */
	Buf_t thr_data[NUM_THREADS];

	/* create threads */
	for (i = 0; i < NUM_THREADS; ++i) {

		pthread_mutex_init(&thr_data[i].buflock, NULL);
		pthread_mutex_init(&thr_data[i].donelock, NULL);
		pthread_cond_init(&thr_data[i].adddata, NULL);
		pthread_cond_init(&thr_data[i].remdata, NULL);
		if ((rc = pthread_create(&thr[i], NULL, serialConsumer, &thr_data[i]))) {
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return EXIT_FAILURE;
		}
	}
	/* block until all threads complete */
	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_join(thr[i], NULL);
	}
	return NUM_THREADS;
}
int main(int argc, char *argv[])
{
	char * argsx[7] = { "",
		"--port"
		, "9090"
		, "--mqtt"
		,"--mqttclient" 
	, "--comport"
	, "7"};


	processParms(argc, argv);

	cfg = new Config("config.conf");
	if (cfg->isConfFileCorrupt()) return 0;
	cfg->setmqttBroker(mqtt_server);
	cfg->setmqttClient(mqtt_client);
	cfg->setmqttPassword(mqtt_password);
	cfg->setmqttTopic(mqtt_topic);
	cfg->setcomPort(to_string(comport));
	if (!cfg->isPopulated()) {
		iSYSResult res;
		//cfg->createDefault("config.conf", 8);d
		cfg->setNotify("5");
		res = initRadar(cfg, true);
		if (ERR_OK == res) {
			FILE *fp = fopen("config.conf", "w");
			json j1;
			cfg->toJSON(j1);
			string print = j1.dump(4);
			fprintf(fp, print.c_str());
			cout << print.c_str() << endl;
			fclose(fp);
		}
	}

	//return 0;
	pthread_setconcurrency(8);

	spiUSB(nullptr);

	startSerialRadar(argc, argv);

	//startThreads();

	//spiInit();

	//startSPIUSB();


}

int startSPIUSB()
{

	pthread_t thr[NUM_THREADS];
	int i, rc;
	/* create a thread_data_t argument array */
	Buf_t thr_data[NUM_THREADS];

	/* create threads */
	for (i = 0; i < NUM_THREADS; ++i) {

		pthread_mutex_init(&thr_data[i].buflock, NULL);
		pthread_mutex_init(&thr_data[i].donelock, NULL);
		pthread_cond_init(&thr_data[i].adddata, NULL);
		pthread_cond_init(&thr_data[i].remdata, NULL);

		if ((rc = pthread_create(&thr[i], NULL, SPIProducer, &thr_data[i]))) {
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return EXIT_FAILURE;
		}
	}
	/* block until all threads complete */
	for (i = 0; i < NUM_THREADS; ++i) {
		pthread_join(thr[i], NULL);
	}

	return 1;
}

/*int startSerialConsumer() {

	pthread_t radar_thread;

	/* create a second thread which executes inc_x(&x) 
	if (pthread_create(&radar_thread, NULL, serialConsumer, &bufparm)) {

		fprintf(stderr, "Error creating radar thread\n");
		return 0;

	}

	/* wait for the second thread to finish 
	if (pthread_join(radar_thread, NULL)) {

		fprintf(stderr, "Error joining thread\n");
		return 2;

	}
	return 1;
}
*/
int startSerialRadar(int argc, char *argv[])
{
	pthread_t webgo_thread;
	pthread_t radar_thread;
	Buf_t bufparm;
	bufparm.argc = argc;
	bufparm.args = argv;
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << endl;
	}
	pthread_mutex_init(&bufparm.webgo, NULL);
	/* create a second thread which executes inc_x(&x) */
	if (pthread_create(&radar_thread, NULL, serialRadar, &bufparm)) {

		fprintf(stderr, "Error creating radar thread\n");
		return 0;

	}


	if (pthread_create(&webgo_thread, NULL, config_server, &bufparm)) {

		fprintf(stderr, "Error creating radar thread\n");
		return 0;

	}

	/* wait for the second thread to finish */
	if (pthread_join(radar_thread, NULL)) {

		fprintf(stderr, "Error joining thread\n");
		return 2;

	}
	return 1;
}


void processParms(int argc, char *args[]) {

	for (int i = 0; i < argc; i++) {
		char **argslocal = args;
		cout << "parm serial radar " << args[i] << endl;
		if (0 == strcmp(args[i], "--mqtt")) {
			cout << " got mqtt " << endl;
			if (i + 1 < argc) {
				cout << " mqtt val:" << args[i + 1] << endl;
				mqtt_server = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add an mqtt server" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--client")) {
			cout << " got client " << endl;
			if (i + 1 < argc) {
				cout << " client val:" << args[i + 1] << endl;
				mqtt_client = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add an mqtt client" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--config")) {

			cout << " got config " << endl;
			if (i + 1 < argc) {
				cout << " config val:" << args[i + 1] << endl;
				mqtt_client = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add a config file" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--commport")) {

			cout << " got comport " << endl;
			if (i + 1 < argc) {
				cout << " comport val:" << args[i + 1] << endl;
				comport = atoi(args[i + 1]);
				i++;
			}
			else
			{
				cout << "need to add a config file" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--mqtt_topic")) {

			cout << " got mqtt topic " << endl;
			if (i + 1 < argc) {
				cout << " topic val:" << args[i + 1] << endl;
				mqtt_topic = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add a config file" << endl;
			}
		}
	}
}
