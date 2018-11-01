#pragma once
#include <string>
#include "json.hpp"
#include "CivetServer.h"

using namespace nlohmann;
using namespace std;


class Config
{
public:
	Config(char * filename);
	virtual ~Config();

private:
	char * conigFile = "config.conf";
	int doesFileExist(const char *filename);
	char * filename;
	string mqttBroker;
	string mqttPort;
	string mqttClient;
	string mqttPassword;
	string mqttTopic;
	string comPort;
	string notify;
	bool populated;
	bool confile_corrupt;

	struct cofigentries {
		string DeviceName;
		string DeviceAddress;
		string FrequencyChannel;
		string x;
		string y;
		string angle;
		string ThresholdMovingTargetsNearRangeMargin;
		string ThresholdMovingTargetsMainRangeMargin;
		string ThresholdMovingTargetsLongRangeMargin;
		string FirmwareVersion;
		string SerialNumber;
		string DspHardwareVersion;
		string RfeHardwareVersion;
		string ProductInfo;
		bool ProcessingRcsCalibrationEnable;
		bool TargetClusteringEnable;

	} entries;

	FILE *fconfig;

public:
	bool 		toJSON(json &j);
	bool		fromJSON(json j);
	bool		populate(char  *filename);
	bool		isPopulated();
	bool		isConfFileCorrupt();
	string		getcomPort();
	string		getmqttBroker();
	string		getmqttPort();
	string		getmqttClient();
	string		getmqttPassword();
	string		getmqttTopic();
	string		getDeviceName();
	string		getDeviceAddress();
	string		getFrequencyChannel();
	string		getx();
	string		gety();
	string		getAngle();
	string		getThresholdMovingTargetsNearRangeMargin();
	string		getThresholdMovingTargetsMainRangeMargin();
	string		getThresholdMovingTargetsLongRangeMargin();
	string		getFirmwareVersion();
	string		getSerialNumber();
	string		getDspHardwareVersion();
	string		getRfeHardwareVersion();
	string		getProductInfo();
	string		getNotify();
	bool		isTargetClusteringEnable();
	bool		isProcessingRcsCalibrationEnable();

	void		setcomPort(string parm);
	void		setmqttBroker(string  parm);
	void		setmqttPort(string parm);
	void		setmqttClient(string  parm);
	void		setmqttPassword(string  parm);
	void		setmqttTopic(string  parm);


	void		setDeviceName(string parm);
	void 		setDeviceAddress(string parm);
	void 		setFrequencyChannel(string  parm);
	void		setx(string parm);
	void 		sety(string parm);
	void 		setAngle(string  parm);

	void 		setThresholdMovingTargetsNearRangeMargin(string  parm);
	void 		setThresholdMovingTargetsMainRangeMargin(string  parm);
	void 		setThresholdMovingTargetsLongRangeMargin(string  parm);
	void 		setFirmwareVersion(string  parm);
	void 		setSerialNumber(string  parm);
	void 		setDspHardwareVersion(string  parm);
	void 		setRfeHardwareVersion(string  parm);
	void 		setProductInfo(string  parm);
	void		setNotify(string parm);
	void		setTargetClusteringEnable(bool parm);
	void		setProcessingRcsCalibrationEnable(bool parm);

	void		createDefault(char *filename, int comport);
	void		webFormPrint(string &pretty);
	bool		webFormProcess(struct mg_connection *conn, string &s);
};