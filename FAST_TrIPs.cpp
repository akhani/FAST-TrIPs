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

#include "ft_stop.h"
#include "ft_route.h"
#include "ft_trip.h"
#include "ft_TAZ.h"
#include "ft_passenger.h"
#include "ft_TBSP.h"
#include "ft_TBHP.h"
#include "ft_simulation.h"
#include "ft_assignment.h"

int	main(){
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
