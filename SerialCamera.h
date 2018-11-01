#include "opencv2/core.hpp"
#include "opencv2/highgui/highgui.hpp"



class SerialCamera {
public:
	SerialCamera::SerialCamera()
	{
	}
	SerialCamera::~SerialCamera()
	{
	}
	int init();
	void begin();
	void open(char * usbport);
	bool isOpen();
	int snap(char * filename);
	void record(char *filename);
	void close();

private:

};
