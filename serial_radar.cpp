#define HAVE_STRUCT_TIMESPEC
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <signal.h>
#include <functional>
#include <conio.h>
#include <chrono>
#include <inttypes.h>
#include "SerialCamera.h"
#include <cstdio>
#include <list>
using namespace std;

#include "pthread.h"
#include "pthreaddefs.h"
#include "serial_radar.h"
#include "serial_radarAPI_if.h"
#include "async_publish.h"
#include "json.hpp"
#include "Config.h"

using namespace nlohmann;

iSYSHandle_t pHandle;
iSYSResult_t res;
int deviceAddress;
int notify_interval = 0;
string sensorConfig("Sensor:\n");
bool start_acquisition = false;
bool stop_acquisition = false;



/*****************************************************************
* Forward
******************************************************************/
iSYSResult_t decodeTargetFrame(uint8_t *pData
	, uint16_t nrOfElements
	, uint16_t productcode
	, uint8_t bitrate
	, uint8_t nrOfTargets
	, iSYSTargetList_t *targetList);

boolean spiCheck(vector<unsigned char> &);

iSYSResult decodeMessage(unsigned char * buf, uint16_t size, iSYSTargetList_t *targetList);
iSYSResult startSensorConfig();

void printTheList(char *filename, iSYSTargetList *targetList, char *mode, char *description, string data);

void formatTheList(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items);

void formatMQTTMessage(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items);

float unpackFloat(const void *buf, int *i);

iSYSResult stopAndStart();

void printHex(uint8_t *buf, uint16_t size);

iSYSResult_t initRadar(Config *cfg, bool populate);

iSYSResult_t processRadar(Buf_t *buf);

iSYSResult closeRadar();

bool running = true;

void intHandler(int dummy = 0) {
	printf("good bye\n");
	running = false;
}

//SerialCamera *camera = nullptr;
bool restart = false;
// void *serialRadar(void *x_void_ptr)


void *serialRadar(void  *pbuf)
{
	signal(SIGINT, intHandler);

	//camera = new SerialCamera();
	//camera->init();
	//printf("should have started the camera\n");



	res = initRadar(cfg, false);

	setRadarConfig(false);

	if (ERR_OK != res) {
		printf("Could not open the radar unit %s\n", ERROR_CODES[res]);
		return nullptr;
	}
	res = processRadar((Buf_t *)pbuf);
	if (ERR_OK != res) {
		printf("ERror processing the radar %s\n", ERROR_CODES[res]);
		return nullptr;
	}
	res = closeRadar();
	if (ERR_OK != res) {
		printf("ERror closing the radar %s\n", ERROR_CODES[res]);
		return nullptr;
	}


	// while (true) {
	Sleep(1000);
	//}
	return nullptr;
}

uint8_t enable;
iSYSFrequencyChannel_t radarchannel;
iSYSSaveLocation_t saveLocation;
uint8_t addr;
extern Config *cfg;


