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
