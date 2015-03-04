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
using namespace std;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class	route{
protected:
	string						routeId;
	string						routeShortName;
	string						routeLongName;
	int							routeType;
	vector<string>				routeTrips;
public:
	route(){}
	~route(){}
	int							initializeRoute(string _tmpIn);
	void						attachTrip(string _tripId);
	int							getNumTrips();
	string						getTripId(int _i);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
map<string,route*>				routeSet;

int	readRoutes();
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int		readRoutes(){
	string			tmpIn, tmpRouteId, buf;
	vector<string>	tokens;

	ifstream inFile;
	inFile.open("ft_input_routes.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_routes.dat";
		exit(1);
	}

	getline(inFile,tmpIn);
	while (!inFile.eof()){
		buf.clear();
		tokens.clear();
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		stringstream ss(tmpIn);
		if (ss >> buf){
			tokens.push_back(buf);
		}
		tmpRouteId = "r";
		tmpRouteId.append(tokens[0]);
		routeSet[tmpRouteId] = new route;
		routeSet[tmpRouteId]->initializeRoute(tmpIn);
	}
	inFile.close();
	return routeSet.size();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int		route::initializeRoute(string _tmpIn){
	stringstream	ss(_tmpIn);
	string	buf;
	vector<string>	tokens;
	while (ss >> buf){
		tokens.push_back(buf);
	}
	routeId = tokens[0];
	routeShortName = tokens[1];
	routeLongName = tokens[2];
	routeType = atoi(tokens[3].c_str());
	return 0;
}
void	route::attachTrip(string _tripId){
	routeTrips.push_back(_tripId);
}
int		route::getNumTrips(){
	return this->routeTrips.size();
}
string	route::getTripId(int _i){
	return this->routeTrips[_i];
}
