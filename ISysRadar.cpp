#include "ISysRadar.h"

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



using namespace nlohmann;



ISysRadar::ISysRadar()
{
}


ISysRadar::~ISysRadar()
{
}

iSYSResult ISysRadar::init()
{
	return ERR_OK;
}

iSYSResult ISysRadar::run()
{
	return ERR_OK;
}
iSYSResult ISysRadar::start()
{
	return ERR_OK;
}
iSYSResult ISysRadar::stop() 
{
	return ERR_OK;
}





bool ISysRadar::spiCheck(vector<unsigned char> &) {
	return true;
}


void ISysRadar::intHandler(int dummy = 0) {
	printf("good bye\n");
	running = false;
}



void ISysRadar::processParms(Buf_t *buf) {

	for (int i = 0; i < buf->argc; i++) {
		char **args = buf->args;
		cout << "parm serial radar " << args[i] << endl;
		if (0 == strcmp(args[i], "--mqtt")) {
			cout << " got mqtt " << endl;
			if (i + 1 < buf->argc) {
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
			if (i + 1 < buf->argc) {
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
			if (i + 1 < buf->argc) {
				cout << " config val:" << args[i + 1] << endl;
				mqtt_client = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add a config file" << endl;
			}
		}
	}
}
void *ISysRadar::serialRadar(void  *pbuf)
{
	//signal(SIGINT, intHandler);

	processParms((Buf_t *)pbuf);

	//camera = new SerialCamera();
	//camera->init();
	//printf("should have started the camera\n");

	res = initRadar();
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


extern Config *cfg;

iSYSResult_t ISysRadar::getRadarConfig(char *file, bool update) {
	float32_t version;
	uint16_t major, fix, minor;
	uint16_t productInfo;
	sint16_t margin = 0;
	char buf[32];
	json j;
	cfg = new Config(nullptr == file ? "config.conf" : file);
	FILE *fp;
	fp = fopen("Config.conf", "w");
	uint32_t serialNumber;
	char deviceName[21];

	res = iSYS_getApiVersion(&version);

	cout << "---------------------------------\n";
	cout << "    serial radarAPI Version: ---- " << version << "\n";
	cout << "---------------------------------\n\n";

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
	cfg->setThresholdMovingTargetsNearRangeMargin(buf);


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
	cfg->setThresholdMovingTargetsMainRangeMargin(buf);

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
	cfg->setThresholdMovingTargetsLongRangeMargin(buf);



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
	json j1;
	cfg->toJSON(j1);

	string print = j1.dump(4);
	fprintf(fp, print.c_str());
	cout << print.c_str() << endl;
	return res;
}
int ISysRadar::doesFileExist(const char *filename) {
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}

iSYSResult_t ISysRadar::initRadar()
{
	/* variables */
	FILE *fconfig = nullptr;
	iSYSResult_t res;
	int comport;

	char * configfile = "config.conf";
	saveLocation = ISYS_LOCATION_RAM;
	cout << "file " << configfile;
	if (doesFileExist("config.conf")) {
		cout << " exists";
	}
	else {
		cout << " does not exist";
	}
	cout << endl;
	// spiUSB(nullptr);

	comport = 8;
	cout << "Please enter the serial port number COMX:";
	//cin >> comport;
	cout << "Please enter the device address (default 400X = 128, 5XXX = 100, 600X = 100):";
	deviceAddress = 100;
	//cin >> deviceAddress;

	/* initialize com port */
	cout << "\ninitializing COM port.. " << comport << "\n";
	res = iSYS_initComPort(&pHandle, static_cast<uint8_t>(comport), ISYS_BAUDRATE_115200);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not initialize serial port -- program closes automatically";
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

	/******************************************/
	/** set and read iSYS-5010 configuration **/
	/******************************************/
	res = getRadarConfig(configfile, true);


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

	res = stopAndStart();

	return res;
}

iSYSResult ISysRadar::startSensorConfig()
{
	/* init system with deviceaddress and timeout 1000ms */
	cout << "initializing sensor.. \n";
	res = iSYS_initSystem(pHandle, deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "could not initialize system -- program closes automatically";
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

	/*	sint16_t margin = 2;
		res = iSYS_setThresholdMovingTargetsNearRangeMargin(pHandle, ISYS_LOCATION_RAM, margin, 100, 1000);
		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "can not set NearRangeMargin! \n";
			return res;
		}

		margin = 3;
		res = iSYS_setThresholdMovingTargetsMainRangeMargin(pHandle, ISYS_LOCATION_RAM, margin, 100, 1000);
		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "can not set MainRangeMargin! \n";
			return res;
		}

		margin = 5;
		res = iSYS_setThresholdMovingTargetsLongRangeMargin(pHandle, ISYS_LOCATION_RAM, margin, 100, 1000);

		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "can not set LongRangeMargin! \n";
			return res;

		}
	*/
	res = stopAndStart();

	return res;
}
/****************************************************************************
*
*	Stop the acquisition and start it again
*
****************************************************************************/

iSYSResult ISysRadar::stopAndStart() {
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

iSYSResult ISysRadar::stopAcquisition()
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


iSYSResult ISysRadar::startAcquisition()
{
	spiInit();

	res = stopAndStart();
	return res;
}

void ISysRadar::notify(iSYSTargetList_t *targetList, char desc[], bool print) {
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
		async_publish(mqtt_server, mqtt_client, "test/fromradar", (char *)complete.c_str(), "test/radarlastwill", 1);
		cout << complete.c_str() << endl;
	}

	//cout << "send  " << items << " targets of " << targetList->nrOfTargets << " rcvd" << endl;

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
iSYSResult_t ISysRadar::processRadar(Buf_t *buf) {
	iSYSTargetList_t *targetList = nullptr;

	char desc[128] = { 0 };
	int descindex = 0;
	string filestring = string("image");
	uint16_t rcvdSize = 0;
	while (running)
	{
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
		if (notify_interval++ > 10 && nullptr != targetList && targetList->nrOfTargets > 0)
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
iSYSResult ISysRadar::closeRadar() {
	/* stop measurement */
	cout << "stop measurement.. \n";
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
bool ISysRadar::list_start(uint8_t msg_check[], uint8_t &entries)
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
bool ISysRadar::list_end(uint8_t msg_check[]) {
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
iSYSResult ISysRadar::decodeMessage(unsigned char * msg, uint16_t msgSize, iSYSTargetList_t *targetList)
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
iSYSResult_t ISysRadar::decodeTargetFrame(uint8_t *pData
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

float ISysRadar::unpackFloat(const void *buf, int *i) {
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


void ISysRadar::printHex(uint8_t *buf, uint16_t size)
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

void ISysRadar::formatTheList(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items) {
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

void ISysRadar::printTheList(char *filename, iSYSTargetList *targetList, char *mode, char *description, string data) {
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

bool ISysRadar::myfunction(int i, int j) { return (i < j); }


void ISysRadar::sortRange(iSYSTarget_t *targets, int number) {

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
void *ISysRadar::serialConsumer(void  *pbuf)
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

iSYSResult ISysRadar::processMessage(vector<unsigned char> msgBuf) {


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
