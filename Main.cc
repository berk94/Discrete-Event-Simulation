//============================================================================
// Name        : CMPE_250_Project_4.cpp
// Author      : Kemal Berk Kocabagli
// Version     :
// Copyright   : BOGAZICI UNIVERSITY CMPE DEPARTMENT
// Description : Discrete Event Simulation, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <cstdlib>
#include <cmath>
#include <iomanip>

using namespace std;

int no_of_units; //total number of units
int no_of_jobs; //total number of jobs
double factory_finish_t_2; //the time when the last job exits from the factory (random)
double factory_finish_t_1; //the time when the last job exits from the factory (unit w/ shortest queue first)

void Simulate (std::string input_filename, std::string output_filename, int n);
int main(int argc, char *argv[]){
  
  Simulate (argv[1],argv[2], 1); // simulates the event using random unit assignment
  Simulate (argv[1],argv[3], 0); // simulates the event using "shortest queue first" unit assignment
  return 0;

}

struct unit { //represents a unit at the factory

int vertex; // unit id (unit 0, unit 1 etc.)
double proc_time; // the time this unit requires to process a single job
double busy_time; // total busy time of this unit 
double free; // the minimum time when this unit is available (idle)
int q_length; // queue length of this unit at a specific instant
vector <unit*>neighbors; // neighbors of this unit
int max=0; // maximum queue length of this unit
//constructor
	unit (int v, double p_c) {
		vertex = v;
		free=0.0;
		proc_time = p_c;
		busy_time=0.0;
		q_length=0;
	}
};

struct job { //represents a job that arrives at the factory

	int id; //job id (job 0, job 1 etc.)
	int which_Case; // will the simulation this job undergoes be based on random selection or "shortest queue first"  
	unit* u; //unit of this job at a specific instant
	unit* from; //previous unit of this job
	double arrival_time; //arrival time of this job to the factory
	double time; //the exiting time of this job from a unit or from the factory at last 
	bool started =false; //has this job entered the factory (from unit 0)
	bool done =false; //is this job done

	//constructor
	job (double t, int i_d, int wc) {
		id=i_d;
		which_Case = wc;
		arrival_time = t;
		time= arrival_time;
		u=NULL;
		from=NULL;
	}
	void process(){
		//cout << "processing job " << this->id <<" at " << u->vertex << " with free time: "<< u->free << endl;
		//cout << "queue length now is " << u->q_length << " and max length is " << u->max << endl;
		//if (from!=NULL) cout << "queue length of predecessor: " << from->q_length << endl;

		if (time >= u->free) { //unit available when the job arrives - the job will not wait
			//cout << "didnt wait!" << endl;
			u->q_length--;
			u->busy_time += u->proc_time;
			u->free = time +u->proc_time;
			time += u->proc_time;
		}
		else{ //unit not available when the job arrives - the job will wait
			//cout << "waited!" << endl;
			u->busy_time += u->proc_time;
			time = u->free + u->proc_time;
			u->free += u->proc_time;
		}
		if (u->neighbors.empty()) { //the job is at a terminal unit - after it is processed here, it's done
			done = true;
			if (which_Case == 0)  {
			if(time>factory_finish_t_1)factory_finish_t_1= time;
			}
			else{
			if(time>factory_finish_t_2)factory_finish_t_2= time;
			}
		}
		if(u->q_length>u->max)u->max=u->q_length; //update the maximum queue length

		//cout << "queue length now is " << u->q_length << " and max length is " << u->max << endl;

	}
};

struct greater1{ //comparison defined to rank jobs in the event list heap
	bool operator()(const job* a,const job* b) const{
		return a->time>b->time;
	}
};

/*simulates the discrete event based on two different unit assignment methods - n=1 -> random
										n=0 -> shortest queue first */
void Simulate (std::string input_filename, std::string output_filename, int n) {
vector <job*>jobs; //the  list that holds all the jobs in the simulation
vector <unit*>units; //the list that holds all the units in the simulation
priority_queue<job*, vector<job*>, greater1 > event_list; //the "to do" list of the simulation - the event with higher priority is executed first

if (n==1) std::srand(std::time(0)); //random generator

ifstream myFile(input_filename); //input file
ofstream outPut;
outPut.open(output_filename.c_str()); //creates the output file

int lineNo =1;
int id=0;
	//read the input file
	while (myFile.good()) {
		string line;
		while (getline (myFile, line)) {
			stringstream ss;
			ss<<line;
			if (lineNo ==1) {
				// read the number of units and initialize the units list
				ss >> no_of_units;
				units.reserve(no_of_units);
				for (int i=0; i<no_of_units; i++){
					unit* u= new unit (i, 0);
					units.push_back(u);
				}
				lineNo ++;
			}
			else if (lineNo<=1+no_of_units){
				// read the processing time and neighbors of each unit
				int s;
				int d;
				double p_c;
				ss >> s;
				ss >> p_c;
				units[s]->proc_time=p_c;
				while (ss>>d) {
					units [s]->neighbors.push_back (units[d]);
				}
				lineNo++;
			}
			else if (lineNo==2+no_of_units)
			{	// read number of jobs
				ss >> no_of_jobs;
				jobs.reserve(no_of_jobs);
				lineNo ++;
			}
			else {
				// read the job arrival times and fill the jobs list
				double t;
				ss >> t;
				job* j= new job(t, id, n);
				jobs.push_back(j);
				event_list.push(j);
				lineNo ++;
				id++;
			}
		}
	}

//execute all the events until none is left
while(!event_list.empty()){
		job* j= event_list.top();
		event_list.pop();
		if (j->done){
			//cout <<"job "<<j->id << " done !! Final time: " << j->time << endl;
			if(j->u->q_length>0) j->u->q_length--;
		}
		else {
		//cout << "event popped " << j->id << endl;
		if(!j->started){ //first unit should be unit 0 - the job enters the factory
			j->u= units[0];
			j->started =true;
			j->u->q_length++;
			j->process();
			//cout <<"job "<<j->id << " pushed into queue with time: "<< j->time<< endl;
			//cout <<"unit became" << j->u->vertex << endl;
			event_list.push(j);
		}
		else if (!j->done){
			if (n==0){ //next unit is the one with the shortest queue length
			unit* next=j->u->neighbors[0];
			for (int i=1; i<j->u->neighbors.size(); i++){
				if (j->u->neighbors[i]->q_length < next->q_length){
					next =j->u->neighbors[i];
				}
			}
			j->from= j->u;
			j->u = next;
			}
			else { //next unit is assigned randomly
			int rand = std ::floor(((double)std::rand())/RAND_MAX * j->u->neighbors.size()); 
			unit* next = j->u->neighbors[rand];
			j->from= j->u;
			j->u= next;
			}

			j->u->q_length++;
			j->process();
			if(j->from->q_length>0) j->from->q_length--;

			 //cout << "queue length of predecessor: " << j->from->q_length << endl;
			event_list.push(j);
		}
		}
		//cout << "----------------------------------------------"<< endl;
	}
double factory_finish_t= factory_finish_t_1;
if (n==1)factory_finish_t= factory_finish_t_2;

//write the results to the output file
outPut << fixed << setprecision(4) << factory_finish_t << endl;

	for(int i=0; i<no_of_units; i++){
			outPut <<i<<  " " << fixed << setprecision(2)<< (double)units[i]->busy_time/factory_finish_t << " " << units[i]->max << endl;
		}
	for (int i=0; i<no_of_jobs; i++){
			double turnaround= jobs[i]->time-jobs[i]->arrival_time;
			outPut << i << " " << fixed << setprecision(4) <<turnaround << endl;
		}


}
