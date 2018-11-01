#include <stdlib.h>
/*
 * @file spi_slave_test_slave_side.cpp
 *
 * @author FTDI
 * @date 2018-03-27
 *
 * Copyright 2011 Future Technology Devices International Limited
 * Company Confidential
 *
 * Revision History:
 * 1.0 - initial version
 * 1.1 - spi slave with protocol and ack function
  */
#define _REEENTRANT

  //------------------------------------------------------------------------------
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <conio.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>

//#include <fcntl.h>

#include "pthreaddefs.h"
//------------------------------------------------------------------------------
// include FTDI libraries
//
#include "ftd2xx.h"
#include "LibFT4222.h"
#include "serial_radar.h"

#define USER_WRITE_REQ      0x4a
#define USER_READ_REQ       0x4b

std::vector< FT_DEVICE_LIST_INFO_NODE > g_FTAllDevList;
std::vector< FT_DEVICE_LIST_INFO_NODE > g_FT4222DevList;

uint16 rxSize;
uint16 sizeTransferred;


FT_HANDLE ftHandle = NULL;
FT_STATUS ftStatus;
FT_STATUS ft4222_status;

void thread_loop(bool ready);

//------------------------------------------------------------------------------
inline std::string DeviceFlagToString(DWORD flags)
{
	std::string msg;
	msg += (flags & 0x1) ? "DEVICE_OPEN" : "DEVICE_CLOSED";
	msg += ", ";
	msg += (flags & 0x2) ? "High-speed USB" : "Full-speed USB";
	return msg;
}

void ListFtUsbDevices()
{
	FT_STATUS ftStatus = 0;

	DWORD numOfDevices = 0;
	ftStatus = FT_CreateDeviceInfoList(&numOfDevices);

	for (DWORD iDev = 0; iDev < numOfDevices; ++iDev)
	{
		FT_DEVICE_LIST_INFO_NODE devInfo;
		memset(&devInfo, 0, sizeof(devInfo));

		ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type, &devInfo.ID, &devInfo.LocId,
			devInfo.SerialNumber,
			devInfo.Description,
			&devInfo.ftHandle);

		if (FT_OK == ftStatus)
		{
			printf("Dev %d:\n", iDev);
			printf("  Flags= 0x%x, (%s)\n", devInfo.Flags, DeviceFlagToString(devInfo.Flags).c_str());
			printf("  Type= 0x%x\n", devInfo.Type);
			printf("  ID= 0x%x\n", devInfo.ID);
			printf("  LocId= 0x%x\n", devInfo.LocId);
			printf("  SerialNumber= %s\n", devInfo.SerialNumber);
			printf("  Description= %s\n", devInfo.Description);
			printf("  ftHandle= 0x%x\n", devInfo.ftHandle);

			const std::string desc = devInfo.Description;
			g_FTAllDevList.push_back(devInfo);

			if (desc == "FT4222" || desc == "FT4222 A")
			{
				g_FT4222DevList.push_back(devInfo);
			}
		}
	}
}


class SPIMessage
{
public:
	SPIMessage(uint16 size)
	{
		data.resize(size);

		for (uint16 i = 0; i < data.size(); i++)
		{
			data[i] = (uint8)i;
		}
	}

public:
	std::vector< unsigned char > data;
};


static bool keepRunning = true;

int num = 0;
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
void *spiUSB(void *x_void_ptr)
{

	ListFtUsbDevices();

	if (g_FT4222DevList.empty()) {
		printf("No FT4222 device is found!\n");
		return nullptr;
	}

	/*	for (int idx = 0; idx < 10; idx++)
		{
			printf("select dev num(0~%d) as spi slave\n", g_FTAllDevList.size() - 1);
			num = getch();
			num = num - '0';
			if (num >= 0 && num < g_FTAllDevList.size())
			{
				break;
			}
			else
			{
				printf("input error , please input again\n");
			}

		}
		*/
	return spiInit();
}

