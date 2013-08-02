//GK12 Smarthouse Source
//	Created By:		Mike Moss and Ben Neubauer
//	Modified On:	08/01/2013

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

//Web Server Header
#include "msl/webserver.hpp"

//Our Service Client Function Declaration
bool service_client(msl::socket& client,const std::string& message);

//Global Serial Sync Object
SerialSync ss("/dev/ttyUSB0",9600);











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

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

int fd_spi;

#define nLEDs 18
#define nBARs 1		// Number of WS2803 chained together

uint8_t ledBar[nBARs][nLEDs];

struct spi_ioc_transfer tr;


int init_spi(){
int ret = 0;

	fd_spi = open(device, O_RDWR);
	if (fd_spi < 0)
		pabort("can't open device");

	ret = ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);
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

void loadWS2803(){
	if(ioctl(fd_spi, SPI_IOC_MESSAGE(1), &tr) < 1)
		pabort("can't send spi message");
}

void clearWS2803(){
int wsOut;

        for(wsOut = 0; wsOut < nLEDs; wsOut++){
		if(nBARs > 1)
			ledBar[0][wsOut] = ledBar[1][wsOut] = 0x00;
		else
			ledBar[0][wsOut] = 0x00;
		loadWS2803();
	}
}












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

	//Be a server...forever...
	while(server_passed)
	{
		//Update Server
		server.update();

		//Update Serial
		ss.loop();

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

	//Check For a Custom Request
	if(request=="/temperatures?")
	{
		//Package Temperatures in JSON
		msl::json temperatures;
		temperatures.set("0",ss.get(0));
		temperatures.set("1",ss.get(1));
		temperatures.set("2",ss.get(2));
		temperatures.set("3",ss.get(3));

		//Send Custom Message
		client<<msl::http_pack_string(temperatures.str(),"text/plain");

		//Return True (We serviced the client)
		return true;
	}

	else if(msl::starts_with(request,"/color1?"))
	{
		//Change ?'s to spaces
		for(unsigned int ii=0;ii<request.size();++ii)
			if(request[ii]=='?')
				request[ii]=' ';

		//Variables for Parsing
		std::istringstream istr(request);
		std::string header;
		int red1=0;
		int green1=0;
		int blue1=0;

		//Get Header
		istr>>header;

		//Get Color
		istr>>red1;
		istr>>green1;
		istr>>blue1;

		//Print Color
		//std::cout<<"change color\t"<<red<<"\t"<<green<<"\t"<<blue<<std::endl;

		//Change Color
		ledBar[0][0] = (uint8_t)(blue1);
		ledBar[0][1] = (uint8_t)(red1);
		ledBar[0][2] = (uint8_t)(green1);
		loadWS2803();

		return true;
	}
	else if(msl::starts_with(request,"/color2?"))
	{
		//Change ?'s to spaces
		for(unsigned int ii=0;ii<request.size();++ii)
			if(request[ii]=='?')
				request[ii]=' ';

		//Variables for Parsing
		std::istringstream istr(request);
		std::string header;
		int red2=0;
		int green2=0;
		int blue2=0;

		//Get Header
		istr>>header;

		//Get Color
		istr>>red2;
		istr>>green2;
		istr>>blue2;

		//Print Color
		//std::cout<<"change color\t"<<red<<"\t"<<green<<"\t"<<blue<<std::endl;

		//Change Color
		ledBar[0][3] = (uint8_t)(blue2);
		ledBar[0][4] = (uint8_t)(red2);
		ledBar[0][5] = (uint8_t)(green2);
		loadWS2803();

		return true;
	}
	else if(msl::starts_with(request,"/color3?"))
	{
		//Change ?'s to spaces
		for(unsigned int ii=0;ii<request.size();++ii)
			if(request[ii]=='?')
				request[ii]=' ';

		//Variables for Parsing
		std::istringstream istr(request);
		std::string header;
		int red3=0;
		int green3=0;
		int blue3=0;

		//Get Header
		istr>>header;

		//Get Color
		istr>>red3;
		istr>>green3;
		istr>>blue3;

		//Print Color
		//std::cout<<"change color\t"<<red<<"\t"<<green<<"\t"<<blue<<std::endl;

		//Change Color
		ledBar[0][6] = (uint8_t)(blue3);
		ledBar[0][7] = (uint8_t)(red3);
		ledBar[0][8] = (uint8_t)(green3);
		loadWS2803();

		return true;
	}
	else if(msl::starts_with(request,"/color4?"))
	{
		//Change ?'s to spaces
		for(unsigned int ii=0;ii<request.size();++ii)
			if(request[ii]=='?')
				request[ii]=' ';

		//Variables for Parsing
		std::istringstream istr(request);
		std::string header;
		int red4=0;
		int green4=0;
		int blue4=0;

		//Get Header
		istr>>header;

		//Get Color
		istr>>red4;
		istr>>green4;
		istr>>blue4;

		//Print Color
		//std::cout<<"change color\t"<<red<<"\t"<<green<<"\t"<<blue<<std::endl;

		//Change Color
		ledBar[0][9] = (uint8_t)(blue4);
		ledBar[0][10] = (uint8_t)(red4);
		ledBar[0][11] = (uint8_t)(green4);
		loadWS2803();

		return true;
	}

	//Default Return False (We did not service the client)
	return false;
}