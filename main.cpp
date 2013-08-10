//GK12 Smarthouse Source
//	Created By:		Mike Moss and Ben Neubauer
//	Modified On:	08/03/2013

//THIS IS LINUX ONLY

//IO Stream Header
#include <iostream>

//JSON Header
#include "msl/json.hpp"

//Serial Sync Header
#include "SerialSync.hpp"

//Socket Header
#include "msl/socket.hpp"

//Socket Utility Header
#include "msl/socket_util.hpp"

//String Stream Header
#include <sstream>

//String Utility Header
#include "msl/string_util.hpp"

//Time Utility Header
#include "msl/time_util.hpp"

//Web Server Header
#include "msl/webserver.hpp"

//Our Service Client Function Declaration
bool service_client(msl::socket& client,const std::string& message);

//Global Serial Sync Object
SerialSync ss("/dev/ttyUSB0",9600);














//Linux Headers For Accessing File Descriptor and SPI Lines
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char* device="/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
int fd_spi;
#define nLEDs 18
#define nBARs 1
uint8_t ledBar[nBARs][nLEDs];
struct spi_ioc_transfer tr;


int init_spi()
{
	fd_spi=open(device,O_RDWR);

	if (fd_spi < 0)
		pabort("can't open device");

	int ret=ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);

	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd_spi, SPI_IOC_RD_MODE, &mode);

	if (ret == -1)
		pabort("can't get spi mode");

	ret = ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);

	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits);

	if (ret == -1)
		pabort("can't get bits per word");

	ret = ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

	if (ret == -1)
		pabort("can't get max speed hz");

	tr.tx_buf = (unsigned long)ledBar;
	tr.len = nBARs * nLEDs;
	tr.delay_usecs = delay;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;

	return ret;
}

void loadWS2803()
{
	if(ioctl(fd_spi,SPI_IOC_MESSAGE(1),&tr)<1)
		pabort("can't send spi message");
}

void clearWS2803()
{
	for(int wsOut=0;wsOut<nLEDs;++wsOut)
	{
		if(nBARs>1)
			ledBar[0][wsOut]=ledBar[1][wsOut]=0x00;
		else
			ledBar[0][wsOut]=0x00;

		loadWS2803();
	}
}










//Global Temperature Variable
int desired_temp_min=60;
int desired_temp_max=75;
int desired_temp=75;

//Main
int main(int argc,char* argv[])
{
	//Server Variables
	std::string server_port="8080";
	std::string server_serial="/dev/ttyUSB0";
	unsigned int server_baud=9600;
	bool server_passed=true;

	//Get Command Line Port
	if(argc>1)
		server_port=argv[1];

	//Get Command Line Serial Port
	if(argc>2)
		server_serial=argv[2];

	//Get Command Line Serial Baud
	if(argc>3)
		server_baud=msl::to_int(argv[3]);

	//Create Server
	msl::webserver server("0.0.0.0:"+server_port,service_client);
	server.setup();

	//Check Server
	if(server.good())
	{
		std::cout<<"Socket "<<server_port<<" =)"<<std::endl;
	}
	else
	{
		std::cout<<"Socket "<<server_port<<" =("<<std::endl;
		server_passed=false;
	}

	//Endable SPI
	if(init_spi()!=-1)
	{
		std::cout<<"SPI =)"<<std::endl;
	}
	else
	{
		std::cout<<"SPI =("<<std::endl;
		server_passed=false;
	}

	//Check Serial
	try
	{
		ss=SerialSync(server_serial,server_baud);
		ss.setup();
		std::cout<<"Serial "<<server_serial<<" "<<server_baud<<" =)"<<std::endl;
	}
	catch(...)
	{
		std::cout<<"Serial "<<server_serial<<" "<<server_baud<<" =("<<std::endl;
		server_passed=false;
	}

	//Global Timer Variable (TESTING)
	long timer=msl::millis();

	//Be a server...forever...
	while(server_passed)
	{
		//Update Server
		server.update();

		//Update Serial
		ss.loop();

		//Check Temperatures (Every 200ms)
		if(msl::millis()-timer>=200)
		{
			//Temperature 0
			if(((ss.get(0)*5/1023.0)*100-50)*9/5.0+32>desired_temp)
				ss.set(5,0);
			else
				ss.set(5,1);

			if(((ss.get(1)*5/1023.0)*100-50)*9/5.0+32>desired_temp)
				ss.set(6,0);
			else
				ss.set(6,1);

			if(((ss.get(2)*5/1023.0)*100-50)*9/5.0+32>desired_temp)
				ss.set(7,0);
			else
				ss.set(7,1);

			if(((ss.get(3)*5/1023.0)*100-50)*9/5.0+32>desired_temp)//Room 4
				ss.set(8,0);
			else
				ss.set(8,1);

			//Update Timer
			timer=msl::millis();
		}

		//Give OS a Break
		usleep(0);
	}

	//Call Me Plz T_T
	return 0;
}

