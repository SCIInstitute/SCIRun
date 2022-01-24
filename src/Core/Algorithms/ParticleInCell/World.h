#ifndef _WORLD_H
#define _WORLD_H

#include <vector>
#include <random>
#include <chrono>
#include "Field.h"

class Species;

/*define constants*/
namespace Const
{
	const double EPS_0 = 8.85418782e-12;  	// C/(V*m), vacuum permittivity
	const double QE = 1.602176565e-19;		// C, electron charge
	const double AMU = 1.660538921e-27;		// kg, atomic mass unit
	const double ME = 9.10938215e-31;		// kg, electron mass
	const double K = 1.380648e-23;			// J/K, Boltzmann constant
	const double PI = 3.141592653;			// pi
	const double EvToK = QE/K;				// 1eV in K ~ 11604
}

/*object for sampling random numbers*/
class Rnd {
public:
	//constructor: set initial random seed and distribution limits
	Rnd(): mt_gen{std::random_device()()}, rnd_dist{0,1.0} {}
	double operator() () {return rnd_dist(mt_gen);}

protected:
	std::mt19937 mt_gen;	    //random number generator
	std::uniform_real_distribution<double> rnd_dist;  //uniform distribution
};

extern Rnd rnd;		//tell the compiler that an object of type Rnd called rnd is defined somewhere

/*defines the computational domain*/
class World
{
public:	
	/*constructor, allocates memory*/
	World(int ni, int nj, int nk);

	/*functions to set mesh origin and spacing*/
	void setExtents(const double3 x0, const double3 xm);
	
	double3 getX0() const {return double3(x0);}
	double3 getXm() const {return double3(xm);}
	double3 getXc() const {return double3(xc);}
	double3 getDh() const {return double3(dh);}

	/*functions for accessing time information*/
	int getTs() const {return ts;}
	double getTime() const {return time;}
	double getWallTime();  /*returns wall time in seconds*/
	double getDt() const {return dt;}
	bool isLastTimeStep() const {return ts==num_ts-1;}

	bool inBounds(double3 pos) {
		for (int i=0;i<3;i++)
			if (pos[i]<x0[i] || pos[i]>=xm[i]) return false;
		return true;
	}

	/*sets time step and number of time steps*/
	void setTime(double dt, int num_ts) {this->dt=dt;this->num_ts=num_ts;}
	
	/*advances to the next time step, returns true as long as more time steps remain*/
	bool advanceTime() {time+=dt;ts++;return ts<=num_ts;}

	/*converts physical position to logical coordinate*/
    double3 XtoL(double3 x) const {
	  	double3 lc;
			lc[0] = (x[0]-x0(0))/dh(0);
			lc[1] = (x[1]-x0(1))/dh(1);
			lc[2] = (x[2]-x0(2))/dh(2);
			return lc;
	  }

	/*computes charge density from rho = sum(charge*den)*/
	void computeChargeDensity(std::vector<Species> &species);

	/*returns the system potential energy*/
	double getPE();

	//mesh geometry
	const int ni,nj,nk;	//number of nodes
	const int3 nn;	//another way to access node counts

	Field phi;			//potential
	Field rho;			//charge density
	Field node_vol;	//node volumes
	Field3 ef;			//electric field components

protected:
	double3 x0;	//mesh origin
	double3 dh;	//cell spacing

	double3 xm;		//origin-diagonally opposite corner (max bound)
	double3 xc;		//domain centroid
	
	double dt = 0;		//time step length
	double time = 0;	//physical time
	int ts = -1;		//current time step
	int num_ts = 0;		//number of time steps

	std::chrono::time_point<std::chrono::high_resolution_clock> time_start;	//time at simulation start

	void computeNodeVolumes();
};

#endif