iSYSResult setRadarConfig(bool update) {
	float32_t version;
	uint16_t major, fix, minor;
	uint16_t productInfo;
	sint16_t margin = 0;
	char buf[32];
	json j;

	uint32_t serialNumber;
	char deviceName[21];

	res = iSYS_getApiVersion(&version);

	cout << "---------------------------------\n";
	cout << "    serial radarAPI Version: ---- " << version << "\n";
	cout << "---------------------------------\n\n";
	if (nullptr == pHandle) return ERR_HANDLE_NOT_INITIALIZED;

	res = iSYS_getThresholdMovingTargetsNearRangeMargin(pHandle, ISYS_LOCATION_RAM, &margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read NearRangeMargin\n";
	}
	else {
		memset(buf, 0, sizeof buf);
		snprintf(buf, sizeof buf, "%d", margin);
		sensorConfig.append("NearRangeMargin: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setThresholdMovingTargetsNearRangeMargin(to_string(margin));

	res = iSYS_getThresholdMovingTargetsMainRangeMargin(pHandle, ISYS_LOCATION_RAM, &margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read MainRangeMargin\n";
	}
	else {
		memset(buf, 0, sizeof buf);
		snprintf(buf, sizeof buf, "%d", margin);
		sensorConfig.append("MainRangeMargin: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setThresholdMovingTargetsMainRangeMargin(to_string(margin));

	res = iSYS_getThresholdMovingTargetsLongRangeMargin(pHandle, ISYS_LOCATION_RAM, &margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read LongRangeMargin\n";
	}
	else {
		memset(buf, 0, sizeof buf);
		snprintf(buf, sizeof buf, "%d", margin);
		sensorConfig.append("LongRangeMargin: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setThresholdMovingTargetsLongRangeMargin(to_string(margin));



	/* get devicename */
	res = iSYS_ReadDeviceName(pHandle, deviceName, 21, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read devicename\n";
	}
	else {
		sensorConfig.append("device name: ");
		sensorConfig.append(deviceName);
		sensorConfig.append("\n");
	}
	cfg->setDeviceName((char *)deviceName);

	/* get device address */

	res = iSYS_getDeviceAddress(pHandle, &addr, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read address \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d", deviceAddress);
		sensorConfig.append("device address: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setDeviceAddress(buf);

	/* get firmware version */

	res = iSYS_getFirmwareVersion(pHandle, &major, &fix, &minor, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read firmware version\n";
	}
	else {
		cout << "firmware version: " << major << "." << minor << "\n";
	}
	//cfg->setFirmwareVersion(major << "." << minor);
	//j["FirmwareVersion"] = cfg->getFirmwareVersion();

	/* get product info */

	res = iSYS_getProductInfo(pHandle, &productInfo, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read product info! \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d", productInfo);
		sensorConfig.append("product info:  ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setProductInfo(buf);

	/* get serialnumber */

	res = iSYS_getSerialNumber(pHandle, &serialNumber, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read serial number \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d", serialNumber);
		sensorConfig.append("serial number:  ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");

	}
	cfg->setSerialNumber(buf);

	/* get DSP hw version from sensor */

	res = iSYS_getDspHardwareVersion(pHandle, &major, &fix, &minor, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read DSP version! \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d:%d", major, minor);
		sensorConfig.append("DSP version:   ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setDspHardwareVersion(buf);

	/* get RFE hw version from sensor */

	res = iSYS_getRfeHardwareVersion(pHandle, &major, &fix, &minor, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read RFE version! \n\n";
	}
	else {
		snprintf(buf, sizeof buf, "%d:%d", major, minor);
		sensorConfig.append("RFE version:   ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	cfg->setRfeHardwareVersion(buf);
	cfg->setx("0.0");
	cfg->sety("0.0");
	cfg->setAngle("0.0");
	json j1;
	cfg->toJSON(j1);
	string print = j1.dump(4);
	cout << print.c_str() << endl;

	return res;
}

iSYSResult_t setRadarConfigFile(char *filename, Config *cfg) {
	std::string::size_type sz;   // alias of size_t
	int i_dec = std::stoi(cfg->getThresholdMovingTargetsNearRangeMargin(), &sz);
	res = iSYS_setThresholdMovingTargetsNearRangeMargin(pHandle, ISYS_LOCATION_RAM, i_dec, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set NearRangeMargin\n";
	}

	i_dec = std::stoi(cfg->getThresholdMovingTargetsMainRangeMargin(), &sz);
	res = iSYS_setThresholdMovingTargetsMainRangeMargin(pHandle, ISYS_LOCATION_RAM, i_dec, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read MainRangeMargin\n";
	}

	i_dec = std::stoi(cfg->getThresholdMovingTargetsLongRangeMargin(), &sz);
	res = iSYS_setThresholdMovingTargetsLongRangeMargin(pHandle, ISYS_LOCATION_RAM, i_dec, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read LongRangeMargin\n";
	}
	
	i_dec = std::stoi(cfg->getFrequencyChannel(), &sz);
	res = iSYS_setFrequencyChannel(pHandle, (iSYSFrequencyChannel_t)i_dec, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set Frequency Channel\n";
	}

	//res = iSYS_setTargetClusteringEnable(pHandle, saveLocation, cfg->isTargetClusteringEnable(), deviceAddress, 1000);
	res = iSYS_setTargetClusteringEnable(pHandle, saveLocation, 1, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set clustering! \n";
	}
	//res = iSYS_setRcsOutputEnable(pHandle, saveLocation, cfg->isProcessingRcsCalibrationEnable(), deviceAddress, 1000);
	res = iSYS_setRcsOutputEnable(pHandle, saveLocation, 0, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set rcs output! \n";
	}

	return res;
}

int doesFileExist(const char *filename) {
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}

iSYSResult_t initRadar(Config *cfg, bool populate)
{
	cout << endl;
	int comport = 0;
	// spiUSB(nullptr);
	std::string::size_type sz;   // alias of size_t
	if (!cfg->isPopulated() && !populate) {
		return ERR_OK;
	}
	try {
		//cout << "Please enter the serial port number COMX:";
		int i_dec = std::stoi(cfg->getcomPort(), &sz);
		comport = i_dec;

	}
	catch (std::exception err) {
		cout << "Got an exxception trying to set the com port from the cofiguration" << endl;
	}
	//cin >> comport;
	//cout << "Please enter the device address (default 400X = 128, 5XXX = 100, 600X = 100):";
	deviceAddress = 100;
	//cin >> deviceAddress;

	/* initialize com port */
	cout << "\ninitializing COM port.. " << comport << "\n";
	res = iSYS_initComPort(&pHandle, static_cast<uint8_t>(comport), ISYS_BAUDRATE_115200);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not initialize serial port - press \"-\" to exit" << "\n";
		Sleep(5000);
		return ERR_COMPORT_CANT_INITIALIZE;
	}

	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read modulation type! \n";
	}

	res = startSensorConfig();

	cout << "system successfully initialized\n\n";
	Sleep(500);

	if (populate) {
		/******************************************/
		/** set and read iSYS-5010 configuration **/
		/******************************************/

		/* get modulation type */

		res = iSYS_getFrequencyChannel(pHandle, &radarchannel, deviceAddress, 1000);
		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "can not read modulation type! \n";
		}
		else {
			if (radarchannel == ISYS_CHANNEL_1) {
				sensorConfig.append("up-ramp active \n");
			}
			else {
				sensorConfig.append("down-ramp active \n");
			}
		}
		cfg->setFrequencyChannel(to_string(radarchannel));


		/* get RCS output setting from RAM */
		res = iSYS_getRcsOutputEnable(pHandle, saveLocation, &enable, deviceAddress, 1000);
		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "can not read RCS setting! \n";
		}
		else {
			if (enable == 0) {
				cfg->setProcessingRcsCalibrationEnable(false);
				sensorConfig.append("RCS output OFF \n");
			}
			else {
				cfg->setProcessingRcsCalibrationEnable(true);
				sensorConfig.append("RCS output ON \n");
			}
		}


		/* get clustering from RAM */

		res = iSYS_getTargetClusteringEnable(pHandle, saveLocation, &enable, deviceAddress, 1000);
		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "can not read clustering setting! \n";
		}
		else {
			if (enable == 0) {
				sensorConfig.append("clustering OFF \n");
				cfg->setTargetClusteringEnable(false);
			}
			else {
				sensorConfig.append("clustering ON \n");
				cfg->setTargetClusteringEnable(true);

			}
		}
		Sleep(500);

		cout << sensorConfig.c_str();
		printTheList("listprint.txt", nullptr, "w", "Sensor:\n", sensorConfig);
		res = setRadarConfig(true);
		if (ERR_OK == res) {
			res = stopAndStart();
		}
	}
	return res;
}

iSYSResult startSensorConfig()
{
	/* init system with deviceaddress and timeout 1000ms */
	cout << "initializing sensor.. \n";
	res = iSYS_initSystem(pHandle, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		//cout << "could not initialize system -- program closes automatically";
		Sleep(5000);
		return ERR_HANDLE_NOT_INITIALIZED;
	}
	radarchannel = ISYS_CHANNEL_2;
	res = iSYS_setFrequencyChannel(pHandle, radarchannel, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set up-ramp modulation! \n";
		return res;
	}
	/* set RCS output in target list ON and store the value in RAM */
	saveLocation = ISYS_LOCATION_RAM;

	enable = 0;
	res = iSYS_setRcsOutputEnable(pHandle, saveLocation, enable, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set RCS output ON! \n";
	}


	/* set clustering enable and store value in RAM */

	enable = 1;
	res = iSYS_setTargetClusteringEnable(pHandle, saveLocation, enable, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set clustering! \n";
	}
	res = stopAndStart();

	return res;
}
/****************************************************************************
*
*	Stop the acquisition and start it again
*
****************************************************************************/

iSYSResult stopAndStart() {
	/* stop measurement */
	cout << "stop measurement.. \n";
	res = iSYS_StopAcquisition(pHandle, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	cout << "wait for 500ms.. \n";
	Sleep(500);

	/* start measurement */
	cout << "start measurement.. \n";
	res = iSYS_StartAcquisition(pHandle, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not start measurement!\n";
	}
	return res;
}

iSYSResult stopAcquisition()
{
	/* stop measurement */


	cout << "stop measurement.. \n";
	res = iSYS_StopAcquisition(pHandle, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	cout << "wait for 500ms.. \n";
	Sleep(500);

	spiClose();

	return res;
}


iSYSResult startAcquisition()
{
	spiInit();

	res = stopAndStart();
	return res;
}

void notify(iSYSTargetList_t *targetList, char desc[], bool print) {
	string buff;
	uint16_t items = 0;
	formatTheList(targetList, "this is a new one", buff, items);
	if (print) {
		printTheList("listprint.txt", targetList, "a", desc, buff);
	}
	if (buff.length() > 0) {
		string complete = string("[");
		complete.append(buff);
		complete.append("]");
		//async_publish((char *)cfg->getmqttBroker().c_str(), (char *)cfg->getmqttClient().c_str(), "test/fromradar", (char *)complete.c_str(), "test/radarlastwill", 1);
		//cout << complete.c_str() << endl;
	}
	
	buff.clear();

	formatMQTTMessage(targetList, "no message", buff, items);

	if (buff.length() > 0) {
		
		string topic = cfg->getmqttTopic();
		topic.append("/targets");
		string lastwill = cfg->getmqttTopic();
		lastwill.append("/lastwill");
		string complete = string("[");
		complete.append(buff);
		complete.append("]");
		async_publish((char *)cfg->getmqttBroker().c_str(), (char *)cfg->getmqttClient().c_str()
			, (char *)topic.c_str(), (char *)complete.c_str(), (char *)lastwill.c_str(), 1);
		cout << complete.c_str() << endl;
	}

}
/*
void snap_a_picture(SerialCamera *camera, string filestring)
{
	if (nullptr != camera) {
		if (camera->isOpen()) {
			string str = filestring;
			str.append("ABC");
			str.append(".PNG");
			camera->snap((char *)str.c_str());
		}
	}
}
*/

/******************************************************************
*
*	Check for messages from the radar and input from the keyboard
*
*******************************************************************/
iSYSResult_t processRadar(Buf_t *buf) {
	iSYSTargetList_t *targetList = nullptr;

	char desc[128] = { 0 };
	int descindex = 0;
	string filestring = string("image");
	uint16_t rcvdSize = 0;
	std::string::size_type sz;   // alias of size_t

	while (running)
	{
		int notifyCycle = std::stoi(cfg->getNotify(), &sz);
		char * testinput = "";
		vector<unsigned char> msgBuf;
		char in = 0x00;
		bool snapshot = false;
		// get a new one
		if (nullptr == targetList)	targetList = new iSYSTargetList();

		if (spiCheck(msgBuf))
		{

			rcvdSize = (uint16_t)msgBuf.size();
			if (rcvdSize > 0) {
				unsigned char *char_ptr = nullptr;
				uint32_t *pRead_data = (uint32_t *)msgBuf.data();
				char_ptr = (unsigned char *)pRead_data;

				// create a target list
				iSYSResult res = decodeMessage(char_ptr, rcvdSize, targetList);
				if (ERR_OK != res) {
					fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[res]);
					printHex(char_ptr, rcvdSize);
					msgBuf.clear();

					spiClose();

					spiInit();

					// stop and start the radar unit
					res = stopAndStart();

				}
			}

		}

		// keyboard input
		if (_kbhit())
		{
			in = _getch();
			switch (in)
			{
				// carriage return
			case '\r':
				printf("\n");
				if (nullptr != targetList && targetList->nrOfTargets > 0) {
					printf("save the target list\n");
					desc[descindex] = '\0';

					notify(targetList, desc, true);

					//snap_a_picture(camera, filestring);

					memset(desc, 0, sizeof desc);
					descindex = 0;
				}
				if (nullptr != targetList) delete(targetList);
				targetList = nullptr;
				break;

				// backspace
			case 0x08:
			{
				printf("\b \b");
				desc[descindex != 0 ? descindex-- : descindex] = '\0';
				break;
			}

			// exit
			case '-':
			{
				if (nullptr != targetList) delete(targetList);
				targetList = nullptr;
				return ERR_OK;
			}

			// put it in the description string
			default:
				if (in != '\0') {
					desc[descindex++] = in;
					cout << in;
				}
			}
		}

		if (notify_interval++ > notifyCycle && nullptr != targetList && targetList->nrOfTargets > 0)
		{
			// publish but don't save it to the file
			notify(targetList, "automated\n", false);
			notify_interval = 0;
		}
		pthread_mutex_unlock(&buf->webgo);

		if (start_acquisition) {
			start_acquisition = false;
			cout << "got the indicator to start" << endl;
			startAcquisition();
		}
		if (stop_acquisition) {
			if (nullptr != targetList) {
				delete(targetList);
				targetList = nullptr;
			}
			stop_acquisition = false;
			cout << "got the indicator to stop" << endl;
			stopAcquisition();
		}
		// cycle time
		Sleep(50);

	}
	return ERR_OK;
}

/************************************************************
*
*	Shutdown the radar unit by stopping the acquisition
*	and exit the system
*
*************************************************************
*/
iSYSResult closeRadar() {
	/* stop measurement */
	cout << "stop measurement.. \n";
	if (nullptr == pHandle) return ERR_HANDLE_NOT_INITIALIZED;
	res = iSYS_StopAcquisition(pHandle, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n\n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	cout << "wait for 500ms.. \n";
	Sleep(500);
	/* exit system with deviceAddress */
	res = iSYS_exitSystem(pHandle, deviceAddress);
	if (res != ERR_OK) {
		system("cls");
		cout << "Error: " << ERROR_CODES[res] << "\n";
		cout << "could not exit system";
	}

	/* Close serialport connection */
	res = iSYS_exitComPort(pHandle);
	if (res == ERR_OK)
	{
		cout << "\n\n---------------------------------";
		cout << "\n  handle finished";
		cout << "\n---------------------------------";
	}
	else
	{
		system("cls");
		cout << "Error: " << ERROR_CODES[res] << "\n";
		cout << "could not exit system";
	}

	return res;
}

/********************************************************************************
*
*	Checks for the start sequence [0xFEED5010] in a target list message
*
**********************************************************************************/
bool list_start(uint8_t msg_check[], uint8_t &entries)
{
	if (msg_check[0] == 0xFE && msg_check[1] == 0xED && msg_check[2] == 0x50 && msg_check[3] == 0x10) {
		entries = msg_check[7];
		return true;
	}
	return false;
}

/************************************************************************
*
*	Checks for the end sequence [0xFEED0000] in a target list message
*
*************************************************************************/
bool list_end(uint8_t msg_check[]) {
	if (msg_check[0] == 0xFE && msg_check[1] == 0xED && msg_check[2] == 0x00 && msg_check[3] == 0x00) return true;
	return false;
}

/***********************************************************************
Function: decodes Message received from iSYS device.
Input arguments:
	Frame array:  array with from iSYS received target list message
	rcvdSize: number of bytes in the frame array
	targetList: struct with decoded target list
Return value:
	ErrorCode

***********************************************************************/
iSYSResult decodeMessage(unsigned char * msg, uint16_t msgSize, iSYSTargetList_t *targetList)
{
	uint8_t * frames_start = nullptr;
	uint16_t frames_length = 0;
	uint8_t listEntries = 0;
	uint8_t msgState = LIST_BEGIN;

	for (int i = 0; i < msgSize; i++)
	{
		switch (msgState) {
		case LIST_BEGIN:
		{
			if (msgSize > 3)
			{
				if (!list_start((uint8_t *)(msg + i), listEntries))
				{
					printf("no idea what's coming in\n");
					msgState = LIST_ERROR;
					// initRadar();
					stopAndStart();
				}
				else
				{
					msgState = LIST_PROCESSING;
					i += 7;
					frames_start = (uint8_t *)(msg + i + 1);
					frames_length = 0;
				}
			}
			else
			{
				fprintf(stderr, "List buffer received %d bytes - must be 3 or greater\n", msgSize);
				msgState = LIST_ERROR;
			}
			break;
		}
		case LIST_PROCESSING:
		{
			if (!list_end((uint8_t *)(msg + i))) {
				i += 23;
				frames_length += 24;
			}
			else {
				// end of frame
				if (frames_start == nullptr) {
					printf("never set the frames_start pointer can't decode rcvd: %d i:%d frames_length:%d\n", msgSize, i, frames_length);
					//printHex(msg, i);
					msgState = LIST_ERROR;
					break;
				}
				// populate the target list
				iSYSResult res = decodeTargetFrame(frames_start
					, frames_length
					, 5010
					, 32
					, listEntries
					, targetList);
				if (ERR_OK != res)
				{
					printf("error from decode target %s \n", ERROR_CODES[res]);
					msgState = LIST_ERROR;
					break;
				}
				// process any additional
				msgState = LIST_BEGIN;
				i += 7;
			}
			break;
		}
		case LIST_ERROR:
			msgState = LIST_BEGIN;
			printf("received list error msgSize: %d i:%d frames_length:%d\n", msgSize, i, frames_length);
			return ERR_COMMAND_NO_VALID_FRAME_FOUND;
		}
	}
	return ERR_OK;
}


/***********************************************************************
Function: decodes target List frame received from iSYS device.
Input arguments:
	Frame array:  array with from iSYS received target list frame
	nrOfElements: number of bytes in the frame array
	productcode: product code of the connected iSYS (e.g. 6003, 4001, …)
	bitrate: resolution of the target list in the frame array (16-Bit or 32-Bit)
	argetList: struct for decoded target list Output arguments:
	targetList: struct with decoded target list
Return value:
	ErrorCode

***********************************************************************/
iSYSResult_t decodeTargetFrame(uint8_t *pData
	, uint16_t nrOfElements
	, uint16_t productcode
	, uint8_t bitrate
	, uint8_t nrOfTargets
	, iSYSTargetList_t *targetList)
{
	uint8_t output_number = 0;

	if (nullptr == pData || nullptr == targetList)
	{
		return ERR_COMMAND_NO_VALID_FRAME_FOUND;
	}
	/* check for valid amount of targets */
	if ((nrOfTargets > MAX_TARGETS) && (nrOfTargets != 0xff))
	{
		printf("too many targets %d  max: %d\n", nrOfTargets, MAX_TARGETS);
		return ERR_COMMAND_MAX_DATA_OVERFLOW;
	}
	// printHex(pData, nrOfElements);
	targetList->nrOfTargets = nrOfTargets;
	targetList->clippingFlag = 0;
	targetList->outputNumber = output_number;
	if (nrOfTargets != 0xff) { //0xff clipping
		int i = 0;
		for (int j = 0; j < nrOfTargets; j++)
		{
			targetList->targets[j].signal = unpackFloat(pData + i, &i);
			targetList->targets[j].range = unpackFloat(pData + i, &i);
			targetList->targets[j].velocity = unpackFloat(pData + i, &i);
			targetList->targets[j].angle = unpackFloat(pData + i, &i);
			targetList->targets[j].reserved1 = unpackFloat(pData + i, &i);
			targetList->targets[j].reserved2 = unpackFloat(pData + i, &i);
			chrono::high_resolution_clock m_clock;

			targetList->targets[j].timestamp = chrono::duration_cast<chrono::milliseconds>
				(m_clock.now().time_since_epoch()).count();
		}
	}

	if (nrOfTargets == MAX_TARGETS) {
		targetList->error.iSYSTargetListError = TARGET_LIST_FULL;
	}
	else {
		targetList->error.iSYSTargetListError = TARGET_LIST_OK;
	}
	return ERR_OK;
}


/******************************************************************
*
*
*	functions for using the target list
*
*
*******************************************************************/
#define float32_t float

float unpackFloat(const void *buf, int *i) {
	const unsigned char *b = (const unsigned char *)buf;

	uint32_t temp = 0;
	*i += 4;
	temp = ((b[0]) << 24 |
		(b[1] << 16) |
		(b[2] << 8) |
		b[3]);
	float f = *((float *)&temp);
	return f;
}


void printHex(uint8_t *buf, uint16_t size)
{
	int j = 0;

	cout << "buff size " << size << endl;
	for (int i = 0; i < 16; i++)
		//	for (int i = 0; i < size; i++)
	{
		if (j == 0) printf("%d -- ", i);
		printf("%02X ", *(buf + i));
		if (j++ > 6) {
			printf("\n");
			j = 0;
		}
	}
	printf("\n\n");
}

void sortRange(iSYSTarget_t *targetList, int number);

void formatMQTTMessage(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items) {
	char buffx[128];
	char buffy[128];
	char buffangle[128];
	char buffrange[128];
	char buffvelocity[128];
	json j;
	json j_object;
	bool firsttime = true;

	// serialize without indentation
	for (int i = 0; i < targetList->nrOfTargets; i++) 
	{
		if (targetList->targets[i].range < 10.0 &&
			((targetList->targets[i].velocity > 0.01 ||
			(targetList->targets[i].velocity < -0.01))))
		{
			if (!firsttime) {
				sb.append(",");
			}
			items++;
			firsttime = false;
			std::string::size_type sz;   // alias of size_t
			float xfloat = std::stof(cfg->getx(), &sz);
			float yfloat = std::stof(cfg->gety(), &sz);
			float adjAngle = std::stof(cfg->getAngle());
			float a = targetList->targets[i].angle;
			float r = targetList->targets[i].range;
			float v = targetList->targets[i].velocity;
			float xcoord = (r * cos(a)) + xfloat;
			float ycoord = (r * sin(a)) + yfloat;
			memset(buffx, 0, sizeof(buffx));
			snprintf(buffx, sizeof(buffx), "%.1f", xcoord);
			memset(buffy, 0, sizeof(buffy));
			snprintf(buffy, sizeof(buffy), "%.1f", ycoord);
			memset(buffrange, 0, sizeof(buffrange));
			snprintf(buffrange, sizeof(buffrange), "%.1f", r);
			memset(buffangle, 0, sizeof(buffangle));
			snprintf(buffangle, sizeof(buffangle), "%.1f", a);
			memset(buffvelocity, 0, sizeof(buffvelocity));
			snprintf(buffvelocity, sizeof(buffvelocity), "%.1f", v);
			sb.append("{\"x\":");
			sb.append(buffx);
			sb.append(",");
			sb.append("\"y\":");
			sb.append(buffy);
			sb.append(",");
			sb.append("\"r\":");
			sb.append(buffrange);
			sb.append(",");
			sb.append("\"a\":");
			sb.append(buffangle);
			sb.append(",");
			sb.append("\"v\":");
			sb.append(buffvelocity);
			sb.append(",");
			time_t now;
			time(&now);
			char buf[sizeof "2011-10-08T07:07:09Z"];
			strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
			// this will work too, if your compiler doesn't support %F or %T:
			//strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
			sb.append("\"time\":\"");
			sb.append(buf);
			sb.append("\"");
			sb.append("}");
		}
		else {
			continue;
		}
	}
}
void formatTheList(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items) {
	bool firsttime = true;


	//sortRange(targetList->targets, targetList->nrOfTargets);

	for (int i = 0; i < targetList->nrOfTargets; i++) {
		char buff[128];

		//		if (!(targetList->targets[i].velocity > 5.0) &&
		//			targetList->targets[i].velocity > -5.0 &&
		//			targetList->targets[i].range < 10.0)
		//		{
/*		if (((targetList->targets[i].velocity > 0.0001) ||
			(targetList->targets[i].velocity < -0.0001)) &&
			(targetList->targets[i].angle < 30.0) &&
			(targetList->targets[i].range > 2.0) &&
			targetList->targets[i].range < 10.0)
		{
			//if (targetList->targets[i].range < 10.0) {
*/			if (!firsttime) {
	sb.append(",");
}
		items++;
		firsttime = false;
		int size = snprintf(buff, sizeof(buff), "{ \"s\":%.1f ,\"r\":%.1f ,\"v\":%.2f ,\"a\":%.1f ,\"r\":%f ,\"r2\":%f ,\"ts\":%" PRIu64 "}\n"
			, targetList->targets[i].signal
			, targetList->targets[i].range
			, targetList->targets[i].velocity
			, targetList->targets[i].angle
			, targetList->targets[i].reserved1
			, targetList->targets[i].reserved2
			, targetList->targets[i].timestamp);
		sb.append(buff, size);
		//		}

	}

}

void printTheList(char *filename, iSYSTargetList *targetList, char *mode, char *description, string data) {
	FILE *fp = fopen(filename, mode);
	if (nullptr == targetList) {
		fprintf(fp, "%s", data.c_str());
	}
	else {
		fprintf(fp, "\ndescription %s\n\n", description);
		fprintf(fp, "%s\n", data.c_str());
		//sortRange(targetList->targets, targetList->nrOfTargets);
	}
	printf("saved %d bytes\n", data.size());


	// sortRange(targetList);
	fclose(fp);
}

bool myfunction(int i, int j) { return (i < j); }

struct myclass {
	bool operator() (int i, int j) { return (i < j); }
} myobject;

void sortRange(iSYSTarget_t *targets, int number) {

	//std::vector<iSYSTarget_t> mytargets(targetList->targets, targetList->nrOfTargets);
	int x = sizeof(float);
	float32_t *array = (float32_t *)calloc(number, x);

	for (int i = 0; i < number; i++) {
		*(i + array) = targets[i].range;
	}
	vector<float32_t> vect;
	for (int i = 0; i < number; i++) {
		vect.push_back(*(array + i));
	}

	// using default comparison (operator <):
	std::sort(vect.begin(), vect.begin() + number);           //(12 32 45 71)26 80 53 33

	// print out content:
	std::cout << "myvector contains:";
	for (std::vector<float>::iterator it = vect.begin(); it != vect.end(); ++it)
		std::cout << ' ' << *it;
	std::cout << '\n';
	// using function as comp
//	std::sort(myvector.begin() + 4, myvector.end(), myfunction); // 12 32 45 71(26 33 53 80)
	free(array);
	// using object as comp
//	std::sort(myvector.begin(), myvector.end(), myobject);     //(12 26 32 33 45 53 71 80)
}

iSYSResult processMessage(vector<unsigned char> msgBuf);

/* The consumer thread */
void *serialConsumer(void  *pbuf)
{
	Buf_t *buf = (Buf_t *)pbuf;
	/* check to see if any buffers are filled or if the done flag is set */
	while (1) {

		pthread_cond_wait(&buf->adddata, &buf->buflock);

		if (!buf->available) {
			pthread_mutex_unlock(&buf->buflock);

		}
		else {

			processMessage(*buf->msgBufs[buf->nextrem]);

			printf("got the data");
			buf->available = false;
		}
		/* signal the producer that a buffer is empty */
		pthread_cond_signal(&buf->remdata);

		/* release the mutex */
		pthread_mutex_unlock(&buf->buflock);
	}

	/* exit the thread */
	pthread_exit((void *)0);
	return nullptr;
}

iSYSTargetList *targetList = nullptr;
iSYSResult processMessage(vector<unsigned char> msgBuf) {


	uint16_t rcvdSize = (uint16_t)msgBuf.size();
	unsigned char *char_ptr = nullptr;
	uint32_t *pRead_data = (uint32_t *)msgBuf.data();
	char_ptr = (unsigned char *)pRead_data;
	if (nullptr == targetList)	targetList = new iSYSTargetList();

	// create a target list
	iSYSResult res = decodeMessage(char_ptr, rcvdSize, targetList);
	if (ERR_OK != res) {
		fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[res]);
		printHex(char_ptr, rcvdSize);
		msgBuf.clear();

		spiClose();

		spiInit();

		// stop and start the radar unit
		res = stopAndStart();

	}
	return res;
}

void targetToJSON(json j) {

}