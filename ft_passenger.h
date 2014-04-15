/*-------------------------------------------------------
FAST-TrIPs: Flexible Assignment and Simulation Tool for Transit and Intermodal Passengers
Copyright (C) 2013 by Alireza Khani
Released under the GNU General Public License, version 2.
-------------------------------------------------------
Code primarily written by Alireza Khani
Under supervision of Mark Hickman

Contact:
    Alireza Khani:  akhani@utexas.edu or akhani@email.arizona.edu
    Mark Hickman:   m.hickman1@uq.edu.au
-------------------------------------------------------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
-------------------------------------------------------*/

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class passenger{
protected:
	string				passengerString;
	string				passengerId;
	string				passengerOriginTAZ;
	string				passengerDestinationTAZ;
	int					passengerMode;
	int					passengerTimePeriod;
	int					passengerTourHalf;
	double				passengerPDT;
	double				passengerPAT;
	string				passengerTazTime;

	//Simulation
	string				assignedPath;
	double				startTime;
	double				endTime;
	int 				lengthOfPassengerTrip;
	vector<string>		boardingStops;
	vector<string>		trips;
	vector<string>		alightingStops;
	vector<double>		walkingTimes;

	int					passengerStatus;	//-1=not assigned, 0=assigned, 1=walking, 2=waiting, 3=on board, 4=missed, 5=arrived,
	int					passengerPathIndex;
	vector<double>		experiencedArrivalTimes;
	vector<double>		experiencedBoardingTimes;
	vector<double>		experiencedAlightingTimes;
	string				experiencedPath;
	double				experiencedCost;

public:
	passenger(){}
	~passenger(){}

	void			initializePassenger(string _passengerStr);
	string			getPassengerString();
	string			getPassengerId();
	string			getOriginTAZ();
	string			getDestinationTAZ();
	double			getPDT();
	double			getPAT();
	int				getTourHalf();
	int				getMode();
	string			getTazTime();
	int				getTimePeriod();

	//For Assignment
	void			setAssignedPath(string _tmpPath);
	string			getAssignedPath();

	//For Simulation
	void			resetPathInfo();
	int				initializePath();
	int				getPathIndex();
	double			getStartTime();
	void			setPassengerStatus(int	_status);
	int				getPassengerStatus();
	void			increasePathIndex();
	void			setEndTime(double _endTime);
	string			getCurrentBoardingStopId();
	string			getCurrentTripId();
	string			getCurrentAlightingStopId();
	double			getCurrentWalkingTime();

	void			addArrivalTime(double _arrivalTime);
	void			addBoardingTime(double	_boardingTime);
	void			addAlightingTime(double _alightingTime);
	double			getArrivalTime();
	double			getBoardingTime();
	double			getAlightingTime();
	string			getExperiencedPath();

	//For Available Capacity Definition
	string			getLastTripId();

	void			calculateExperiencedCost();
	double			getExperiencedCost();

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
map<string,passenger*>				passengerSet;
list<passenger*>					passengerList;
map<string,string>					tazTimeSet;
list<passenger*>					passengers2transfer;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int			readPassengers(){
	string			tmpIn, buf, tmpPassengerId, tmpMode;
	vector<string>	tokens;
	passenger*		tmpPassengerPntr;

	ifstream inFile;
	inFile.open("ft_input_demand.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_demand.dat";
		exit(1);
	}
	getline(inFile,tmpIn);
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		while (ss >> buf){
			tokens.push_back(buf);
		}
		tmpPassengerId = "p";
		tmpPassengerId.append(tokens[0]);
		tmpPassengerPntr = NULL;
		tmpPassengerPntr = new passenger;
		passengerSet[tmpPassengerId] = tmpPassengerPntr;
		passengerList.push_back(tmpPassengerPntr);
		passengerSet[tmpPassengerId]->initializePassenger(tmpIn);
	}
	inFile.close();
	return passengerSet.size();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int			readExistingPaths(){
	int				tmpNumPaths;
	string			tmpIn, buf, tmpPassengerId, tmpPath;
	vector<string>	tokens;

	ifstream inFile;
	inFile.open("ft_output_passengerPaths.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_output_passengerPaths.dat";
		exit(1);
	}

	tmpNumPaths = 0;
	getline(inFile,tmpIn);
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		while (ss >> buf){
			tokens.push_back(buf);
		}
		tmpPassengerId = "p";
		tmpPassengerId.append(tokens[0]);
		tmpPath = tokens[4] + "\t" + tokens[5] + "\t" + tokens[6] + "\t" +  tokens[7] + "\t" + tokens[8];
		passengerSet[tmpPassengerId]->setAssignedPath(tmpPath);
		tmpNumPaths++;
	}
	inFile.close();
	return tmpNumPaths;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void		passenger::initializePassenger(string _tmpIn){
	string				buf, tmpStr;
	vector<string>		tokens;
	char				chr[99];

	passengerString = _tmpIn;
	buf.clear();
	tokens.clear();
	stringstream	ss1(_tmpIn);
	while (ss1 >> buf){
		tokens.push_back(buf);
	}
	passengerOriginTAZ = "t" + tokens[1];
	passengerDestinationTAZ = "t" + tokens[2];
	passengerMode = atoi(tokens[3].c_str());
	passengerTimePeriod = atoi(tokens[4].c_str());
	passengerTourHalf = atoi(tokens[5].c_str());
	if(passengerTourHalf==1){
		passengerPAT = atof(tokens[6].c_str());
		if(passengerPAT<180){
			passengerPAT = passengerPAT + 1440;
		}
		passengerPDT = 0;
	}else{
		passengerPDT = atof(tokens[6].c_str());
		if(passengerPDT<180){
			passengerPDT = passengerPDT + 1440;
		}
		passengerPAT = 1800;
	}
	passengerId = "p";
	passengerId.append(tokens[0]);

	passengerStatus = -1;
	experiencedCost = 999999;
	startTime = -101;
}
string		passenger::getPassengerString(){
	return this->passengerString;
}
string		passenger::getPassengerId(){
	return this->passengerId;
}
string		passenger::getOriginTAZ(){
	return this->passengerOriginTAZ;
}
string		passenger::getDestinationTAZ(){
	return this->passengerDestinationTAZ;
}
double		passenger::getPDT(){
	return this->passengerPDT;
}
double		passenger::getPAT(){
	return this->passengerPAT;
}
int			passenger::getTourHalf(){
	return this->passengerTourHalf;
}
int			passenger::getMode(){
	return this->passengerMode;
}
void		passenger::setAssignedPath(string _tmpPath){
	assignedPath = _tmpPath;
}
string		passenger::getAssignedPath(){
	return this->assignedPath;
}
int			passenger::getTimePeriod(){
	return this->passengerTimePeriod;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void		passenger::resetPathInfo(){
	startTime = -101;
	endTime = -101;
	lengthOfPassengerTrip = 0;
	boardingStops.clear();
	trips.clear();
	alightingStops.clear();
	walkingTimes.clear();

	passengerStatus = -1;	//-1=not assigned, 0=assigned, 1=walking, 2=waiting, 3=on board, 4=missed, 5=arrived,
	passengerPathIndex = 0;

	experiencedArrivalTimes.clear();
	experiencedBoardingTimes.clear();
	experiencedAlightingTimes.clear();
	experiencedPath = "";
	//experiencedCost = 999999;
}
int			passenger::initializePath(){
	vector<string>		tokens;
	string				buf, tmpPathString, tmpBoardingStops, tmpTrips, tmpAlightingStops, tmpWalkingTimes;

	buf.clear();
	tokens.clear();
	tmpPathString = assignedPath;
	stringstream ss(tmpPathString);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	startTime = atof(tokens[0].c_str());
	tmpBoardingStops = tokens[1];
	tmpTrips = tokens[2];
	tmpAlightingStops = tokens[3];
	tmpWalkingTimes = tokens[4];										//cout<<tmpWalkingTimes<<endl;

	//boardingStops.clear();
	stringstream ss1(tmpBoardingStops);
	while (getline(ss1, buf, ',')){
		boardingStops.push_back(buf);
	}
	//trips.clear();
	stringstream ss2(tmpTrips);
	while (getline(ss2, buf, ',')){
		trips.push_back(buf);
	}
	//alightingStops.clear();
	stringstream ss3(tmpAlightingStops);
	while (getline(ss3, buf, ',')){
		alightingStops.push_back(buf);
	}
	//walkingTimes.clear();
	stringstream ss4(tmpWalkingTimes);
	while (getline(ss4, buf, ',')){
		walkingTimes.push_back(atof(buf.c_str()));
	}

	lengthOfPassengerTrip = trips.size();
	passengerPathIndex = 0;

	//experiencedArrivalTimes.clear();
	//experiencedBoardingTimes.clear();
	//experiencedAlightingTimes.clear();
	//experiencedPath = "";
	//experiencedCost = 0;
	//endTime = -101;
	return lengthOfPassengerTrip;
}
int			passenger::getPathIndex(){
	return this->passengerPathIndex;
}
double		passenger::getStartTime(){
	return this->startTime;
}
void		passenger::setPassengerStatus(int _status){
	passengerStatus = _status;
}
int			passenger::getPassengerStatus(){
	return this->passengerStatus;
}
void		passenger::increasePathIndex(){
	passengerPathIndex = passengerPathIndex + 1;
}
void		passenger::setEndTime(double _endTime){
	endTime = _endTime;
}
string		passenger::getCurrentBoardingStopId(){
	if(passengerPathIndex<lengthOfPassengerTrip){
		return this->boardingStops[passengerPathIndex];
	}else{
		return "-101";
	}
}
string		passenger::getCurrentTripId(){
	if(passengerPathIndex<lengthOfPassengerTrip){
		return this->trips[passengerPathIndex];
	}else{
		return "-101";
	}
}
string		passenger::getCurrentAlightingStopId(){
	if(passengerPathIndex<lengthOfPassengerTrip){
		return this->alightingStops[passengerPathIndex];
	}else{
		return "-101";
	}
}
double		passenger::getCurrentWalkingTime(){
	return this->walkingTimes[passengerPathIndex];
}
void		passenger::addArrivalTime(double _arrivalTime){
	experiencedArrivalTimes.push_back(_arrivalTime);
}
void		passenger::addBoardingTime(double	_boardingTime){
	experiencedBoardingTimes.push_back(_boardingTime);
}
void		passenger::addAlightingTime(double _alightingTime){
	experiencedAlightingTimes.push_back(_alightingTime);
}
double		passenger::getArrivalTime(){
	return	this->experiencedArrivalTimes.back();
}
double		passenger::getBoardingTime(){
	return	this->experiencedBoardingTimes.back();
}
double		passenger::getAlightingTime(){
	if(passengerPathIndex == 0){
		return	this->startTime;
	}else{
		return	this->experiencedAlightingTimes.back();
	}
}
string		passenger::getExperiencedPath(){
	int		i;
	string	tmpExperiencedPath, tmpArrivalTimes, tmpBoardingTimes, tmpAlightingTimes;
	char	chr[99];
	for(i=0;i<experiencedArrivalTimes.size();i++){
		if(i!=0){
			tmpArrivalTimes.append(",");
			tmpBoardingTimes.append(",");
			tmpAlightingTimes.append(",");
		}
		tmpArrivalTimes.append(itoa(int(100*experiencedArrivalTimes[i])/100,chr,10));
		tmpArrivalTimes.append(".");
		if(int(100*experiencedArrivalTimes[i])%100<10)	tmpArrivalTimes.append("0");
		tmpArrivalTimes.append(itoa(int(100*experiencedArrivalTimes[i])%100,chr,10));

		tmpBoardingTimes.append(itoa(int(100*experiencedBoardingTimes[i])/100,chr,10));
		tmpBoardingTimes.append(".");
		if(int(100*experiencedBoardingTimes[i])%100<10)	tmpBoardingTimes.append("0");
		tmpBoardingTimes.append(itoa(int(100*experiencedBoardingTimes[i])%100,chr,10));

		tmpAlightingTimes.append(itoa(int(100*experiencedAlightingTimes[i])/100,chr,10));
		tmpAlightingTimes.append(".");
		if(int(100*experiencedAlightingTimes[i])%100<10)	tmpAlightingTimes.append("0");
		tmpAlightingTimes.append(itoa(int(100*experiencedAlightingTimes[i])%100,chr,10));
	}
	tmpExperiencedPath = passengerId;
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(itoa(passengerMode,chr,10));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(passengerOriginTAZ.substr(1,99));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(passengerDestinationTAZ.substr(1,99));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(itoa(int(100*startTime)/100,chr,10));
	tmpExperiencedPath.append(".");
	if(int(100*startTime)%100<10)	tmpExperiencedPath.append("0");
	tmpExperiencedPath.append(itoa(int(100*startTime)%100,chr,10));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(itoa(int(100*endTime)/100,chr,10));
	tmpExperiencedPath.append(".");
	if(int(100*endTime)%100<10)	tmpExperiencedPath.append("0");
	tmpExperiencedPath.append(itoa(int(100*endTime)%100,chr,10));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(tmpArrivalTimes);
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(tmpBoardingTimes);
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(tmpAlightingTimes);
	return tmpExperiencedPath;
}
//For Available Capacity Definition
string		passenger::getLastTripId(){
	if(passengerPathIndex>0){
		return	this->trips[passengerPathIndex];
	}else{
		return	"Access";
	}
}
///////////////////////////////////////////////
void	passenger::calculateExperiencedCost(){
	double	tmpWaitingTime, tmpAccessWalkingTime, tmpEgressWalkingTime, tmpTransferWalkingTime, tmpInVehTime;
	int		tmpNumTransfers, i;

	if(passengerStatus==5){		//-1=not assigned, 0=assigned, 1=walking, 2=waiting, 3=on board, 4=missed, 5=arrived,
		tmpWaitingTime = 0;
		tmpAccessWalkingTime = 0;
		tmpEgressWalkingTime = 0;
		tmpTransferWalkingTime = 0;
		tmpInVehTime = 0;
		tmpNumTransfers = trips.size() - 1;
		for(i=0;i<trips.size();i++){
			tmpWaitingTime = tmpWaitingTime + experiencedBoardingTimes[i] - experiencedArrivalTimes[i];
			if(i>0){
				tmpTransferWalkingTime = tmpTransferWalkingTime + experiencedArrivalTimes[i] - experiencedAlightingTimes[i-1];
			}
			tmpInVehTime = tmpInVehTime + experiencedAlightingTimes[i] - experiencedBoardingTimes[i];
		}

		experiencedCost = inVehTimeEqv*tmpInVehTime + waitingEqv*tmpWaitingTime + transferWalkEqv*tmpTransferWalkingTime
						+ originWalkEqv*walkingTimes.front() + destinationWalkEqv*walkingTimes.back()
						+ transferPenalty*tmpNumTransfers + (60.0*fare/VOT)*(tmpNumTransfers+1);
	}else if(passengerStatus==4){
		experiencedCost = 999999;
	}else{
		//cout <<passengerId<<"\tSTATUS = "<<passengerStatus<<endl;
	}
}
double	passenger::getExperiencedCost(){
	return this->experiencedCost;
}