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

using namespace std;
//////////////////////////////////////////////////////////////////////////////////////////////////////////

int		iterationFlag,	    	//{1, 2, ...}
		pathModelFlag,			//{0, 1}
		printPassengersFlag,	//{0, 1}
		simulationFlag,			//{0, 1}
		pathTimeBuffer,			//[15, 60]
		transitTrajectoryFlag,	//{0, 1}
		skimFlag,				//{0, 1}
		skimStartTime,			//[0, 1440]
		skimEndTime,				//[0, 1440]
                numThreads              //{1,20};
		;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void	readParameters(){
    cout <<"reading parameters"<<endl;
        string			tmpIn, buf;
	vector<string>	tokens;
	ifstream inFile;
	inFile.open("ft_input_parameters.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_parameters.dat";
		exit(1);
	}

	tokens.clear();
	while (!inFile.eof()){
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		buf.clear();
		stringstream ss(tmpIn);
		if (ss >> buf){
			tokens.push_back(buf);
		}
	}

	iterationFlag = atoi(tokens[0].c_str());
	if(iterationFlag<1){
		cout <<"ERROR - # OF ITERATION SHOULD BE A POSITIVE VALUE"<<endl;
		exit(1);
	}

	pathModelFlag = atoi(tokens[1].c_str());
	if (pathModelFlag!=0 && pathModelFlag!=1 && pathModelFlag!=2){
		cout <<"ERROR - PATH MODEL PARAMETER SHOULD BE EITHER 0, 1 or 2"<<endl;
		exit(1);
	}

	simulationFlag = atoi(tokens[2].c_str());
	if(simulationFlag!=0 && simulationFlag!=1){
		cout <<"ERROR -	SWITCH FOR SIMULATION SHOULD BE EITHER 0 OR 1"<<endl;
		exit(1);
	}

	printPassengersFlag = atoi(tokens[3].c_str());
	if(printPassengersFlag!=0 && printPassengersFlag!=1){
		cout <<"ERROR -	SWITCH FOR PRINTING PATHS SHOULD BE EITHER 0 OR 1"<<endl;
		exit(1);
	}
	if(pathModelFlag==0 && simulationFlag==0 && printPassengersFlag==1){
		cout <<"ERROR -	PATH ASSIGNMENT and SIMULATION IS REQUIRED FOR DETAILED PASSENGER OUTPUTS"<<endl;
		exit(1);
	}

	pathTimeBuffer = atoi(tokens[4].c_str());
	if(pathTimeBuffer<10 || pathTimeBuffer>60){
		cout <<"ERROR -	THE TIME BUFFER SHOULD BE BETWEEN 10 AND 60"<<endl;
		exit(1);
	}

	transitTrajectoryFlag = atoi(tokens[5].c_str());
	if(transitTrajectoryFlag!=0 && transitTrajectoryFlag!=1){
		cout <<"ERROR -	SWITCH FOR USING TRANSIT VEHICLE TRAJECTORIES SHOULD BE EITHER 0 OR 1"<<endl;
		exit(1);
	}

	skimFlag = atoi(tokens[6].c_str());
	if(skimFlag!=0 && skimFlag!=1){
		cout <<"ERROR -	SWITCH FOR SKIM GENERATION SHOULD BE EITHER 0 OR 1"<<endl;
		exit(1);
	}
	if(pathModelFlag==9 && skimFlag!=1){
		cout <<"ERROR -	SWITCH FOR SKIM GENERATION SHOULD BE ON WHEN PATH MODEL PARAMETER IS 9"<<endl;
		exit(1);
	}
        
	/*skimStartTime = atoi(tokens[7].c_str());
	if(skimStartTime<0 || skimStartTime>1410){
		cout <<"ERROR -	BEGINNING OF SKIM TIME PERIOD"<<endl;
		exit(1);
	}

	skimEndTime = atoi(tokens[8].c_str());
	if(skimEndTime<30 || skimEndTime>1440){
		cout <<"ERROR -	END OF SKIM TIME PERIOD"<<endl;
		exit(1);
	}
	if(skimEndTime < skimStartTime+30){
		cout <<"ERROR -	SKIM TIME PERIOD"<<endl;
		exit(1);
	}*/
        
        if(tokens.size()>9){
            numThreads = atoi(tokens[9].c_str());
            if(numThreads<1 || numThreads>20){
	        cout <<"ERROR - # OF THREADS SHOULD BE GREATER THAN 0, LESS THAN 20 and LESS THAN THE NUMBER OF CORES"<<endl;
                cout <<"DEFAULT VALUE OF 1 IS BEING USED FOR THE NUMBER OF THREADS!"<<endl;
                numThreads = 1;
	    }
        }else{
            numThreads = 1;
            cout <<"It seems that multithreading is not available for stochastic assignment!\n DEFAULT VALUE OF 1 IS BEING USED FOR THE NUMBER OF THREADS!"<<endl;
        }
        if(pathModelFlag!=1){
            numThreads = 1;
        }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void	readRouteChoiceModel(){
    cout <<"reading route choice"<<endl;
	string			tmpIn, buf;
	vector<string>	tokens;
	ifstream inFile;
	inFile.open("ft_input_routeChoice.dat");
	if (!inFile) {
		cerr << "Unable to open file ft_input_routeChoice.dat";
		exit(1);
	}

	getline(inFile,tmpIn);
	tokens.clear();
	while (!inFile.eof()){
		getline(inFile,tmpIn);
		if(tmpIn=="")	continue;
		buf.clear();
		stringstream ss(tmpIn);
		if (ss >> buf){
			tokens.push_back(buf);
		}
	}

	inVehTimeEqv = atof(tokens[0].c_str());
	waitingEqv = atof(tokens[1].c_str());
	originWalkEqv = atof(tokens[2].c_str());
	destinationWalkEqv = atof(tokens[3].c_str());
	transferWalkEqv = atof(tokens[4].c_str());
	transferPenalty = atof(tokens[5].c_str());
	scheduleDelayEqv = atof(tokens[6].c_str());
	fare = atof(tokens[7].c_str());
	VOT = atof(tokens[8].c_str());
	theta = atof(tokens[9].c_str());
	capacityConstraint = atoi(tokens[10].c_str());

	railInVehTimeEqv = atoi(tokens[11].c_str());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void		passengerAssignment(){
	string				tmpIn, buf;
	vector<string>		tokens;
	int					numAssignedPassengers, numArrivedPassengers, numMissedPassengers;
	double				capacityGap, tmpAssignmentTime, tmpSimulationTime;

	ofstream		outFile;
	outFile.open("ft_output_runStatistics.dat");
	outFile <<"******************************* SUMMARY ****************************************"<<endl;

    for(int iter=1;iter<=iterationFlag;iter++){
        cout <<"***************************** ITERATION "<<iter<<" **************************************"<<endl;
        outFile <<"***************************** ITERATION "<<iter<<" **************************************"<<endl;

        numAssignedPassengers = 0;
        numArrivedPassengers = 0;
        numMissedPassengers = 0;

        if(pathModelFlag == 0){
            numAssignedPassengers = readExistingPaths();
            cout <<numAssignedPassengers<<"\t Paths"<<endl;
        }else if(pathModelFlag==1){
            numAssignedPassengers = disaggregateDeterministicAssignment(iter, pathTimeBuffer, numThreads);
        }else if(pathModelFlag==2){
            //numAssignedPassengers = disaggregateStochasticAssignment(iter, pathTimeBuffer, numThreads);
            numAssignedPassengers = pathBasedStochasticAssignment(iter, pathTimeBuffer, printPassengersFlag, numThreads);
        }

        if(simulationFlag==1){
            cout <<"****************************** SIMULATING *****************************"<<endl;
            numArrivedPassengers = simulation(pathModelFlag);
            numMissedPassengers = numAssignedPassengers - numArrivedPassengers;
        }

        if(printPassengersFlag==1){
            printPassengerPaths();
            printPassengerTimes();
            printPaths();
            printUnlinkedTrips();
        }

        capacityGap = 100.0*numMissedPassengers/(numArrivedPassengers+numMissedPassengers);

        outFile <<"\t\t\tTOTAL ASSIGNED PASSENGERS:\t"<<numArrivedPassengers+numMissedPassengers<<endl;
        outFile <<"\t\t\tARRIVED PASSENGERS:\t\t\t"<<numArrivedPassengers<<endl;
        outFile <<"\t\t\tMISSED PASSENGERS:\t\t\t"<<numMissedPassengers<<endl;
        outFile <<"\t\t\tCAPACITY GAP:\t\t\t\t"<<capacityGap<<endl;

        if(capacityGap<0.001){
			break;
		}
    }

    cout <<"**************************** WRITING OUTPUTS ****************************"<<endl;
    printLoadProfile();

	outFile <<"************************************* END **************************************"<<endl;
	outFile.close();
	cout <<"FAST-TrIPs run completed successfully"<<endl;
}
