#pragma once
#define HAVE_STRUCT_TIMESPEC
#include "serial_radarAPI_if.h"
#include "pthread.h"
#include "pthreaddefs.h"
#include "serial_radar.h"
#include "serial_radarAPI_if.h"
#include "async_publish.h"
#include "json.hpp"
#include "config.h"

class ISysRadar
{
public:
	ISysRadar();
	~ISysRadar();


	iSYSResult init();
	iSYSResult run();
	iSYSResult start();
	iSYSResult stop();
	//iSYSResult shutdown();

private:



	iSYSHandle_t pHandle;
	iSYSResult_t res;
	int deviceAddress;
	int notify_interval = 0;
	char * mqtt_server = nullptr;
	char * mqtt_client = nullptr;
	string sensorConfig;
	json jsonconfig;
	bool start_acquisition = false;
	bool stop_acquisition = false;
	bool running = true;
	bool restart = false;

	uint8_t enable;
	iSYSFrequencyChannel_t radarchannel;
	iSYSSaveLocation_t saveLocation;
	uint8_t addr;
	iSYSTargetList *targetList = nullptr;

	bool myfunction(int i, int j);

	struct myclass {
		bool operator() (int i, int j) { return (i < j); }
	};
	struct myclass myobject;


private:
	iSYSResult_t decodeTargetFrame(uint8_t *pData
		, uint16_t nrOfElements
		, uint16_t productcode
		, uint8_t bitrate
		, uint8_t nrOfTargets
		, iSYSTargetList_t *targetList);

	void *serialRadar(void *x_void_ptr);
	//void *spiUSB(void *x_void_ptr);
	//void *spiInit();
	//void spiClose();

	int doesFileExist(const char *filename);
	iSYSResult stopAcquisition();
	iSYSResult startAcquisition();
	bool list_start(uint8_t msg_check[], uint8_t &entries);
	bool list_end(uint8_t msg_check[]);
	void sortRange(iSYSTarget_t *targets, int number);
	void *serialConsumer(void  *pbuf);
	void notify(iSYSTargetList_t *targetList, char desc[], bool print);
	void processParms(Buf_t *buf);
	iSYSResult_t getRadarConfig(char *file, bool update);
	iSYSResult processMessage(vector<unsigned char> msgBuf);
	void intHandler(int dummy);
	bool spiCheck(vector<unsigned char> &);

	iSYSResult decodeMessage(unsigned char * buf, uint16_t size, iSYSTargetList_t *targetList);
	iSYSResult startSensorConfig();

	void printTheList(char *filename, iSYSTargetList *targetList, char *mode, char *description, string data);
	void formatTheList(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items);

	float unpackFloat(const void *buf, int *i);

	iSYSResult stopAndStart();

	void printHex(uint8_t *buf, uint16_t size);

	iSYSResult_t initRadar();

	iSYSResult_t processRadar(Buf_t *buf);

	iSYSResult closeRadar();

	const char ERROR_CODES[38][40] = {
		"ERR_OK",
		"ERR_FUNCTION_DEPRECATED",
		"ERR_DLL_NOT_FINISHED",
		"ERR_HANDLE_NOT_INITIALIZED",
		"ERR_COMPORT_DOESNT_EXIST",
		"ERR_COMPORT_CANT_INITIALIZE",
		"ERR_COMPORT_ACCESS_DENIED",
		"ERR_COMPORT_BAUDRATE_NOT_VALID",
		"ERR_COMPORT_CANT_OPEN",
		"ERR_COMPORT_CANT_SET_FLOW_CONTROL",
		"ERR_COMPORT_CANT_SET_PARITY",
		"ERR_COMPORT_CANT_SET_STOP_BITS",
		"ERR_COMPORT_CANT_SET_DATA_BITS",
		"ERR_COMPORT_CANT_SET_BAUDRATE",
		"ERR_COMPORT_ALREADY_INITIALIZED",
		"ERR_COMPORT_EQUALS_NULL",
		"ERR_COMPORT_NOT_OPEN",
		"ERR_COMPORT_NOT_READABLE",
		"ERR_COMPORT_NOT_WRITEABLE",
		"ERR_COMPORT_CANT_WRITE",
		"ERR_COMPORT_CANT_READ",
		"ERR_COMMAND_NOT_WRITTEN ",
		"ERR_COMMAND_NOT_READ",
		"ERR_COMMAND_NO_DATA_RECEIVED",
		"ERR_COMMAND_NO_VALID_FRAME_FOUND",
		"ERR_COMMAND_RX_FRAME_DAMAGED",
		"ERR_COMMAND_FAILURE",
		"ERR_UNDEFINED_READ",
		"ERR_COMPORT_LESS_DATA_READ",
		"ERR_COMPORT_SYSTEM_INIT_FAILED",
		"ERR_COMPORT_SYSTEM_ALREADY_INITIALIZED",
		"ERR_COMMAND_RX_FRAME_LENGTH",
		"ERR_COMMAND_MAX_DATA_OVERFLOW",
		"ERR_COMMAND_MAX_IQPAIRS_OVERFLOW",
		"ERR_COMMAND_NOT_ACCEPTED",
		"ERR_NULL_POINTER",
		"ERR_CALC_CORRECTION_PARAMS",
		"ERR_PARAMETER_OUT_OF_RANGE"
	};

	enum {
		LIST_BEGIN = 0
		, LIST_START
		, LIST_PROCESSING
		, LIST_END
		, LIST_ERROR
	};
};
