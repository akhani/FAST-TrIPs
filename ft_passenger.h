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

    //path-based assignment
    map<string,int>						pathSet;
    map<string,int>::iterator			pathIter;
    map<string,double>					pathUtility;
	map<string,double>::iterator		pathIter2;
    map<string,int>                     pathCapacity;
    
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
	void			decreasePathIndex();
	void			setEndTime(double _endTime);
	double			getEndTime();
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
    string			getLastAlightingStop();

	void			calculateExperiencedCost();
	double			getExperiencedCost();

    //path-based assignment
    void            resetPaths();
    void            addPaths(string _tmpPath);
    void            analyzePaths();
    string          assignPath();

    int				getNumUnlinkedTrips();
    double			getAccessTime();
    double			getEgressTime();
    string			getUnlinkedTrip(int _i);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
map<string,passenger*>				passengerSet;
list<passenger*>					passengerList;
map<string,string>					tazTimeSet;
list<passenger*>					passengers2transfer;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
int			readPassengers(){
    cout <<"reading demand:\t\t\t";

	string			tmpIn, buf, tmpPassengerId, tmpMode;
	vector<string>	tokens;
	passenger*		tmpPassengerPntr;

	ifstream inFile;
	inFile.open("ft_input_demand.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_demand.dat";
		exit(1);
	}
	//getline(inFile,tmpIn);
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
    cout <<"reading paths:\t\t\t";

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
	//getline(inFile,tmpIn);
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
	char				chr[999];

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
void		passenger::decreasePathIndex(){
	passengerPathIndex = passengerPathIndex - 1;
}
void		passenger::setEndTime(double _endTime){
	endTime = _endTime;
}
double		passenger::getEndTime(){
	return this->endTime;
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
	char	chr[999];
	for(i=0;i<experiencedArrivalTimes.size();i++){
		if(i!=0){
			tmpArrivalTimes.append(",");
			tmpBoardingTimes.append(",");
			tmpAlightingTimes.append(",");
		}
        sprintf(chr,"%d",int(100*experiencedArrivalTimes[i])/100);
        tmpArrivalTimes.append(string(chr));
		tmpArrivalTimes.append(".");
		if(int(100*experiencedArrivalTimes[i])%100<10)	tmpArrivalTimes.append("0");
        sprintf(chr,"%d",int(100*experiencedArrivalTimes[i])%100);
        tmpArrivalTimes.append(string(chr));

		sprintf(chr,"%d",int(100*experiencedBoardingTimes[i])/100);
        tmpBoardingTimes.append(string(chr));
        tmpBoardingTimes.append(".");
		if(int(100*experiencedBoardingTimes[i])%100<10)	tmpBoardingTimes.append("0");
        sprintf(chr,"%d",int(100*experiencedBoardingTimes[i])%100);
		tmpBoardingTimes.append(string(chr));

		sprintf(chr,"%d",int(100*experiencedAlightingTimes[i])/100);
        tmpAlightingTimes.append(string(chr));
        tmpAlightingTimes.append(".");
		if(int(100*experiencedAlightingTimes[i])%100<10)	tmpAlightingTimes.append("0");
		sprintf(chr,"%d",int(100*experiencedAlightingTimes[i])%100);
        tmpAlightingTimes.append(string(chr));
    }
	tmpExperiencedPath = passengerId.substr(1,999);
	tmpExperiencedPath.append("\t");
    sprintf(chr,"%d",passengerMode);
    tmpExperiencedPath.append(string(chr));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(passengerOriginTAZ.substr(1,999));
	tmpExperiencedPath.append("\t");
	tmpExperiencedPath.append(passengerDestinationTAZ.substr(1,999));
	tmpExperiencedPath.append("\t");
    sprintf(chr,"%d",int(100*startTime)/100);
	tmpExperiencedPath.append(string(chr));
    tmpExperiencedPath.append(".");
	if(int(100*startTime)%100<10)	tmpExperiencedPath.append("0");
    sprintf(chr,"%d",int(100*startTime)%100);
	tmpExperiencedPath.append(string(chr));
    tmpExperiencedPath.append("\t");
    sprintf(chr,"%d",int(100*endTime)/100);
	tmpExperiencedPath.append(string(chr));
    tmpExperiencedPath.append(".");
	if(int(100*endTime)%100<10)	tmpExperiencedPath.append("0");
    sprintf(chr,"%d",int(100*endTime)%100);
	tmpExperiencedPath.append(string(chr));
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
string		passenger::getLastAlightingStop(){
	if(passengerPathIndex>0){
		return	this->alightingStops[passengerPathIndex];
	}else{
		return	"Origin";
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
///////////////////////////////////////////////
void    passenger::resetPaths(){
    pathSet.clear();
}
void    passenger::addPaths(string _tmpPath){
    pathIter = pathSet.find(_tmpPath);
    if (pathIter==pathSet.end())
        pathSet[_tmpPath] = 1;
    else{
        pathSet[_tmpPath] = pathSet[_tmpPath] + 1;
    }
}
void    passenger::analyzePaths(){
	//For Choice Set Attributes
    int                         n, routeType;
	string					    buf, buf2;
	vector< vector<string> >	tokens;
	vector<string>			    tokens2;
	vector<string>			    tmpTrips, tmpBoardings, tmpAlightings, tmpWalkings;
	string					    tmpStartTime, tmpFromToAt;
	double					    NTR, IWT, IVT, railIVT, TRT, ACD, EGD, TRD, ScDelay, FARE, tmpUtility;
	string  					routeId, routeComb, tripComb, transferComb; //transferComb for capacity constraints

    pathUtility.clear();
    pathCapacity.clear();
    for(pathIter=pathSet.begin();pathIter!=pathSet.end();pathIter++){
        buf.clear();
        tokens.clear();
        stringstream ss((*pathIter).first);
        while (ss >> buf){
            stringstream ss2(buf);
            buf2.clear();
            tokens2.clear();
            while (getline(ss2, buf2, ',')){
                tokens2.push_back(buf2);
            }
            tokens.push_back(tokens2);
        }
        tmpStartTime = tokens[0][0];
        tmpTrips = tokens[2];
        tmpBoardings = tokens[1];
        tmpAlightings = tokens[3];
        tmpWalkings = tokens[4];

        routeComb = "";
        tripComb = "";
        NTR = tmpTrips.size() - 1;
        IWT = tripSet[tmpTrips[0]]->getSchDepartureByStop(tmpBoardings[0]) - atof(tmpWalkings[0].c_str()) - atof(tmpStartTime.c_str());
        IWT = roundf(IWT * 100) / 100;
        IVT = 0;
        railIVT = 0;
        TRT = 0;
        ACD = atof(tmpWalkings[0].c_str());
        EGD = atof(tmpWalkings[tmpWalkings.size()-1].c_str());
        TRD = 0;
        if(passengerTourHalf==1){
            ScDelay = 0;//passengerPAT - passengerPAT; //should be fixed
        }else{
            ScDelay = 0;//atof(tmpStartTime.c_str()) - passengerPDT;
        }
        FARE = 0;
        pathCapacity[(*pathIter).first] = 1;
        
        for (n=0;n<tmpTrips.size();n++){
            if(routeComb==""){
                routeComb = tripSet[tmpTrips[n]]->getRouteId().substr(1,999);
            }else{
                routeComb = routeComb + "," + tripSet[tmpTrips[n]]->getRouteId().substr(1,999);
            }
            if(tripComb==""){
                tripComb = tmpTrips[n].substr(1,999);
            }else{
                tripComb = tripComb + "," + tmpTrips[n].substr(1,999);
            }

            routeId = tripSet[tmpTrips[n]]->getRouteId();
            routeType = routeSet[routeId]->getRouteType();
            if(routeType==3){
            	IVT = IVT + tripSet[tmpTrips[n]]->getSchArrivalByStop(tmpAlightings[n]) - tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n]);
            	if(tripSet[tmpTrips[n]]->getSchArrivalByStop(tmpAlightings[n]) - tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n]) < 0){
            		cout <<tripSet[tmpTrips[n]]->getSchArrivalByStop(tmpAlightings[n]) - tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n])<<endl;
            	}
            }else if(routeType==0 || routeType==1 || routeType==2){
				railIVT = railIVT + tripSet[tmpTrips[n]]->getSchArrivalByStop(tmpAlightings[n]) - tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n]);
				if(tripSet[tmpTrips[n]]->getSchArrivalByStop(tmpAlightings[n]) - tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n]) < 0){
					cout <<tripSet[tmpTrips[n]]->getSchArrivalByStop(tmpAlightings[n]) - tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n])<<endl;
				}
            }

			if(n!=0 && n!=tmpTrips.size()-1){
                TRT = TRT + tripSet[tmpTrips[n]]->getSchDepartureByStop(tmpBoardings[n]) - tripSet[tmpTrips[n-1]]->getSchArrivalByStop(tmpAlightings[n-1]) - atof(tmpWalkings[n].c_str());
                TRD = TRD + atof(tmpWalkings[n].c_str());
            }
            
            //This is for Austin TX network with fare=$1.0 per boarding without transfer credit.
            //FARE = FARE + 1.0;
            //if ((tripSet[tmpTrips[n]]->getRouteId()).length()>3 && (tripSet[tmpTrips[n]]->getRouteId()).substr(1,1)=="9") FARE = FARE + 1.50;
            //This is for Twin Cities MN network with fare is defined per route type and 2.5 hours transfer credit.
            if(n==0){
                if (routeSet[routeId]->getRouteFare()==-1.0){
                    if (routeType==0) FARE = FARE + 1.75; //LRT
                    if (routeType==2) FARE = FARE + 3.0; //Northstar
                    if (routeType==3) FARE = FARE + 1.75; //Local buses
                    if (routeType==3 && routeId.length()>3) FARE = FARE + 0.5; //Express buses
                    //Shuttle routes
                }else{
                    FARE = routeSet[routeId]->getRouteFare();
                }
            }
            
            if(n==0){
                tmpFromToAt = "Access," + tmpTrips[n] + "," + tmpBoardings[n];
                if(availableCapacity2.find(tmpFromToAt)!=availableCapacity2.end()){
                    if(availableCapacity2[tmpFromToAt]==0){
                        pathCapacity[(*pathIter).first] = 0;
                    }
                }
            }else{
                tmpFromToAt = tmpTrips[n-1] + "," + tmpAlightings[n-1] + "," + tmpTrips[n] + "," + tmpBoardings[n];
                if(availableCapacity2.find(tmpFromToAt)!=availableCapacity2.end()){
                    if(availableCapacity2[tmpFromToAt]==0){
                        pathCapacity[(*pathIter).first] = 0;
                    }
                }
            }
        }
        IVT = roundf(IVT * 100) / 100;
        railIVT = roundf(railIVT * 100) / 100;
        tmpUtility = inVehTimeEqv*IVT + railInVehTimeEqv*railIVT + waitingEqv*(IWT+TRT) + originWalkEqv*ACD + destinationWalkEqv*EGD + transferWalkEqv*TRD + transferPenalty*NTR + scheduleDelayEqv*ScDelay + 60*FARE/VOT;
        tmpUtility = roundf(tmpUtility * 100) / 100;
        pathUtility[(*pathIter).first] = tmpUtility;
        //cout	<<passengerId.substr(1,99)<<"\t"<<(*pathIter).second<<"\t"<<pathUtility[(*pathIter).first];
        //cout  <<"\t"<<NTR<<"\t"<<IWT<<"\t"<<IVT<<"\t"<<TRT<<"\t"<<ACD<<"\t"<<EGD<<"\t"<<TRD<<"\t"<<FARE<<endl;
    }
}
string  passenger::assignPath(){
	int				i, j, tmpAltProb, tmpMaxProb, tmpRandNum;
    double          tmpLogsum;
	vector<string>	tmpAlternatives;
	vector<int>		tmpAltProbabilities;

    //If there is no path in the path set, skip
    if(pathUtility.size()==0){
        if(pathSet.size()==0){
            //cout <<"NO ASSIGNED PATH"<<endl;
            return "-101";
        }else{
            cout <<"PATH SET != PATH UTILITY"<<endl;
        }
	}

    //Calculate the denominator of the logit model
	i = 0;
    tmpLogsum = 0;
	for(pathIter2=pathUtility.begin();pathIter2!=pathUtility.end();pathIter2++){
        if(pathCapacity[(*pathIter2).first]>0){
            i++;
            tmpLogsum = tmpLogsum + exp(-theta*(*pathIter2).second);
        }
    }
    if(i==0){
		//cout <<"i=0; ";
		return "-101";
    }
	if(tmpLogsum==0){
		cout <<"LOGSUM=0"<<endl;
		return "-101";
	}

    //calculate the probability of each alternative
	j=-1;
	tmpMaxProb = 0;
	for(pathIter2=pathUtility.begin();pathIter2!=pathUtility.end();pathIter2++){
        if(pathCapacity[(*pathIter2).first]>0){
            tmpAltProb = int(1000000*(exp(-theta*(*pathIter2).second))/tmpLogsum);
            if(tmpAltProb < 1){
                continue;
            }
            j++;
            if(j>0)	tmpAltProb = tmpAltProb + tmpAltProbabilities[j-1];

            tmpAlternatives.push_back((*pathIter2).first);
            tmpAltProbabilities.push_back(tmpAltProb);
            tmpMaxProb = tmpAltProb;
        }
	}
	if(tmpMaxProb<1){
		cout <<"MAXPROB=0"<<endl;
		return "-101";
	}

    //select an alternative
	tmpRandNum = rand()%tmpMaxProb;
	for(j=0;j<tmpAlternatives.size();j++){
		if(tmpRandNum <= tmpAltProbabilities[j]){
			assignedPath = tmpAlternatives[j];
            return tmpAlternatives[j];
		}
	}

    //If nothing returned, return error!
	cout <<"WHAT?"<<endl;
	return "-101";
}
int		passenger::getNumUnlinkedTrips(){
	return this->trips.size();
}
double	passenger::getAccessTime(){
	return this->walkingTimes.front();
}
double	passenger::getEgressTime(){
	return this->walkingTimes.back();
}
string passenger::getUnlinkedTrip(int _i){
	string tmpUnlinkedTrip, tmpStr;
	char	chr[999];

	tmpUnlinkedTrip = this->trips[_i].substr(1,999) + "\t" + this->boardingStops[_i].substr(1,999) + "\t" + this->alightingStops[_i].substr(1,999);


    sprintf(chr,"%d",int(100*walkingTimes[_i])/100);
    tmpStr = string(chr);
    tmpStr.append(".");
	if(int(100*walkingTimes[_i])%100<10)	tmpStr.append("0");
    sprintf(chr,"%d",int(100*walkingTimes[_i])%100);
    tmpStr.append(string(chr));
    tmpUnlinkedTrip.append("\t" + tmpStr);

    sprintf(chr,"%d",int(100*experiencedArrivalTimes[_i])/100);
    tmpStr = string(chr);
    tmpStr.append(".");
	if(int(100*experiencedArrivalTimes[_i])%100<10)	tmpStr.append("0");
    sprintf(chr,"%d",int(100*experiencedArrivalTimes[_i])%100);
    tmpStr.append(string(chr));
    tmpUnlinkedTrip.append("\t" + tmpStr);

	sprintf(chr,"%d",int(100*experiencedBoardingTimes[_i])/100);
	tmpStr = string(chr);
	tmpStr.append(".");
	if(int(100*experiencedBoardingTimes[_i])%100<10)	tmpStr.append("0");
	sprintf(chr,"%d",int(100*experiencedBoardingTimes[_i])%100);
	tmpStr.append(string(chr));
	tmpUnlinkedTrip.append("\t" + tmpStr);

	sprintf(chr,"%d",int(100*experiencedAlightingTimes[_i])/100);
	tmpStr = string(chr);
	tmpStr.append(".");
	if(int(100*experiencedAlightingTimes[_i])%100<10)	tmpStr.append("0");
	sprintf(chr,"%d",int(100*experiencedAlightingTimes[_i])%100);
	tmpStr.append(string(chr));
	tmpUnlinkedTrip.append("\t" + tmpStr);

	return tmpUnlinkedTrip;
}