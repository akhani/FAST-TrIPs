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
#include <omp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <queue>
#include <math.h>

#include <stdlib.h>
using namespace std;
double		inVehTimeEqv, waitingEqv, originWalkEqv, destinationWalkEqv, transferWalkEqv, transferPenalty, scheduleDelayEqv, fare, VOT, theta, capacityConstraint,
			railInVehTimeEqv;

map<string,double>		availableCapacity;
map<string,int>		availableCapacity2;

string getTime(){
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  return asctime(timeinfo);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
class	stop{
protected:
	string					stopId;
	string					stopName;
	string					stopDescription;
	double					stopLat;
	double					stopLon;
	int						stopPassengerCap;

	int						stopTransferStop;

	vector<string>			stopTransfers;
	vector<double>			stopTransferDists;
	vector<double>			stopTransferTimes;
	set<string>				stopRouteSet;
	vector<string>			stopRoutes;
	vector<string>			stopDirections;
	vector<string>			stopTrips;
	vector<int>				stopSequences;
	vector<double>			stopSchArrivals;
	vector<double>			stopSchDepartures;

	vector<string>			stopTazs;
	vector<double>			stopAccessDistances;
	vector<double>			stopAccessTimes;
	vector<int>			stopAccessTypes;

	vector<double>			stopLabels;
	vector<double>			stopArrivals;
	vector<double>			stopDepartures;
	vector<string>			stopArrivalModes;
	vector<string>			stopDepartureModes;
	vector<string>			stopPredecessors;
	vector<string>			stopSuccessors;
	vector<int>			permanentLabels;

	//For TBHP
	double			        stopStrategyLebel;
	int				strategyPermanentLabel;
	vector<double>			stopCosts;
	double			        stopNonWalkLabel;

	//For Simulation
	list<string>			waitingPassengers;
        

public:
	stop(){}
	~stop(){}
	void				initializeStop(string _tmpIn);
	void				attachTransfer(string _tmpIn);
	void				attachRoute(string _routeId, string _dir);
	void				attachStopTime(string _tmpIn);
	void				attachTaz(string _tmpIn);

	void				setTransferStop(int _transferStop);

	//For TBSP
	string				getStopId();
	int					getNumRoutes();
	int					getNumTrips();
	double				getPrevTripDeparture(string _route, string _dir, double _time);
	int					getTransferStop();
	int					getNumTransfers();
	void				parallelize(int _numThreads);
	void				resetStop(int _threadId);
	void				forwardUpdate(double _label, double _arrival, string _arrivalMode, string _predecessor, int _threadId);
	void				backwardUpdate(double _label, double _departure, string _departureMode, string _successor, int _threadId);
	double				getLabel(int _threadId);
	string				getTransfer(int _i);
	double				getTransferTime(int _i);
	double				getDeparture(int _threadId);
	double				getArrival(int _threadId);
	string				getRouteId(int _i);
	string				getForwardAccessibleTrips(double _arrival, int _timeBuffer);
	string				getBackwardAccessibleTrips(double _departure, int _timeBuffer);
	void				setPermanentLabel(int _label, int _threadId);
	int					getPermanentLabel(int _threadId);
	string				getTaz();
	string				printPath(int _threadId);

	//For TBHP
	void				resetStopStrategy();
	void				forwardStrategyUpdate(double _label, double _arrival, string _arrivalMode, string _predecessor, double _cost, double _departure);
	void				backwardStrategyUpdate(double _label, double _departure, string _departureMode, string _successor, double _cost, double _arrival);
	double				getStrategyLabel();
	double				getStrategyEarliestArrival();
	double				getStrategyLatestArrival();
	double				getStrategyEarliestDeparture();
	double				getStrategyLatestDeparture();
	void				setStrategyPermanentLabel();
	int					getStrategyPermanentLabel();
	double				getForwardNonWalkLabel();
	double				getBackwardNonWalkLabel();
	string				getForwardAssignedAlternative(double _departureTime, string _lastTrip);
	string				getBackwardAssignedAlternative(double _arrivalTime, string _lastTrip);

	//Path Backtracking
	string				getPredecessor(int _threadId);
	string				getSuccessor(int _threadId);
	string				getDepartureTripId(int _threadId);
	string				getArrivalTripId(int _threadId);

	//For Simulation
	void				resetStopForSimulation();
	int					getNoOfWaitingPassenegrs();
	void				addPassenger(string _passengerId);
	void				removePassenger(int _passCntr);
	string				getPassengerId(int _passCntr);

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
map<string,stop*>					stopSet;
list<stop*>							stopList;
map<string,double>					transferTimes;

int			readStops();
int			readTransfers();
int			defineTransferStops();
int			parallelizeStops(int _numThreads);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int		readStops(){
    cout <<"reading stops:\t\t\t";
    
	string			tmpIn, tmpStopId, buf;
	vector<string>	tokens;
	stop*			tmpStopPntr;

	ifstream inFile;
	inFile.open("ft_input_stops.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_stops.dat";
		exit(1);
	}

	//getline(inFile,tmpIn);
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		if (ss >> buf){
			tokens.push_back(buf);
		}
		tmpStopId = "s";
		tmpStopId.append(tokens[0]);
		tmpStopPntr = NULL;
		tmpStopPntr = new stop;
		stopSet[tmpStopId] = tmpStopPntr;
		stopSet[tmpStopId]->initializeStop(tmpIn);
		stopList.push_back(tmpStopPntr);
	}
	inFile.close();
	return stopSet.size();
}
int		readTransfers(){
    cout <<"reading transfers:\t\t";

	string			tmpIn, tmpStopId, buf;
	int				numTransfers;
	vector<string>	tokens;

	ifstream inFile;
	inFile.open("ft_input_transfers.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_transfers.dat";
		exit(1);   // call system to stop
	}
	//getline(inFile,tmpIn);
	numTransfers = 0;
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		while (ss >> buf){
			tokens.push_back(buf);
		}
		tmpStopId = "s";
		tmpStopId.append(tokens[0]);
		stopSet[tmpStopId]->attachTransfer(tmpIn);
		transferTimes[tmpStopId+",s"+tokens[1]] = atof(tokens[2].c_str())/3.0*60;
		numTransfers++;
	}
	inFile.close();
	return numTransfers;
}
int		defineTransferStops(){
    cout <<"defining transfer stops:\t";

        int						tmpNumRoutes, tmpNumTransfers, numTransferStops;
	list<stop*>::iterator	tmpStopListIter;
	stop*					tmpStopPntr;

	numTransferStops = 0;
	for(tmpStopListIter=stopList.begin();tmpStopListIter!=stopList.end();tmpStopListIter++){
		tmpStopPntr = NULL;
		tmpStopPntr = *tmpStopListIter;
		tmpNumRoutes = tmpStopPntr->getNumRoutes();
		tmpNumTransfers = tmpStopPntr->getNumTransfers();
		tmpStopPntr->setTransferStop(0);
		if(tmpNumRoutes>1){
			tmpStopPntr->setTransferStop(1);
			numTransferStops++;
		}else if(tmpNumTransfers>0){
			tmpStopPntr->setTransferStop(1);
			numTransferStops++;
		}
	}
	return numTransferStops;
}

