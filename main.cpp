//GK12 Smarthouse Source
//	Created By:		Mike Moss and Ben Neubauer
//	Modified On:	08/03/2013

//Serial Sync Variables
//0			Temp Sensor
//1			Temp Sensor
//2			Temp Sensor
//3			Temp Sensor
//4			Light Sensor
//5-8		Fans
//9-11		BRG
//12-14		BRG
//15-17		BRG
//18-20		BRG

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

//Vector Header
#include <vector>

//Web Server Header
#include "msl/webserver.hpp"

//Our Service Client Function Declaration
bool service_client(msl::socket& client,const std::string& message);

//Global Serial Sync Object
SerialSync ss("/dev/ttyUSB0",9600);

//Global Temperature Variables
int desired_temp_min=60;
int desired_temp_max=75;
int desired_temp=75;
int temp_deadband=3;
unsigned int temp_sample_size=50;
std::vector<double> temp_samples[4];

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
			//Regulate Temperature
			for(int ii=0;ii<4;++ii)
			{
				//Calculate Temperature in F
				double temp=((ss.get(ii)*5/1023.0)*100-50)*9/5.0+32;

				//Store Temperature in Samples
				temp_samples[ii].push_back(temp);

				//Resize Samples While Neccessary
				while(temp_samples[ii].size()>temp_sample_size)
					temp_samples[ii].erase(temp_samples[ii].begin());

				//Calculate Average
				double average=0;

				for(unsigned int jj=0;jj<temp_samples[ii].size();++jj)
					average+=temp_samples[ii][jj];

				average/=static_cast<double>(temp_samples[ii].size());

				//Turn On/Off Fans
				if(average>desired_temp+temp_deadband/2.0)
					ss.set(ii+5,0);
				else if(average<desired_temp-temp_deadband/2.0)
					ss.set(ii+5,1);
			}

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
		//Calculate Averages
		double averages[4];

		for(int ii=0;ii<4;++ii)
		{
			averages[ii]=0;

			for(unsigned int jj=0;jj<temp_samples[ii].size();++jj)
				averages[ii]+=temp_samples[ii][jj];

			if(temp_samples[ii].size()>0)
				averages[ii]/=static_cast<double>(temp_samples[ii].size());
		}

		//Package Temperatures in JSON
		msl::json temperatures;
		temperatures.set("0",averages[0]);
		temperatures.set("1",averages[1]);
		temperatures.set("2",averages[2]);
		temperatures.set("3",averages[3]);
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
			ss.set(9,(uint8_t)(msl::to_int(colors.get("blue0"))));
			ss.set(10,(uint8_t)(msl::to_int(colors.get("red0"))));
			ss.set(11,(uint8_t)(msl::to_int(colors.get("green0"))));
		}

		//Change Color 1
		if(colors.get("blue1")!=""&&colors.get("red1")!=""&&colors.get("green1")!="")
		{
			ss.set(12,(uint8_t)(msl::to_int(colors.get("blue1"))));
			ss.set(13,(uint8_t)(msl::to_int(colors.get("red1"))));
			ss.set(14,(uint8_t)(msl::to_int(colors.get("green1"))));
		}

		//Change Color 2
		if(colors.get("blue2")!=""&&colors.get("red2")!=""&&colors.get("green2")!="")
		{
			ss.set(15,(uint8_t)(msl::to_int(colors.get("blue2"))));
			ss.set(16,(uint8_t)(msl::to_int(colors.get("red2"))));
			ss.set(17,(uint8_t)(msl::to_int(colors.get("green2"))));
		}

		//Change Color 3
		if(colors.get("blue3")!=""&&colors.get("red3")!=""&&colors.get("green3")!="")
		{
			ss.set(18,(uint8_t)(msl::to_int(colors.get("blue3"))));
			ss.set(19,(uint8_t)(msl::to_int(colors.get("red3"))));
			ss.set(20,(uint8_t)(msl::to_int(colors.get("green3"))));
		}

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