void *spiInit()
{
	ftStatus = FT_Open(num, &ftHandle);
	if (FT_OK != ftStatus)
	{
		printf("Open a FT4222 device failed!\n");
		return nullptr;
	}

	ftStatus = FT4222_SetClock(ftHandle, SYS_CLK_80);
	if (FT_OK != ftStatus)
	{
		printf("FT4222_SetClock failed!\n");
		return nullptr;
	}

	//Set default Read and Write timeout 1 sec
	ftStatus = FT_SetTimeouts(ftHandle, 1000, 1000);
	if (FT_OK != ftStatus)
	{
		printf("FT_SetTimeouts failed!\n");
		return nullptr;
	}

	// set latency to 1
	ftStatus = FT_SetLatencyTimer(ftHandle, 1);
	if (FT_OK != ftStatus)
	{
		printf("FT_SetLatencyTimerfailed!\n");
		return nullptr;
	}

	//
	ftStatus = FT_SetUSBParameters(ftHandle, 64 * 1024, 0);
	if (FT_OK != ftStatus)
	{
		printf("FT_SetUSBParameters failed!\n");
		return nullptr;
	}


	ft4222_status = FT4222_SPISlave_InitEx(ftHandle, SPI_SLAVE_NO_PROTOCOL);
	if (FT4222_OK != ft4222_status)
	{
		printf("Init FT4222 as SPI master device failed!\n");
		return nullptr;
	}

	if (FT4222_OK != FT4222_SPISlave_SetMode(ftHandle, CLK_IDLE_LOW, CLK_TRAILING))  //// works ////
	{
		printf("Init FT4222 as SPI set mode failed!\n");
		return nullptr;
	}

	ft4222_status = FT4222_SPI_SetDrivingStrength(ftHandle, DS_4MA, DS_4MA, DS_4MA);
	if (FT4222_OK != ft4222_status)
	{
		printf("FT4222_SPI_SetDrivingStrength failed!\n");
		return nullptr;
	}
	printf("start waiting master request..............\n");


	//pthread_mutex_init(&mutex_spi_ready, nullptr);

	thread_loop(true);

	return nullptr;
}

void thread_loop(bool ready) {

	if (ready) {
		//pthread_mutex_unlock(&mutex_spi_ready);
	}
}

static uint64_t last_time = 0;
static uint16_t dots = 0;
boolean spiCheck(std::vector<unsigned char> &msgBuf) {

	ft4222_status = FT4222_SPISlave_GetRxStatus(ftHandle, &rxSize);
	if (ft4222_status == FT4222_OK)
	{
		if (rxSize > 0)
		{
			std::chrono::high_resolution_clock m_clock;

			uint64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>
				(m_clock.now().time_since_epoch()).count();

			// printf("===============================  Received %d bytes  %d \n", rxSize, millis - last_time);
			//if (dots++ > 32) {
				//printf("\n");
				//dots = 0;
			//}
			//printf(".");
			last_time = millis;
			std::vector<unsigned char> tmpBuf;
			msgBuf.resize(rxSize);

			ft4222_status = FT4222_SPISlave_Read(ftHandle, &msgBuf[0], rxSize, &sizeTransferred);
			if ((ft4222_status == FT4222_OK) && (rxSize == sizeTransferred))
			{
				//pthread_mutex_unlock(&mutex_spi_ready);
				return true;
			}
			else
			{
				printf("FT4222_SPISlave_Read error ft4222_status=%d\n", ft4222_status);
				return false;
			}
		}
		else {
			return false;
		}
		return false;
	}
	return false;
}

void spiClose()
{

	FT4222_UnInitialize(ftHandle);
	FT_Close(ftHandle);
}





/* this is the data structure that is used between the producer
   and consumer threads */

void *SPIProducer(void  *pbuf)
{
	/* the producer ! */
	while (1) {
		Buf_t *buf = (Buf_t *)pbuf;

		/* check to see if any buffers are empty */
		/* If not then wait for that condition to become true */
		ft4222_status = FT4222_SPISlave_GetRxStatus(ftHandle, &rxSize);
		if (ft4222_status == FT4222_OK)
		{
			if (rxSize > 0)
			{
				if ((ft4222_status == FT4222_OK) && (rxSize == sizeTransferred))
				{

					/* lock the mutex */
					pthread_mutex_lock(&buf->buflock);
					/* lock the done lock */
					pthread_mutex_lock(&buf->donelock);


					buf->msgBufs[buf->nextadd]->resize(rxSize);
					/* set the next buffer to fill */


					/* increment the number of buffers that are filled */
					buf->occ++;
					vector< uint8_t> receiver;
					receiver = *buf->msgBufs[buf->nextadd];
					ft4222_status = FT4222_SPISlave_Read(ftHandle, &receiver[0], rxSize, &sizeTransferred);

					/* set the done flag and release the mutex lock */
					buf->available = true;
					buf->nextadd = ++buf->nextadd % BUFCNT;

					pthread_mutex_unlock(&buf->donelock);

					/* signal the consumer to start consuming */
					pthread_cond_signal(&buf->adddata);

					/* release the buffer mutex */
					pthread_mutex_unlock(&buf->buflock);
				}
				else
				{
					printf("FT4222_SPISlave_Read error ft4222_status=%d\n", ft4222_status);
				}
			}
		}


	}

	/* wait for the consumer to finish */
	// pthread_join(cons_thr, 0, NULL);

	/* exit the program */
	return nullptr;
}