int		parallelizeStops(int _numThreads){
	list<stop*>::iterator	tmpStopListIter;
	stop*					tmpStopPntr;

	for(tmpStopListIter=stopList.begin();tmpStopListIter!=stopList.end();tmpStopListIter++){
		tmpStopPntr = NULL;
		tmpStopPntr = *tmpStopListIter;
		tmpStopPntr->parallelize(_numThreads);
	}

	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void		stop::initializeStop(string _tmpIn){
	string			buf, tmpStr;
	vector<string>	tokens;

	buf.clear();
	tokens.clear();
	stringstream	ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	stopId = "s";
	stopId.append(tokens[0]);
	stopName = tokens[1];
	stopDescription = tokens[2];
	stopLat = atof(tokens[3].c_str());
	stopLon = atof(tokens[4].c_str());
	stopPassengerCap = atoi(tokens[5].c_str());
}
void		stop::attachTransfer(string _tmpIn){
	string			buf, tmpTransferStop;
	vector<string>	tokens;
	double			tmpTransferDist, tmpTransferTime;

	buf.clear();
	tokens.clear();
	stringstream	ss1(_tmpIn);
	while (ss1 >> buf){
		tokens.push_back(buf);
	}
	tmpTransferStop = "s";
	tmpTransferStop.append(tokens[1]);
	tmpTransferDist = atof(tokens[2].c_str());
	tmpTransferTime = 60*tmpTransferDist/3.0;
	stopTransfers.push_back(tmpTransferStop);
	stopTransferDists.push_back(tmpTransferDist);
	stopTransferTimes.push_back(tmpTransferTime);
}
void		stop::attachRoute(string _routeId, string _dir){
	stopRoutes.push_back(_routeId);
	stopRouteSet.insert(_routeId);
	stopDirections.push_back(_dir);
}
void		stop::attachStopTime(string _tmpIn){
	string			buf, tmpTrip;
	vector<string>	tokens;
	int				tmpSequence, tmpInt;
	double			tmpSchArrival, tmpSchDeparture;

	buf.clear();
	tokens.clear();
	stringstream	ss1(_tmpIn);
	while (ss1 >> buf){
		tokens.push_back(buf);
	}
	tmpTrip = "t";
	tmpTrip.append(tokens[0]);
	tmpSequence = atoi(tokens[4].c_str());
	tmpInt = atoi(tokens[1].c_str());
	tmpSchArrival = 60*(tmpInt/10000) + (tmpInt%10000)/100 + tmpInt%100/60.0;
	tmpInt = atoi(tokens[2].c_str());
	tmpSchDeparture = 60*(tmpInt/10000) + (tmpInt%10000)/100 + tmpInt%100/60.0;

	stopTrips.push_back(tmpTrip);
	stopSequences.push_back(tmpSequence);
	stopSchArrivals.push_back(tmpSchArrival);
	stopSchDepartures.push_back(tmpSchDeparture);
}
void		stop::attachTaz(string _tmpIn){
	string			buf, tmpTaz;
	int			tmpAccessType;
	vector<string>	tokens;
	double			tmpAccessDist, tmpAccessTime;

	buf.clear();
	tokens.clear();
	stringstream	ss(_tmpIn);
	while (ss >> buf){
		tokens.push_back(buf);
	}
	tmpTaz = "t";
	tmpTaz.append(tokens[0]);
	tmpAccessDist = atof(tokens[2].c_str());
	tmpAccessTime = atof(tokens[3].c_str());
	tmpAccessType = atoi(tokens[4].c_str());
	stopTazs.push_back(tmpTaz);
	stopAccessDistances.push_back(tmpAccessDist);
	stopAccessTimes.push_back(tmpAccessTime);
	stopAccessTypes.push_back(tmpAccessType);
}
void		stop::setTransferStop(int _transferStop){
	stopTransferStop = _transferStop;
}
string		stop::getStopId(){
	return this->stopId;
}
int			stop::getNumRoutes(){
	return this->stopRouteSet.size();
}
int			stop::getNumTrips(){
	return this->stopTrips.size();
}
double		stop::getPrevTripDeparture(string _route, string _dir, double _time){
	int			i;
	string		tmpTrip, tmpRoute, tmpDir;
	double		tmpSchDeparture, tmpLatestDeparture;

	tmpLatestDeparture = 0.0;
	for(i=0;i<stopTrips.size();i++){
		tmpTrip = stopTrips[i];
		tmpRoute = stopRoutes[i];
		tmpDir = stopDirections[i];
		if(tmpRoute==_route && tmpDir==_dir){
			tmpSchDeparture = stopSchDepartures[i];
			if(tmpSchDeparture<_time && tmpSchDeparture>tmpLatestDeparture){
				tmpLatestDeparture = tmpSchDeparture;
			}
		}
	}
	return tmpLatestDeparture;
}
int			stop::getTransferStop(){
	return this->stopTransferStop;
}
int			stop::getNumTransfers(){
	return this->stopTransfers.size();
}
void		stop::parallelize(int _numThreads){
	int		tmpCntr;
	for(tmpCntr=0;tmpCntr<_numThreads;tmpCntr++){
		stopLabels.push_back(0);
		stopArrivals.push_back(0);
		stopDepartures.push_back(0);
		stopArrivalModes.push_back("-101");
		stopDepartureModes.push_back("-101");
		stopPredecessors.push_back("-101");
		stopSuccessors.push_back("-101");
		permanentLabels.push_back(0);
	}
}
void		stop::resetStop(int _threadId){
	stopLabels[_threadId] = 999999;
	stopArrivals[_threadId] = 999999;
	stopDepartures[_threadId] = -999999;
	stopArrivalModes[_threadId] = "-101";
	stopDepartureModes[_threadId] = "-101";
	stopPredecessors[_threadId] = "-101";
	stopSuccessors[_threadId] = "-101";
	permanentLabels[_threadId] = 0;

}
void		stop::forwardUpdate(double _label, double _arrival, string _arrivalMode, string _predecessor, int _threadId){
	stopLabels[_threadId] = _label;
	stopArrivals[_threadId] = _arrival;
	stopArrivalModes[_threadId] = _arrivalMode;
	stopPredecessors[_threadId] = _predecessor;
}
void		stop::backwardUpdate(double _label, double _departure, string _departureMode, string _successor, int _threadId){
	stopLabels[_threadId] = _label;
	stopDepartures[_threadId] = _departure;
	stopDepartureModes[_threadId] = _departureMode;
	stopSuccessors[_threadId] = _successor;
}
double		stop::getLabel(int _threadId){
	return this->stopLabels[_threadId];
}
string		stop::getTransfer(int _i){
	return this->stopTransfers[_i];
}
double		stop::getTransferTime(int _i){
	return this->stopTransferTimes[_i];
}
double		stop::getDeparture(int _threadId){
	return this->stopDepartures[_threadId];
}
double		stop::getArrival(int _threadId){
	return this->stopArrivals[_threadId];
}
string		stop::getRouteId(int _i){
	set<string>::iterator		tmpRouteSetIter;
	tmpRouteSetIter = this->stopRouteSet.begin();
	advance(tmpRouteSetIter, _i);
	return *tmpRouteSetIter;
}
string		stop::getForwardAccessibleTrips(double _arrival, int _timeBuffer){
	int			i, tmpSeq;
	double		tmpSchDeparture;
	string		tmpTrip, tmpAccessibleTrips;
	char		chr[99];

	tmpAccessibleTrips = "";
	for(i=0;i<stopTrips.size();i++){
		tmpTrip = stopTrips[i];
		tmpSeq = stopSequences[i];
		tmpSchDeparture = stopSchDepartures[i];
		if(tmpSchDeparture>_arrival && tmpSchDeparture<_arrival+_timeBuffer){
			tmpAccessibleTrips.append(tmpTrip);
			tmpAccessibleTrips.append(" ");
			sprintf(chr,"%d",tmpSeq);
            tmpAccessibleTrips.append(string(chr));
			tmpAccessibleTrips.append(" ");
		}
	}
	return tmpAccessibleTrips;
}
string		stop::getBackwardAccessibleTrips(double _departure, int _timeBuffer){
	int			i, tmpSeq;
	double		tmpSchArrival;
	string		tmpTrip, tmpAccessibleTrips;
	char		chr[99];

	tmpAccessibleTrips = "";
	for(i=0;i<stopTrips.size();i++){
		tmpTrip = stopTrips[i];
		tmpSeq = stopSequences[i];
		tmpSchArrival = stopSchArrivals[i];
		if(tmpSchArrival<_departure && tmpSchArrival>_departure-_timeBuffer){
			tmpAccessibleTrips.append(tmpTrip);
			tmpAccessibleTrips.append(" ");
            sprintf(chr,"%d",tmpSeq);
            tmpAccessibleTrips.append(string(chr));
			tmpAccessibleTrips.append(" ");
		}
	}
	return tmpAccessibleTrips;
}
void		stop::setPermanentLabel(int _label, int _threadId){
	permanentLabels[_threadId] = _label;
}
int			stop::getPermanentLabel(int _threadId){
	return this->permanentLabels[_threadId];
}
string		stop::getTaz(){
	return this->stopTazs[0];
}
string		stop::getPredecessor(int _threadId){
	return this->stopPredecessors[_threadId];
}
string		stop::getSuccessor(int _threadId){
	return this->stopSuccessors[_threadId];
}
string		stop::getDepartureTripId(int _threadId){
	if(stopDepartureModes.size()>0){
		return this->stopDepartureModes[_threadId];
	}else{
		return "";
	}
}
string		stop::getArrivalTripId(int _threadId){
	if(stopArrivalModes.size()>0){
		return this->stopArrivalModes[_threadId];
	}else{
		return "";
	}
}
string		stop::printPath(int _threadId){
	cout <<stopId<<"\t"<<stopLabels[_threadId];
	cout <<"\t\t"<<stopDepartures[_threadId]<<"\t"<<stopSuccessors[_threadId]<<"\t"<<stopDepartureModes[_threadId]<<endl;
	return "";
}
/////////////////////////////////////////////////////////
void		stop::resetStopStrategy(){
	stopStrategyLebel = 999999;
	stopArrivals.clear();
	stopDepartures.clear();
	stopArrivalModes.clear();
	stopDepartureModes.clear();
	stopPredecessors.clear();
	stopSuccessors.clear();
	stopCosts.clear();
    strategyPermanentLabel = 0;
}
void		stop::forwardStrategyUpdate(double _label, double _arrival, string _arrivalMode, string _predecessor, double _cost, double _departure){
	stopStrategyLebel = _label;
	stopArrivals.push_back(_arrival);
	stopArrivalModes.push_back(_arrivalMode);
	stopPredecessors.push_back(_predecessor);
	stopCosts.push_back(_cost);
	stopDepartures.push_back(_departure);
}
void		stop::backwardStrategyUpdate(double _label, double _departure, string _departureMode, string _successor, double _cost, double _arrival){
	stopStrategyLebel = _label;
	stopDepartures.push_back(_departure);
	stopDepartureModes.push_back(_departureMode);
	stopSuccessors.push_back(_successor);
	stopCosts.push_back(_cost);
	stopArrivals.push_back(_arrival);
}
double		stop::getStrategyLabel(){
	return this->stopStrategyLebel;
}
double		stop::getStrategyEarliestArrival(){
	double							tmpMinValue;
	vector<double>::iterator		tmpVectorIter;

	tmpMinValue = 999999;
	for(tmpVectorIter=stopArrivals.begin();tmpVectorIter!=stopArrivals.end();tmpVectorIter++){
		if(*tmpVectorIter < tmpMinValue){
			tmpMinValue = *tmpVectorIter;
		}
	}
	return tmpMinValue;
}
double		stop::getStrategyLatestArrival(){
	double							tmpMaxValue;
	vector<double>::iterator		tmpVectorIter;

	tmpMaxValue = -999999;
	for(tmpVectorIter=stopArrivals.begin();tmpVectorIter!=stopArrivals.end();tmpVectorIter++){
		if(*tmpVectorIter > tmpMaxValue){
			tmpMaxValue = *tmpVectorIter;
		}
	}
	return tmpMaxValue;
}
double		stop::getStrategyEarliestDeparture(){
	double							tmpMinValue;
	vector<double>::iterator		tmpVectorIter;

	tmpMinValue = 999999;
	for(tmpVectorIter=stopDepartures.begin();tmpVectorIter!=stopDepartures.end();tmpVectorIter++){
		if(*tmpVectorIter < tmpMinValue){
			tmpMinValue = *tmpVectorIter;
		}
	}
	return tmpMinValue;
}
double		stop::getStrategyLatestDeparture(){
	double							tmpMaxValue;
	vector<double>::iterator		tmpVectorIter;

	tmpMaxValue = -999999;
	for(tmpVectorIter=stopDepartures.begin();tmpVectorIter!=stopDepartures.end();tmpVectorIter++){
		if(*tmpVectorIter > tmpMaxValue){
			tmpMaxValue = *tmpVectorIter;
		}
	}
	return tmpMaxValue;
}
void		stop::setStrategyPermanentLabel(){
	strategyPermanentLabel = 1;
}
int			stop::getStrategyPermanentLabel(){
	return this->strategyPermanentLabel;
}
double	stop::getForwardNonWalkLabel(){
	int		i;
	double  tmpNonWalkLabel;

	tmpNonWalkLabel = 0;
	for(i=0;i<stopArrivals.size();i++){
		if (stopArrivalModes[i].substr(0,1)=="t"){
			tmpNonWalkLabel = tmpNonWalkLabel + exp(-theta*stopCosts[i]);
		}
	}
	if(tmpNonWalkLabel==0){
		return 999999;
	}else{
		tmpNonWalkLabel = -1.0/theta*log(tmpNonWalkLabel);
		return tmpNonWalkLabel;
	}
}
double	stop::getBackwardNonWalkLabel(){
	int		i;
	double  tmpNonWalkLabel;

	tmpNonWalkLabel = 0;
	for(i=0;i<stopDepartures.size();i++){
		if (stopDepartureModes[i].substr(0,1)=="t"){
			tmpNonWalkLabel = tmpNonWalkLabel + exp(-theta*stopCosts[i]);
		}
	}
	if(tmpNonWalkLabel==0){
		return 999999;
	}else{
		tmpNonWalkLabel = -1.0/theta*log(tmpNonWalkLabel);
		return tmpNonWalkLabel;
	}
}
string		stop::getForwardAssignedAlternative(double _departureTime, string _lastTrip){
	int				i, j, tmpAltProb, tmpMaxProb, tmpRandNum;
	double			tmpLogsum;
	vector<string>	tmpAlternatives;
	vector<int>		tmpAltProbabilities;
	char			chr1[99], chr2[99];

	if(stopArrivals.size()==0){
		//cout <<"SC1"<<endl;
		return "-101";
	}
	tmpLogsum = 0;
	for(i=0;i<stopArrivals.size();i++){
		if((stopArrivalModes[i]=="Transfer" || stopArrivalModes[i]=="Access") && (_lastTrip=="Transfer" || _lastTrip=="Egress")){
			//cout<<"1";
			continue;
		}
		if(stopArrivals[i] > _departureTime){
			//cout<<"2";
			continue;
		}
		tmpLogsum = tmpLogsum + exp(-theta*stopCosts[i]);
	}
	if(tmpLogsum==0){
		//cout <<"SC2"<<endl;
		return "-101";
	}
	j=-1;
	tmpMaxProb = 0;
	for(i=0;i<stopArrivals.size();i++){
		if((stopArrivalModes[i]=="Transfer" || stopArrivalModes[i]=="Access") && (_lastTrip=="Transfer" || _lastTrip=="Egress")){
			//cout<<"3";
			continue;
		}
		if(stopArrivals[i] > _departureTime){
			//cout<<"4";
			continue;
		}
		tmpAltProb = int(1000*(exp(-theta*stopCosts[i])/tmpLogsum));
		if(tmpAltProb < 1){
			//cout<<"5";
			continue;
		}
		j++;
		if(j>0)	tmpAltProb = tmpAltProb + tmpAltProbabilities[j-1];
		sprintf(chr1,"%d",int(100*stopArrivals[i]));
        sprintf(chr2,"%d",int(100*stopDepartures[i]));
        tmpAlternatives.push_back(stopPredecessors[i]+"\t"+stopArrivalModes[i]+"\t"+string(chr1)+"\t"+string(chr2));
        tmpAltProbabilities.push_back(tmpAltProb);
		tmpMaxProb = tmpAltProb;
	}
	if(tmpMaxProb<1){
		//cout <<"SC3"<<endl;
		return "-101";
	}
	tmpRandNum = rand()%tmpMaxProb;
	for(j=0;j<tmpAlternatives.size();j++){
		if(tmpRandNum <= tmpAltProbabilities[j]){
			return tmpAlternatives[j];
		}
	}
	//cout <<"SC4"<<endl;
	return "-101";
}
string		stop::getBackwardAssignedAlternative(double _arrivalTime, string _lastTrip){
	int				i, j, tmpAltProb, tmpMaxProb, tmpRandNum;
	double			tmpLogsum;
	vector<string>	tmpAlternatives;
	vector<int>		tmpAltProbabilities;
	char			chr1[99], chr2[99];

	if(stopDepartures.size()==0){
		//cout <<"SC1"<<endl;
		return "-101";
	}
	tmpLogsum = 0;
	for(i=0;i<stopDepartures.size();i++){
		if((stopDepartureModes[i]=="Transfer" || stopDepartureModes[i]=="Egress") && (_lastTrip=="Transfer" || _lastTrip=="Access")){
			//cout<<"1";
			continue;
		}
		if(stopDepartures[i] < _arrivalTime){
			//cout<<"2";
			continue;
		}
		tmpLogsum = tmpLogsum + exp(-theta*stopCosts[i]);
	}
	if(tmpLogsum==0){
		//cout <<"SC2"<<endl;
		return "-101";
	}
	j=-1;
	tmpMaxProb = 0;
	for(i=0;i<stopDepartures.size();i++){
		if((stopDepartureModes[i]=="Transfer" || stopDepartureModes[i]=="Egress") && (_lastTrip=="Transfer" || _lastTrip=="Access")){
			//cout<<"3";
			continue;
		}
		if(stopDepartures[i] < _arrivalTime){
			//cout<<"4";
			continue;
		}
		tmpAltProb = int(1000*(exp(-theta*stopCosts[i])/tmpLogsum));
		if(tmpAltProb < 1){
			//cout<<"5";
			continue;
		}
		j++;
		if(j>0)	tmpAltProb = tmpAltProb + tmpAltProbabilities[j-1];
		sprintf(chr1,"%d",int(100*stopDepartures[i]));
        sprintf(chr2,"%d",int(100*stopArrivals[i]));
        tmpAlternatives.push_back(stopSuccessors[i]+"\t"+stopDepartureModes[i]+"\t"+string(chr1)+"\t"+string(chr2));
		tmpAltProbabilities.push_back(tmpAltProb);
		tmpMaxProb = tmpAltProb;
	}
	if(tmpMaxProb<1){
		//cout <<"SC3"<<endl;
		return "-101";
	}
	tmpRandNum = rand()%tmpMaxProb;
	for(j=0;j<tmpAlternatives.size();j++){
		if(tmpRandNum <= tmpAltProbabilities[j]){
			return tmpAlternatives[j];
		}
	}
	//cout <<"SC4"<<endl;
	return "-101";
}
/////////////////////////////////////////////////////////////////////////////////////////////
void	stop::resetStopForSimulation(){
	waitingPassengers.clear();
}

int		stop::getNoOfWaitingPassenegrs(){
	return	waitingPassengers.size();
}
void	stop::addPassenger(string _passengerId){
	waitingPassengers.push_back(_passengerId);
}
void	stop::removePassenger(int _passCntr){
	list<string>::iterator			tmpPassengerIter;
	tmpPassengerIter = waitingPassengers.begin();
	advance(tmpPassengerIter, _passCntr);
	waitingPassengers.erase(tmpPassengerIter);
}
string	stop::getPassengerId(int _passCntr){
	list<string>::iterator			tmpPassengerIter;
	tmpPassengerIter = waitingPassengers.begin();
	advance(tmpPassengerIter, _passCntr);
	return *tmpPassengerIter;
}
