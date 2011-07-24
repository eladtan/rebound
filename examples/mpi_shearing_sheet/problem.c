#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "main.h"
#include "particle.h"
#include "boundaries.h"
#include "output.h"
#include "communication_mpi.h"
#include "tree.h"

extern double OMEGA;
extern double coefficient_of_restitution;
extern double minimum_collision_velocity;

extern double (*coefficient_of_restitution_for_velocity)(double); 
double coefficient_of_restitution_bridges(double v); 

extern double opening_angle2;

void problem_init(int argc, char* argv[]){
	// Setup constants
	opening_angle2	= 1;
	OMEGA 		= 0.00013143527;		// 1/s
	G 		= 6.67428e-11;			// N / (1e-5 kg)^2 m^2
	softening 	= 1.;				// m
	dt 		= 1e-2*2.*M_PI/OMEGA;		// s
	root_nx = 1; root_ny = 1; root_nz = 1;
	nghostx = 1; nghosty = 1; nghostz = 0; 		// Use three ghost rings
	double surfacedensity 	= 400; 			// kg/m^2
	double particle_density	= 400;			// g/cm^3
	double particle_radius 	= 2;			// m
	double particle_mass 	= particle_density*4./3.*M_PI*pow(particle_radius,3); 	// kg
	if (argc>1){					// Try to read boxsize from command line
		boxsize = atof(argv[1]);
	}else{
		boxsize = 100;
	}
	// Use Bridges et al coefficient of restitution.
	coefficient_of_restitution_for_velocity = coefficient_of_restitution_bridges;
	minimum_collision_velocity = particle_radius*OMEGA*0.01;  // small fraction of the shear
	// Setup particle structures
	init_box();
	int _N = round(surfacedensity*boxsize_x*boxsize_y/particle_mass);
	// Initial conditions
#ifdef MPI
	// Only initialise particles on master. This should also be parallised, but obviously depends on the problem.
	if (mpi_id==0){
#endif
		for (int i =0;i<_N;i++){
			struct particle pt;
			pt.x 		= ((double)rand()/(double)RAND_MAX-0.5)*boxsize_x;
			pt.y 		= ((double)rand()/(double)RAND_MAX-0.5)*boxsize_y;
			pt.z 		= 0.1*((double)rand()/(double)RAND_MAX-0.5);
			pt.vx 		= 0;
			pt.vy 		= -1.5*pt.x*OMEGA;
			pt.vz 		= 0;
			pt.ax 		= 0;
			pt.ay 		= 0;
			pt.az 		= 0;
			pt.m 		= particle_mass*1.e0;
#ifndef COLLISIONS_NONE
			pt.r 		= particle_radius;
#endif
			particles_add(pt);
		}
#ifdef MPI
	printf("%d particles initialized on root node.\n",_N);
	}
#endif
}

double coefficient_of_restitution_bridges(double v){
	// v in [m/s]
	return 0.34*pow(fabs(v)*100.,-0.234);
}

void problem_inloop(){

}


void print_tree(struct cell* node, FILE* fp){
	fprintf(fp,"%e\t%e\t%e\t%e\t%e\t%e\t%e\t%e\n",node->x,node->y,node->z,node->mx,node->my,node->mz,node->m,node->w);
	for (int o=0;o<8;o++){
		if (node->oct[o]!=NULL){
			print_tree(node->oct[o],fp);
		}
	}
}


void problem_output(){
//	if (output_check(1e-2*2.*M_PI/OMEGA)){
		output_timing();
	//	output_ascii("ascii.txt");
		/*
		integrate_particles();
		calculate_forces();
		FILE* of = fopen("tree.txt","w"); 
		print_tree(tree_root[0],of);
		fclose(of);
//	if (t>0)	exit(-1);
		exit(1);
//	}
*/
}

void problem_finish(){
}
