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

#include "ft_stop.h"
#include "ft_route.h"
#include "ft_trip.h"
#include "ft_TAZ.h"
#include "ft_passenger.h"
#include "ft_TBSP.h"
#include "ft_simulation.h"
#include "ft_assignment.h"

int	main(){
	srand((unsigned)time(0));
	cout <<"****************************** READING INPUTS *****************************"<<endl;
	readParameters();
	readRouteChoiceModel();
	cout <<readStops()<<"\t Stops"<<endl;
	cout <<readTransfers()<<"\t Transfers"<<endl;
	cout <<readRoutes()<<"\t Routes"<<endl;
	cout <<readTrips()<<"\t Trips"<<endl;
	cout <<readStopTimes()<<"\t Stop Times"<<endl;
	cout <<defineTransferStops()<<"\t Transfer Stops"<<endl;
	cout <<readTAZs()<<"\t TAZs"<<endl;
	cout <<readAccessLinks()<<"\t Access Links"<<endl;
	cout <<readPassengers()<<"\t Passengers"<<endl;
	passengerAssignment();
    return 0;
}