/*definitions for species functions*/
#include <math.h>
#include "Species.h"
#include "Field.h"

/*updates velocities and positions of all particles of this species*/
void Species::advance()
{
	/*get the time step*/
	double dt = world.getDt();

	/*save mesh bounds*/
	double3 x0 = world.getX0();
	double3 xm = world.getXm();

	/*continue while particles remain*/
	for (Particle &part: particles)
	{
		/*get logical coordinate of particle's position*/
		double3 lc = world.XtoL(part.pos);
		
		/*electric field at particle position*/
		double3 ef_part = world.ef.gather(lc);
			
		/*update velocity from F=qE*/
		part.vel += ef_part*(dt*charge/mass);

		/*update position from v=dx/dt*/
		part.pos += part.vel*dt;

		/*did this particle leave the domain? reflect back*/
		for (int i=0;i<3;i++)
		{
			if (part.pos[i]<x0[i]) {part.pos[i]=2*x0[i]-part.pos[i]; part.vel[i]*=-1.0;}
			else if (part.pos[i]>=xm[i]) {part.pos[i]=2*xm[i]-part.pos[i]; part.vel[i]*=-1.0;}
		}
	}
}
	
/*compute number density*/
void Species::computeNumberDensity()
{
	den.clear();
	for (Particle &part:particles)
	{
		double3 lc = world.XtoL(part.pos);
		den.scatter(lc, part.mpw);
	}
		
	//divide by node volume
	den /= world.node_vol;
}

/*adds a new particle, rewinding velocity by half dt*/
void Species::addParticle(double3 pos, double3 vel, double mpw)
{
	//don't do anything (return) if pos outside domain bounds [x0,xd)
	if (!world.inBounds(pos)) return;

	//get particle logical coordinate
	double3 lc = world.XtoL(pos);
	
	//evaluate electric field at particle position
    double3 ef_part = world.ef.gather(lc);

	//rewind velocity back by 0.5*dt*ef
    vel -=  charge/mass*ef_part*(0.5*world.getDt());

    //add to list
    particles.emplace_back(pos,vel,mpw);
}

/*loads randomly distributed particles in a x1-x2 box representing num_den number density*/
void Species::loadParticlesBox(double3 x1, double3 x2, double num_den, int num_mp)
{
	double box_vol = (x2[0]-x1[0])*(x2[1]-x1[1])*(x2[2]-x1[2]);		//box volume
	double num_real = num_den * box_vol;		//number of real particles
	double mpw = num_real/num_mp;			//macroparticle weight

	/*preallocate memory for particles*/
	particles.reserve(num_mp);

	/*load particles on an equally spaced grid*/
	for (int p=0;p<num_mp;p++)
	{
		//sample random position
		double3 pos;
		pos[0] = x1[0] + rnd()*(x2[0]-x1[0]);
		pos[1] = x1[1] + rnd()*(x2[1]-x1[1]);
		pos[2] = x1[2] + rnd()*(x2[2]-x1[2]);

		//set initial velocity
		double3 vel {0,0,0};	//stationary particle

		addParticle(pos,vel,mpw);	//add a new particle to the array
	}
}

/*quiet start load of num_sim[0]*num_sim[1]*num_sim[2] particles in a x1-x2 box
representing num_den number density*/
void Species::loadParticlesBoxQS(double3 x1, double3 x2, double num_den, int3 num_mp)
{
	double box_vol = (x2[0]-x1[0])*(x2[1]-x1[1])*(x2[2]-x1[2]);		//box volume
	int num_mp_tot = (num_mp[0]-1)*(num_mp[1]-1)*(num_mp[2]-1);	//total number of simulation particles
	double num_real = num_den * box_vol;		//number of real particles
	double mpw = num_real/num_mp_tot;			//macroparticle weight

	/*compute particle grid spacing*/
	double di = (x2[0]-x1[0])/(num_mp[0]-1);
	double dj = (x2[1]-x1[1])/(num_mp[1]-1);
	double dk = (x2[2]-x1[2])/(num_mp[2]-1);

	/*preallocate memory for particles*/
	particles.reserve(num_mp_tot);

	/*load particles on a equally spaced grid*/
	for (int i=0;i<num_mp[0];i++)
		for (int j=0;j<num_mp[1];j++)
			for (int k=0;k<num_mp[2];k++)
			{
				double pos[3];
				pos[0] = x1[0] + i*di;
				pos[1] = x1[1] + j*dj;
				pos[2] = x1[2] + k*dk;

				//shift particles on max faces back to the domain
				if (pos[0]==x2[0]) pos[0]-=1e-4*di;
				if (pos[1]==x2[1]) pos[1]-=1e-4*dj;
				if (pos[2]==x2[2]) pos[2]-=1e-4*dk;

				double w = 1;	//relative weight
				if (i==0 || i==num_mp[0]-1) w*=0.5;
				if (j==0 || j==num_mp[1]-1) w*=0.5;
				if (k==0 || k==num_mp[2]-1) w*=0.5;

				/*add rewind*/
				double vel[3] = {0,0,0};	//particle is stationary

				addParticle(pos,vel,mpw*w);	//add a new particle to the array
			}
}

/*returns the number of real particles*/
double Species::getRealCount() {
	double mpw_sum = 0;
	for (Particle &part:particles)
		mpw_sum+=part.mpw;
	return mpw_sum;
}

/* returns the species momentum*/
double3 Species::getMomentum() {
	double3 mom;
	for (Particle &part:particles)
		mom+=part.mpw*part.vel;
	return mass*mom;
}

/* returns the species kinetic energy*/
double Species::getKE() {
	double ke = 0;
	for (Particle &part:particles)
	{
		double v2 = part.vel[0]*part.vel[0] + part.vel[1]*part.vel[1] + part.vel[2]*part.vel[2];
		ke += part.mpw*v2;
	}
	return 0.5*mass*ke;
}