//Our Service Client Function Definition
bool service_client(msl::socket& client,const std::string& message)
{
	//Create Parser
	std::istringstream istr(message);

	//Parse the Request
	std::string request;
	istr>>request;
	istr>>request;

	//Check For Temperature Request
	if(request=="/temperatures?")
	{
		//Package Temperatures in JSON
		msl::json temperatures;
		temperatures.set("0",ss.get(0));
		temperatures.set("1",ss.get(1));
		temperatures.set("2",ss.get(2));
		temperatures.set("3",ss.get(3));
		temperatures.set("desired",desired_temp);

		//Send Temperatures
		client<<msl::http_pack_string(temperatures.str(),"text/plain");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Color Set Request
	else if(msl::starts_with(request,"/colors="))
	{
		//Convert Request to JSON
		msl::json colors(request.substr(8,request.size()-8));

		//Change Color 0
		if(colors.get("blue0")!=""&&colors.get("red0")!=""&&colors.get("green0")!="")
		{
			ledBar[0][0]=(uint8_t)(msl::to_int(colors.get("blue0")));
			ledBar[0][1]=(uint8_t)(msl::to_int(colors.get("red0")));
			ledBar[0][2]=(uint8_t)(msl::to_int(colors.get("green0")));
		}

		//Change Color 1
		if(colors.get("blue1")!=""&&colors.get("red1")!=""&&colors.get("green1")!="")
		{
			ledBar[0][3]=(uint8_t)(msl::to_int(colors.get("blue1")));
			ledBar[0][4]=(uint8_t)(msl::to_int(colors.get("red1")));
			ledBar[0][5]=(uint8_t)(msl::to_int(colors.get("green1")));
		}

		//Change Color 2
		if(colors.get("blue2")!=""&&colors.get("red2")!=""&&colors.get("green2")!="")
		{
			ledBar[0][6]=(uint8_t)(msl::to_int(colors.get("blue2")));
			ledBar[0][7]=(uint8_t)(msl::to_int(colors.get("red2")));
			ledBar[0][8]=(uint8_t)(msl::to_int(colors.get("green2")));
		}

		//Change Color 3
		if(colors.get("blue3")!=""&&colors.get("red3")!=""&&colors.get("green3")!="")
		{
			ledBar[0][9]=(uint8_t)(msl::to_int(colors.get("blue3")));
			ledBar[0][10]=(uint8_t)(msl::to_int(colors.get("red3")));
			ledBar[0][11]=(uint8_t)(msl::to_int(colors.get("green3")));
		}

		loadWS2803();

		//Return True (We serviced the client)
		return true;
	}

	//Check For Temperature Set Request
	else if(msl::starts_with(request,"/desired_temp="))
	{
		//Set Desired Temperature
		desired_temp=msl::to_int(request.substr(14,request.size()-14));

		//Limit Low Temperature
		if(desired_temp<desired_temp_min)
			desired_temp=desired_temp_min;

		//Limit High Temperature
		if(desired_temp>desired_temp_max)
			desired_temp=desired_temp_max;

		//Return True (We serviced the client)
		return true;
	}

	//Default Return False (We did not service the client)
	return false;
}