//GK12 Smarthouse Source
//	Created By:		Mike Moss and Ben Neubauer
//	Modified On:	08/29/2013

//Serial Sync Variables
//0			Temp Sensor * 100
//1			Temp Sensor * 100
//2			Temp Sensor * 100
//3			Temp Sensor * 100
//4			Light Sensor
//5			Temp Desired
//6			Temp Min
//7			Temp Max
//8			Fans (Bit Field)
//9-11		BRG
//12-14		BRG
//15-17		BRG
//18-20		BRG
//21		Gas Sensor
//22		Motion Sensor
//23		Key Sensor

//IO Stream Header
#include <iostream>

//JSON Header
#include "msl/json.hpp"

//Serial Sync Header
#include "msl/serial_sync.hpp"

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

//Vector Header
#include <vector>

//Web Server Header
#include "msl/webserver.hpp"

//Global Light Color Variables
uint8_t light_colors[12];

//Our Service Client Function Declaration
bool service_client(msl::socket& client,const std::string& message);

//Global Serial Sync Object
msl::serial_sync ss("/dev/ttyACM0",9600);

//Main
int main(int argc,char* argv[])
{
	//Server Variables
	std::string server_port="8080";
	std::string server_serial="/dev/ttyACM0";
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

	ss=msl::serial_sync(server_serial,server_baud);
	ss.setup();

	//Check Serial
	if(ss.good())
	{
		std::cout<<"Serial "<<server_serial<<" "<<server_baud<<" =)"<<std::endl;
	}
	else
	{
		std::cout<<"Serial "<<server_serial<<" "<<server_baud<<" =("<<std::endl;
		server_passed=false;
	}

	//Global Timer Variable (TESTING)
	long timer=msl::millis();

	//Set Lights
	for(int ii=0;ii<12;++ii)
		light_colors[ii]=128;

	//Be a server...forever...
	while(server_passed)
	{
		//Update Serial RX
		ss.update_rx();

		//Update Server
		server.update();

		//Check Temperatures (Every 200ms)
		if(msl::millis()-timer>=200)
		{
			//std::cout<<ss.get(23)<<std::endl;

			//Update Lights
			for(int ii=0;ii<12;++ii)
				ss.set(9+ii,light_colors[ii]);

			//Update Timer
			timer=msl::millis();
		}

		//Update Serial TX
		ss.update_tx();

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
		temperatures.set("0",ss.get(0)/100.0);
		temperatures.set("1",ss.get(1)/100.0);
		temperatures.set("2",ss.get(2)/100.0);
		temperatures.set("3",ss.get(3)/100.0);
		temperatures.set("desired",ss.get(5));

		//Send Temperatures
		client<<msl::http_pack_string(temperatures.str(),"text/plain");

		//Return True (We serviced the client)
		return true;
	}

	//Check For Color Get Request
	else if(request=="/colors?")
	{
		//Package Colors in JSON
		msl::json colors;

		for(int ii=0;ii<12;++ii)
			colors.set(msl::to_string(ii),msl::to_string((float)light_colors[ii]/255.0));

		//Send Colors
		client<<msl::http_pack_string(colors.str(),"text/plain");

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
			light_colors[0]=(uint8_t)(msl::to_int(colors.get("blue0")));
			light_colors[1]=(uint8_t)(msl::to_int(colors.get("red0")));
			light_colors[2]=(uint8_t)(msl::to_int(colors.get("green0")));
			//ss.set(9,(uint8_t)(msl::to_int(colors.get("blue0"))));
			//ss.set(10,(uint8_t)(msl::to_int(colors.get("red0"))));
			//ss.set(11,(uint8_t)(msl::to_int(colors.get("green0"))));
		}

		//Change Color 1
		if(colors.get("blue1")!=""&&colors.get("red1")!=""&&colors.get("green1")!="")
		{
			light_colors[3]=(uint8_t)(msl::to_int(colors.get("blue1")));
			light_colors[4]=(uint8_t)(msl::to_int(colors.get("red1")));
			light_colors[5]=(uint8_t)(msl::to_int(colors.get("green1")));
			//ss.set(12,(uint8_t)(msl::to_int(colors.get("blue1"))));
			//ss.set(13,(uint8_t)(msl::to_int(colors.get("red1"))));
			//ss.set(14,(uint8_t)(msl::to_int(colors.get("green1"))));
		}

		//Change Color 2
		if(colors.get("blue2")!=""&&colors.get("red2")!=""&&colors.get("green2")!="")
		{
			light_colors[6]=(uint8_t)(msl::to_int(colors.get("blue2")));
			light_colors[7]=(uint8_t)(msl::to_int(colors.get("red2")));
			light_colors[8]=(uint8_t)(msl::to_int(colors.get("green2")));
			//ss.set(15,(uint8_t)(msl::to_int(colors.get("blue2"))));
			//ss.set(16,(uint8_t)(msl::to_int(colors.get("red2"))));
			//ss.set(17,(uint8_t)(msl::to_int(colors.get("green2"))));
		}

		//Change Color 3
		if(colors.get("blue3")!=""&&colors.get("red3")!=""&&colors.get("green3")!="")
		{
			light_colors[9]=(uint8_t)(msl::to_int(colors.get("blue3")));
			light_colors[10]=(uint8_t)(msl::to_int(colors.get("red3")));
			light_colors[11]=(uint8_t)(msl::to_int(colors.get("green3")));
			//ss.set(18,(uint8_t)(msl::to_int(colors.get("blue3"))));
			//ss.set(19,(uint8_t)(msl::to_int(colors.get("red3"))));
			//ss.set(20,(uint8_t)(msl::to_int(colors.get("green3"))));
		}

		//Return True (We serviced the client)
		return true;
	}

	//Check For Temperature Set Request
	else if(msl::starts_with(request,"/desired_temp="))
	{
		//Set Desired Temperature
		short desired_temp=msl::to_int(request.substr(14,request.size()-14));
		ss.set(5,desired_temp);

		//Return True (We serviced the client)
		return true;
	}

	//Default Return False (We did not service the client)
	return false;
}