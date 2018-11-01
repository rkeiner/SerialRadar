#include "Config.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

Config::Config(char * filename)
{
	if (doesFileExist(filename)) {
		if (populate(filename)) {
			populated = true;
		}
	}
	else {
		populated = false;
		fconfig = fopen(filename, "w");
		printf("file %s didn't exist", filename);
		confile_corrupt = false;
	}
}


Config::~Config()
{

}

void Config::createDefault(char *filename, int comport) {
	populated = false;
	fconfig = fopen(filename, "w");

}


int Config::doesFileExist(const char *filename) {
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}
bool	Config::isPopulated() {
	return populated;
}
/*
	Getters
*/
string		Config::getcomPort() {
	return comPort;
}
string		Config::getmqttBroker() {
	return mqttBroker;
}
string	Config::getmqttPort() {
	return mqttPort;
}
string		Config::getmqttClient() {
	return mqttClient;
}

string		Config::getmqttPassword() {
	return mqttPassword;

}
string		Config::getmqttTopic() {
	return mqttTopic;

}

string Config::getDeviceName() {
	return entries.DeviceName;
}
string 		Config::getDeviceAddress() {
	return entries.DeviceAddress;
}
string 		Config::getFrequencyChannel() {
	return entries.FrequencyChannel;
}
string Config::getx() {
	return entries.x;
}
string 		Config::gety() {
	return entries.y;
}
string 		Config::getAngle() {
	return entries.angle;
}

string 		Config::getThresholdMovingTargetsNearRangeMargin() {
	return entries.ThresholdMovingTargetsNearRangeMargin;
}
string 		Config::getThresholdMovingTargetsMainRangeMargin() {
	return entries.ThresholdMovingTargetsMainRangeMargin;
}
string 		Config::getThresholdMovingTargetsLongRangeMargin() {
	return entries.ThresholdMovingTargetsLongRangeMargin;
}
string 		Config::getFirmwareVersion() {
	return entries.FirmwareVersion;
}
string 		Config::getSerialNumber() {
	return entries.SerialNumber;
}
string 		Config::getDspHardwareVersion() {
	return entries.DspHardwareVersion;
}
string 		Config::getRfeHardwareVersion() {
	return entries.RfeHardwareVersion;
}
string 		Config::getProductInfo() {
	return entries.ProductInfo;
}
string		Config::getNotify() {
	return notify;
};

bool		Config::isTargetClusteringEnable() {
	return entries.TargetClusteringEnable;
}
bool		Config::isProcessingRcsCalibrationEnable() {
	return entries.ProcessingRcsCalibrationEnable;
}


/*
		Setters
*/

void		Config::setcomPort(string  parm) {
	comPort = parm;
}
void		Config::setmqttBroker(string  parm) {
	mqttBroker = parm;
}
void		Config::setmqttPort(string p) {
	mqttPort = p;

}
void		Config::setNotify(string p) {
	notify = p;

}
void		Config::setmqttClient(string  parm) {
	mqttClient = parm;
}
void		Config::setmqttPassword(string  parm) {
	mqttPassword = parm;
}
void		Config::setmqttTopic(string  parm) {
	mqttTopic = parm;
}

void		Config::setDeviceName(string parm) {
	entries.DeviceName = parm;
}
void 		Config::setDeviceAddress(string parm) {
	entries.DeviceAddress = parm;
}
void		Config::setFrequencyChannel(string  parm) {
	entries.FrequencyChannel = parm;
};
void		Config::setx(string parm) {
	entries.x = parm;
}
void 		Config::sety(string parm) {
	entries.y = parm;
}
void		Config::setAngle(string  parm) {
	entries.angle = parm;
};
void		Config::setThresholdMovingTargetsNearRangeMargin(string  parm) {
	entries.ThresholdMovingTargetsNearRangeMargin = parm;
};
void	Config::setThresholdMovingTargetsMainRangeMargin(string  parm) {
	entries.ThresholdMovingTargetsMainRangeMargin = parm;
};
void		Config::setThresholdMovingTargetsLongRangeMargin(string  parm) {
	entries.ThresholdMovingTargetsLongRangeMargin = parm;
};
void		Config::setFirmwareVersion(string  parm) {
	entries.FirmwareVersion = parm;
};
void		Config::setSerialNumber(string  parm) {
	entries.SerialNumber = parm;
};
void		Config::setDspHardwareVersion(string  parm) {
	entries.DspHardwareVersion = parm;
};
void		Config::setRfeHardwareVersion(string  parm) {
	entries.RfeHardwareVersion = parm;
};
void		Config::setProductInfo(string  parm) {
	entries.ProductInfo = parm;;
};
void		Config::setTargetClusteringEnable(bool parm) {
	entries.TargetClusteringEnable = parm;
}
void		Config::setProcessingRcsCalibrationEnable(bool parm) {
	entries.ProcessingRcsCalibrationEnable = parm;
}

