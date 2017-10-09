/*-------------------------------------------------------
FAST-TrIPs: Flexible Assignment and Simulation Tool for Transit and Intermodal Passengers
Copyright 2014 Alireza Khani and Mark Hickman
Licensed under the Apache License, Version 2.0
-------------------------------------------------------
Code primarily written by Alireza Khani
Under supervision of Mark Hickman

Contact:
    Alireza Khani:  akhani@utexas.edu or akhani@email.arizona.edu
    Mark Hickman:   m.hickman1@uq.edu.au
-------------------------------------------------------
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-------------------------------------------------------*/

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class	trip{
protected:
	string				tripId;
	string				tripRoute;
	int					tripType;
	double				tripStartTime;
	int					tripCapacity;
	string				tripShape;
	string				tripDirection;

	vector<string>		tripStops;
	vector<double>		tripSchDeparture;
	vector<double>		tripSchArrival;
	vector<double>		tripSchHeadway;

	//For TBSP
	vector<int>			tripUsedBefore;

	//For Simulation
	int					stopIndex;
	list<string>		passengers;
	vector<int>			noOfBoardings;
	vector<int>			noOfAlightings;
	vector<int>			noOfOnBoards;
	vector<int>			dwellTimes;
	vector<int>			tripResidualCapacity;

public:
	trip(){}
	~trip(){}
	void				initializeTrip(string _tmpIn);
	void				attachStopTime(string _tmpIn);
	void				calculateHeadway();
	void				parallelize(int _numThreads);
	string				getTripId();
	string				getRouteId();
	string				getShapeId();
	int					getRouteType();
	string				getDirection();
	int					getMaxSequence();
	string				getStop(int _seq);
	double				getSchArrival(int _seq);
	double				getSchDeparture(int _seq);
	double				getSchHeadway(int _seq);
	double				getSchDepartureByStop(string _stop);
	double				getSchArrivalByStop(string _stop);

	//For TBSP
	void				resetTripUsedBefore(int _threadId);
	void				setTripUsedBefore(int _threadId);
	int					getTripUsedBefore(int _threadId);
	int					getResidualCapacity(int _seq);

	//For Simulation
	void				resetTripForSimulation();
	int					getStopIndex();
	int					getTripStartTime();
	int					getTripCapacity();
	int					getFreeCapacity();
	int					getNoOfOnBoardPassengers();

	int					getNoOfAlightings();
	int					getNoOfBoardings();

	void				addPassenger(string _passengerId);
	void				removePassenger(int _passengerCntr);
	string				getPassengerId(int _passengerCntr);
	void				increaseStopIndex();
	int					checkMissing(string _boardingStop);

	string				getCurrentStop();
	double				getCurrentScheduledArrival();
	double				getCurrentScheduledDeparture();
	void				setBoardings(int _boardings);
	void				setAlightings(int _alightings);
	void				setOnBoards(int _onBoards);
	int					getBoardings(int _seq);
	int					getAlightings(int _seq);
	int					getVehLoad(int _seq);
	void				setDwellTime(int _dwellTime);
	int					getDwellTime(int _seq);
	void				setResidualCapacity(int _residualCapacity);
    
    //For Beijing Metro
    int                 getTripLength();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
map<string,trip*>		tripSet;
list<trip*>				tripList;
list<string>			eventList;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int		readTrips(){
    cout <<"reading trips:\t\t\t";

	string			tmpIn, buf, tmpTripId, tmpRouteId;
	vector<string>	tokens;
	trip*			tmpTripPntr;

	ifstream inFile;
	inFile.open("ft_input_trips.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_trips.dat";
		exit(1);
	}

	//getline(inFile,tmpIn);
	while(!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn == "")		continue;
		stringstream ss(tmpIn);
		while (ss >> buf){
			tokens.push_back(buf);
		}
		tmpTripId = "t";
		tmpTripId.append(tokens[0]);
		tmpTripPntr = NULL;
		tmpTripPntr = new trip;
		tripSet[tmpTripId] = tmpTripPntr;
		tripSet[tmpTripId]->initializeTrip(tmpIn);
		tripList.push_back(tmpTripPntr);

		tmpRouteId = "r";
		tmpRouteId.append(tokens[1]);
		routeSet[tmpRouteId]->attachTrip(tmpTripId);
	}
	inFile.close();
	return tripSet.size();
}
int		readStopTimes(){
    cout <<"reading stop-times:\t\t";

        string						tmpIn, buf, tmpTripId, tmpStopId, tmpRouteId, tmpStr, tmpEventStr, tripDirection;
	int							numStopTimes, tmpTime;
	vector<string>				tokens;
	list<stop*>::iterator		tmpStopListIter;
	list<trip*>::iterator		tmpTripListIter;
	stop*						tmpStopPntr;
	trip*						tmpTripPntr;
	char						chr[999];

	ifstream inFile;
	inFile.open("ft_input_stopTimes.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_stopTimes.dat";
		exit(1);
	}
	//getline(inFile,tmpIn);
	numStopTimes = 0;
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);							//cout<<tmpIn<<endl;
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		while (ss >> buf){
			tokens.push_back(buf);
		}
		tmpTripId = "t";
		tmpTripId.append(tokens[0]);
		tmpStopId = "s";
		tmpStopId.append(tokens[3]);
		tmpRouteId = tripSet[tmpTripId]->getRouteId();
		tripDirection = tripSet[tmpTripId]->getDirection();

		tripSet[tmpTripId]->attachStopTime(tmpIn);
		stopSet[tmpStopId]->attachRoute(tmpRouteId, tripDirection);
		stopSet[tmpStopId]->attachStopTime(tmpIn);
		numStopTimes++;

		//For Simulation
		tmpTime = atof(tokens[1].c_str());
		tmpTime = 3600*(int(tmpTime)/10000) + 60*((int(tmpTime)%10000)/100) + int(tmpTime)%100;
        sprintf(chr,"%d",tmpTime);
        tmpEventStr = string(chr);
        tmpStr.resize(6-tmpEventStr.length(),'0');
		tmpEventStr = tmpStr + tmpEventStr + ",a," + tmpTripId + "," + tmpStopId;
		eventList.push_back(tmpEventStr);

		//For New Simulation
		tmpTime = atof(tokens[2].c_str());
		tmpTime = 3600*(int(tmpTime)/10000) + 60*((int(tmpTime)%10000)/100) + int(tmpTime)%100;
        sprintf(chr,"%d",tmpTime);
        tmpEventStr = string(chr);
		tmpStr.resize(6-tmpEventStr.length(),'0');
		tmpEventStr = tmpStr + tmpEventStr + ",d," + tmpTripId + "," + tmpStopId;
		eventList.push_back(tmpEventStr);
	}
	inFile.close();
	eventList.sort();
    
	//To Calculate Headway
	for(tmpTripListIter=tripList.begin();tmpTripListIter!=tripList.end();tmpTripListIter++){
		tmpTripPntr = (*tmpTripListIter);
		tmpTripPntr->calculateHeadway();
	}

	return eventList.size();
}

int		parallelizeTrips(int _numThreads){
	list<trip*>::iterator	tmpTripListIter;
	trip*					tmpTripPntr;

	for(tmpTripListIter=tripList.begin();tmpTripListIter!=tripList.end();tmpTripListIter++){
		tmpTripPntr = NULL;
		tmpTripPntr = *tmpTripListIter;
		tmpTripPntr->parallelize(_numThreads);
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void		trip::initializeTrip(string _tmpIn){
	string			buf;
	vector<string>	tokens;
	int				tmpInt;
	double			tmpDouble;

	stringstream	ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	tripId = "t";
	tripId.append(tokens[0]);
	tripRoute = "r";
	tripRoute.append(tokens[1]);
	tripType = atoi(tokens[2].c_str());
	tmpInt = atoi(tokens[3].c_str());
	tmpDouble = 60*(tmpInt/10000) + (tmpInt%10000)/100 + tmpInt%100/60.0;
	tripStartTime = tmpDouble;
	tripCapacity = atoi(tokens[4].c_str());
	if(capacityConstraint==0){
		tripCapacity = 999999;
	}
	tripShape = tokens[5];
	tripDirection = tokens[6];
}
void		trip::attachStopTime(string _tmpIn){
	string			buf, tmpStopId;
	vector<string>	tokens;
	int				tmpInt;
	double			tmpSchArrival, tmpSchDeparture;

	buf.clear();
	tokens.clear();
	stringstream ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	tmpInt = atoi(tokens[1].c_str());
	tmpSchArrival = 60*(tmpInt/10000) + (tmpInt%10000)/100 + tmpInt%100/60.0;
	tmpInt = atoi(tokens[2].c_str());
	tmpSchDeparture = 60*(tmpInt/10000) + (tmpInt%10000)/100 + tmpInt%100/60.0;
	tmpStopId = "s";
	tmpStopId.append(tokens[3]);

	tripStops.push_back(tmpStopId);
	tripSchDeparture.push_back(tmpSchArrival);
	tripSchArrival.push_back(tmpSchDeparture);
	tripResidualCapacity.push_back(tripCapacity);
}
void		trip::calculateHeadway(){
	int			i;
	string		tmpStop, tmpSeq;
	double		tmpArrival, tmpPrevDeparture, tmpHeadway;

	for(i=0;i<tripStops.size();i++){
		tmpStop = tripStops[i];
		tmpArrival = tripSchArrival[i];
		tmpPrevDeparture = stopSet[tmpStop]->getPrevTripDeparture(tripRoute, tripDirection, tmpArrival);
		tmpHeadway = tmpArrival - tmpPrevDeparture;
		if(tmpPrevDeparture==0){
			tmpHeadway = 60.0;
		}
		tripSchHeadway.push_back(tmpHeadway);
	}
}
void		trip::parallelize(int _numThreads){
	int		tmpCntr;
	for(tmpCntr=0;tmpCntr<_numThreads;tmpCntr++){
		tripUsedBefore.push_back(0);
	}
}
string		trip::getTripId(){
	return this->tripId;
}
string		trip::getRouteId(){
	return this->tripRoute;
}
string		trip::getShapeId(){
	return this->tripShape;
}
int			trip::getRouteType(){
	return this->tripType;
}
string		trip::getDirection(){
	return this->tripDirection;
}
int			trip::getMaxSequence(){
	return this->tripStops.size();
}
string		trip::getStop(int _seq){
	return this->tripStops[_seq-1];
}
double		trip::getSchArrival(int _seq){
	return this->tripSchArrival[_seq-1];
}
double		trip::getSchDeparture(int _seq){
	return this->tripSchDeparture[_seq-1];
}
double		trip::getSchHeadway(int _seq){
	return this->tripSchHeadway[_seq-1];
}
double		trip::getSchDepartureByStop(string _stop){
	int			i;
	for(i=0;i<tripStops.size();i++){
		if(tripStops[i]==_stop){
			return tripSchDeparture[i];
		}
	}
}
double		trip::getSchArrivalByStop(string _stop){
	int			i;
	for(i=tripStops.size()-1;i>=0;i--){
		if(tripStops[i]==_stop){
			return tripSchArrival[i];
		}
	}
}
void		trip::resetTripUsedBefore(int _threadId){
	this->tripUsedBefore[_threadId] = 0;
}
void		trip::setTripUsedBefore(int _threadId){
	this->tripUsedBefore[_threadId] = 1;
}
int			trip::getTripUsedBefore(int _threadId){
	return this->tripUsedBefore[_threadId];
}
int			trip::getResidualCapacity(int _seq){
	return this->tripResidualCapacity[_seq-1];
}
//Simulation/////////////////////////////////////////////////////////////////////////////////////////////
void		trip::resetTripForSimulation(){
	stopIndex = 0;
	passengers.clear();
	noOfBoardings.clear();
	noOfAlightings.clear();
	noOfOnBoards.clear();
	dwellTimes.clear();
	tripResidualCapacity.clear();
}
int			trip::getStopIndex(){
	return this->stopIndex;
}
int			trip::getTripStartTime(){
	return this->tripStartTime;
}
int			trip::getTripCapacity(){
	return this->tripCapacity;
}
int			trip::getFreeCapacity(){
	int tmpFreeCapacity;
	tmpFreeCapacity = tripCapacity - passengers.size();
	return	tmpFreeCapacity;
}
int			trip::getNoOfOnBoardPassengers(){
	return passengers.size();
}
int			trip::getNoOfAlightings(){
	if(noOfAlightings.size()-1<stopIndex){
		cout <<"Error - noOfAlightings";
	}
	return noOfAlightings[stopIndex];
}
int			trip::getNoOfBoardings(){
	if(noOfBoardings.size()-1<stopIndex){
		cout <<"Error - noOfBoardings";
	}
	return noOfBoardings[stopIndex];
}
void		trip::addPassenger(string _passengerId){
	passengers.push_back(_passengerId);
}
void		trip::removePassenger(int _passengerCntr){
	list<string>::iterator		tmpPassengerIter;
	tmpPassengerIter = passengers.begin();
	advance(tmpPassengerIter,_passengerCntr);
	passengers.erase(tmpPassengerIter);
}
string		trip::getPassengerId(int _passengerCntr){
	list<string>::iterator		tmpPassengerIter;
	tmpPassengerIter = passengers.begin();
	advance(tmpPassengerIter,_passengerCntr);
	return *tmpPassengerIter;
}
void		trip::increaseStopIndex(){
	stopIndex = stopIndex + 1;
}
int			trip::checkMissing(string _boardingStop){
	int			i, tmpMissing;
	tmpMissing = 1;
	for(i=tripStops.size()-1;i>=stopIndex;i--){
		if(tripStops[i] == _boardingStop){
			tmpMissing = 0;
			break;
		}
	}
	return 	tmpMissing;
}
string		trip::getCurrentStop(){
	return this->tripStops[stopIndex];
}
double		trip::getCurrentScheduledArrival(){
	return this->tripSchArrival[stopIndex];
}
double		trip::getCurrentScheduledDeparture(){
	return this->tripSchDeparture[stopIndex];
}
void		trip::setBoardings(int _boardings){
	noOfBoardings.push_back(_boardings);
}
void		trip::setAlightings(int _alightings){
	noOfAlightings.push_back(_alightings);
}
void		trip::setOnBoards(int _onBoards){
	noOfOnBoards.push_back(_onBoards);
}
int			trip::getBoardings(int _seq){
	return this->noOfBoardings[_seq-1];
}
int			trip::getAlightings(int _seq){
	return this->noOfAlightings[_seq-1];
}
int		trip::getVehLoad(int _seq){
	return this->noOfOnBoards[_seq-1];
}
void		trip::setDwellTime(int _dwellTime){
	dwellTimes.push_back(_dwellTime);
}
int		trip::getDwellTime(int _seq){
	return this->dwellTimes[_seq-1];
}
void		trip::setResidualCapacity(int _residualCapacity){
	tripResidualCapacity.push_back(tripCapacity - passengers.size());
}
int         trip::getTripLength(){
    return this->tripStops.size();
}