bool Config::isConfFileCorrupt() {
	return confile_corrupt;
}

bool Config::populate(char  *filename) {
	std::ifstream i(filename);
	try {
		json j;

		if (EOF == i.peek()) {
			cout << "config file empty ---- exiting -------" << endl;
			i.close();
			if (remove(filename) != 0) {
				cout << "Error deleting " << filename << endl;
			} 
			confile_corrupt = true;
			return false; // nothing in the configuration
		}
		confile_corrupt = false;
		try {
			i >> j;
			fromJSON(j);
		}
		catch (exception exc) {
			cout << "JSON exception  " << exc.what() << endl;
		}
	}
	catch (exception err) {
		cout << "JSON exception " << err.what() << endl;
		return false;
	}
	return true;
}

bool 	Config::fromJSON(json j) {
	this->comPort = j["comport"].get<std::string>();
	this->mqttBroker = j["mqttbroker"].get<std::string>();
	this->mqttPort = j["mqttport"].get<std::string>();
	this->mqttClient = j["mqttclient"].get<std::string>();
	this->mqttPassword = j["mqttpassword"].get<std::string>();
	this->mqttTopic = j["mqtttopic"].get<std::string>();
	this->notify = j["notify"].get<std::string>();

	this->entries.FrequencyChannel = j["FrequencyChannel"].get<std::string>();
	this->entries.x = j["x"].get<std::string>();
	this->entries.y = j["y"].get<std::string>();
	this->entries.angle = j["Angle"].get<std::string>();

	this->entries.ThresholdMovingTargetsNearRangeMargin = j["ThresholdMovingTargetsNearRangeMargin"].get<std::string>();

	this->entries.ThresholdMovingTargetsMainRangeMargin = j["ThresholdMovingTargetsMainRangeMargin"].get<std::string>();

	this->entries.ThresholdMovingTargetsLongRangeMargin = j["ThresholdMovingTargetsLongRangeMargin"].get<std::string>();

	this->entries.DeviceName = j["DeviceName"].get<std::string>();

	this->entries.DeviceAddress = j["DeviceAddress"].get<std::string>();

	/* get firmware version */
	//cfg->setFirmwareVersion(major << "." << minor);
	//j["FirmwareVersion"] = cfg->getFirmwareVersion();

	/* get product info */

	this->entries.ProductInfo = j["ProductInfo"].get<std::string>();

	/* get serialnumber */

	this->entries.SerialNumber = j["SerialNumber"].get<std::string>();

	/* get DSP hw version from sensor */

	this->entries.DspHardwareVersion = j["DspHardwareVersion"].get<std::string>();

	/* get RFE hw version from sensor */

	this->entries.RfeHardwareVersion = j["RfeHardwareVersion"].get<std::string>();

	return true;
}

bool 	Config::toJSON(json &j) {

	j["comport"] = this->comPort;
	j["mqttbroker"] = this->mqttBroker;
	j["mqttport"] = this->mqttPort;
	j["mqttclient"] = this->mqttClient;
	j["mqttpassword"] = this->mqttPassword;
	j["mqtttopic"] = this->mqttTopic;
	j["FrequencyChannel"] = this->entries.FrequencyChannel;
	j["notify"] = this->notify;
	j["x"] = this->entries.x;

	j["y"] = this->entries.y;

	j["Angle"] = this->entries.angle;


	j["ThresholdMovingTargetsNearRangeMargin"] = this->entries.ThresholdMovingTargetsNearRangeMargin;

	j["ThresholdMovingTargetsMainRangeMargin"] = this->entries.ThresholdMovingTargetsMainRangeMargin;

	j["ThresholdMovingTargetsLongRangeMargin"] = this->entries.ThresholdMovingTargetsLongRangeMargin;

	j["DeviceName"] = this->entries.DeviceName;

	j["DeviceAddress"] = this->entries.DeviceAddress;

	/* get firmware version */
	//cfg->setFirmwareVersion(major << "." << minor);
	//j["FirmwareVersion"] = cfg->getFirmwareVersion();

	/* get product info */
	j["ProductInfo"] = this->entries.ProductInfo;

	/* get serialnumber */

	j["SerialNumber"] = this->entries.SerialNumber;

	/* get DSP hw version from sensor */

	j["DspHardwareVersion"] = this->entries.DspHardwareVersion;

	/* get RFE hw version from sensor */

	j["RfeHardwareVersion"] = this->entries.RfeHardwareVersion;

	return true;
}

void Config::webFormPrint(string &webform) {

	webform.append("<h1>Server Config</h1>");
	webform.append("<form action = \"ab/config\" method=\"get\">");

	webform.append("<table style = \"width: 300px;\" border=\"black\">"
		"<tbody>");

	webform.append("<td>Comm Port<td>"
		"<input type=\"text\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->comPort);
	webform.append("\" name=\"comport\"/>");
	webform.append("<tr>");
	webform.append("<td>MQTT Broker<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \" "
		"value=\"");
	webform.append(this->mqttBroker);
	webform.append("\" name=\"mqttbroker\"/>");
	webform.append("<tr>");
	webform.append("<td>Port<td>"
		"<input type=\"text\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttPort);
	webform.append("\" name=\"mqttport\"/>");
	webform.append("<tr>");
	webform.append("<td>Client<td>"
		"<input type=\"text\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttClient);
	webform.append("\" name=\"mqttclient\"/>");
	webform.append("<tr>");
	webform.append("<td>Password<td>"
		"<input type=\"password\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttPassword);
	webform.append("\" name=\"mqttpassword\"/>");
	webform.append("<tr>");
	webform.append("<td>Topic<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttTopic);
	webform.append("\" name=\"mqtttopic\"/>");

	webform.append("</tbody></table>");
	webform.append("<h2>Sensor Config</h2>");

	webform.append("<br>");

	webform.append("<table border=\"black\">"
		"<tbody>");

	webform.append("<td>"
		"DeviceName</td><td><span style=\"color: #0000ff; \"/>");
	webform.append(this->entries.DeviceName.c_str());
	webform.append("</td>");

	webform.append("<tr>"
		"<td>");
	webform.append("DeviceAddress<td>");
	webform.append(this->entries.DeviceAddress.c_str());
	webform.append("</td>");

	/* get product info */
	webform.append("<tr>"
		"<td>");
	webform.append("ProductInfo</td><td>");
	webform.append(this->entries.ProductInfo.c_str());
	webform.append("</td>");

	/* get serialnumber */
	webform.append("<tr>"
		"<td>");
	webform.append("SerialNumber</td><td>");
	webform.append(this->entries.SerialNumber.c_str());
	webform.append("</td>");
	/* get DSP hw version from sensor */

	webform.append("<tr>"
		"<td>");
	webform.append("DspHardwareVersion </td>"
		"<td>");
	webform.append(this->entries.DspHardwareVersion.c_str());
	webform.append("</td>");

	/* get RFE hw version from sensor */

	webform.append("<tr>"

		"<td>");
	webform.append("RfeHardwareVersion </td><td>");
	webform.append(this->entries.RfeHardwareVersion.c_str());
	webform.append("</td>");


	webform.append("<tr>"
		"<td>");

	webform.append("x</td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->entries.x.c_str());
	webform.append("\" name=\"");
	webform.append("x");
	webform.append("\">");

	webform.append("</td>");


	webform.append("<tr>"
		"<td>");

	webform.append("y</td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->entries.y.c_str());
	webform.append("\" name=\"");
	webform.append("y");
	webform.append("\">");

	webform.append("</td>");

	webform.append("<tr>"
		"<td>");

	webform.append("angle</td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->entries.angle.c_str());
	webform.append("\" name=\"");
	webform.append("angle");
	webform.append("\">");

	webform.append("</td>");

	webform.append("<tr>"
		"<td>");

	webform.append("Frequency Channel </td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->entries.FrequencyChannel.c_str());
	webform.append("\" name=\"");
	webform.append("FrequencyChannel");
	webform.append("\">");

	webform.append("</td>");
	webform.append("<tr>"
		"<td>");
	webform.append("Notify * 50ms</td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->notify.c_str());
	webform.append("\" name=\"");
	webform.append("notify");
	webform.append("\">");

	webform.append("</td>");
	/*
	webform.append("<tr>"
		"<td>");
	webform.append("TargetsNearRangeMargin </td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->entries.ThresholdMovingTargetsNearRangeMargin.c_str());
	webform.append("\" name=\"");
	webform.append("TargetsNearRangeMargin");
	webform.append("\">");

	webform.append("</td>");

	webform.append("<tr>"

		"<td>");
	webform.append("TargetsMainRangeMargin </td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \" size=5 value=\"");
	webform.append(this->entries.ThresholdMovingTargetsMainRangeMargin.c_str());
	webform.append("\" name=\"");
	webform.append("TargetsMainRangeMargin");
	webform.append("\">");

	webform.append("</td>");

	webform.append("<tr>"

		"<td>");
	webform.append("TargetsLongRangeMargin </td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->entries.ThresholdMovingTargetsLongRangeMargin.c_str());
	webform.append("\" name=\"");
	webform.append("TargetsLongRangeMargin");
	webform.append("\">");
	webform.append("</td>");
	*/
	// webform.append("<tr>");
/*
	webform.append("<td>&nbsp;"
		"<input type = \"checkbox\" name = \"setProcessingRcsCalibrationEnable\""
		"value = \"checkcheck box\"");
	if (this->isProcessingRcsCalibrationEnable()) {
		webform.append(" checked");
	}
	else {
		webform.append("\"");
	}
	webform.append(">RcsCalibrationEnable</td>");

	webform.append("<td>&nbsp;"
		"<input type = \"checkbox\" name = \"setProcessingClusteringEnable\""
		"value = \"checkcheck box\"");
	if (this->isTargetClusteringEnable()) {
		webform.append("checked");
	}
	webform.append(">ClusteringEnable</td>");
	*/
	webform.append("</span>");
	webform.append("</tbody>"
		"</table>");

	webform.append(
		"<input type = \"submit\" value = \"Save Config\" name = \"config\" />"
		"<br><br>"
		"</form>"

		"<form action = \"start\" method=\"get\">"
		"<input type = \"submit\" value = \"StartAcquisition\" param = \"start\" />"
		"</form>"

		"<form action = \"stop\" method=\"get\">"
		"<input type = \"submit\" value = \"StopAcquisition\" param = \"stop\"/>"
		"</form>");


}

bool Config::webFormProcess(struct mg_connection *conn, string &s) {

	if (CivetServer::getParam(conn, "comport", s)) {
		this->setcomPort(s);
	}
	if (CivetServer::getParam(conn, "mqttbroker", s)) {
		this->setmqttBroker(s);
	}	
	if (CivetServer::getParam(conn, "mqttport", s)) {
		this->setmqttPort(s);
	}
	if (CivetServer::getParam(conn, "mqttclient", s)) {
		this->setmqttClient(s);
	}
	if (CivetServer::getParam(conn, "mqttpassword", s)) {
		this->setmqttPassword(s);
	}
	if (CivetServer::getParam(conn, "mqtttopic", s)) {
		this->setmqttTopic(s);
	}

	if (CivetServer::getParam(conn, "x", s)) {
		this->setx(s);
	}
	if (CivetServer::getParam(conn, "y", s)) {
		this->sety(s);
	}
	if (CivetServer::getParam(conn, "angle", s)) {
		this->setAngle(s);
	}

	if (CivetServer::getParam(conn, "FrequencyChannel", s)) {
		this->setFrequencyChannel(s);
	}
	if (CivetServer::getParam(conn, "notify", s)) {
		this->setNotify(s);
	}
	if (CivetServer::getParam(conn, "TargetsMainRangeMargin", s)) {
		this->setThresholdMovingTargetsMainRangeMargin(s);
	}
	if (CivetServer::getParam(conn, "TargetsNearRangeMargin", s)) {
		this->setThresholdMovingTargetsNearRangeMargin(s);
	}
	if (CivetServer::getParam(conn, "TargetsLongRangeMargin", s)) {
		this->setThresholdMovingTargetsLongRangeMargin(s);
	}
	/*
	if (CivetServer::getParam(conn, "setProcessingRcsCalibrationEnable", s)) {
		bool checked = s._Equal("enable") ? true : false;
		this->setProcessingRcsCalibrationEnable(checked);
	}
	if (CivetServer::getParam(conn, "setProcessingClusteringEnable", s)) {
		bool checked = s._Equal("enable") ? true : false;
		this->setTargetClusteringEnable(checked);
	}
	*/



	return true;
}

