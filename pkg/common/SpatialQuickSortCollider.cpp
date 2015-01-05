/*************************************************************************
*  Copyright (C) 2008 by Sergei Dorofeenko				 *
*  sega@users.berlios.de                                                 *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/
#include "SpatialQuickSortCollider.hpp"
#include <core/Scene.hpp>
#include <core/BodyContainer.hpp>
#include <cmath>
#include <algorithm>

YADE_PLUGIN((SpatialQuickSortCollider));

void SpatialQuickSortCollider::action()
{
	if(scene->isPeriodic){ throw runtime_error("SpatialQuickSortCollider doesn't handle periodic boundaries."); }

	// update bounds
	boundDispatcher->scene=scene; boundDispatcher->action();

	const shared_ptr<BodyContainer>& bodies = scene->bodies;

	// This collider traverses all interactions at every step, therefore all interactions
	// that were requested for erase might be erased here and will be recreated if necessary.
	scene->interactions->eraseNonReal();

	size_t nbElements=bodies->size();
static int l(0);
if(l<500) { std::cerr<<"nbElements="<<nbElements<<"\n"; l++;};
	if (nbElements!=rank.size())
	{
		size_t n = rank.size();
		rank.resize(nbElements);
		for (; n<nbElements; ++n)
			rank[n] = shared_ptr<AABBBound>(new AABBBound);
	}

// WTF ?????????????????????????? I Don't resolve it anymore. Will just copy over the new file.

// FIXME - I don't know why but sometimes this FOREACH is not working in Quantum Mechanics. It started to work with `for(int i=0;i<bodies->size();i++)` but I don't put this here now. I suspect it's optimised away
//         Also I was testing here a collider problem in commit "Testing FFT "+1" or "+0" and a collider problem."
	Vector3r min,max;
// <<<<<<< HEAD
	int i=0;
	FOREACH(const shared_ptr<Body>& b, *bodies){
		if(!b->bound) continue;
		min = b->bound->min;
		max = b->bound->max;
<<<<<<< HEAD
=======

bool hasNan=(isnan(min[0])||isnan(min[1])||isnan(min[2])||isnan(max[0])||isnan(max[1])||isnan(max[2]));
if(hasNan){std::cerr<<"NAN found in bounds! Try to recalculate.\n"; boundDispatcher->scene=scene; boundDispatcher->action(); }
if(l<500) { std::cerr<<"loop bodies min="<<min<<"  max="<<max<<"\n"; l++;};

>>>>>>> More debug info in SpatialQuickSortCollider
		rank[i]->id = b->getId();
		rank[i]->min = min;
		rank[i]->max = max;
		i++;
// =======
// //	int i=0;
// //	FOREACH(const shared_ptr<Body>& b, *bodies){ // FIXME - I don't know why but sometimes it's not working. I suspect it's optimised away
// 	for(int i=0;i<bodies->size();i++)
// 	{
// 		const shared_ptr<Body>& b( (*(bodies))[i] );
// 		if(!(b->bound)) continue;
// 		min = b->bound->min;
// 		max = b->bound->max;
// 
// bool hasNan=(isnan(min[0])||isnan(min[1])||isnan(min[2])||isnan(max[0])||isnan(max[1])||isnan(max[2]));
// if(hasNan){std::cerr<<"NAN found in bounds! Try to recalculate.\n"; boundDispatcher->scene=scene; boundDispatcher->action(); }
// if(l<100) { std::cerr<<"loop bodies min="<<min<<"  max="<<max<<"\n"; l++;};
// 
// 		rank[i]->id = b->getId();
// 		rank[i]->min = min;
// 		rank[i]->max = max;
// //		i++;
// >>>>>>> Sometimes interactions->insert(interaction);
	}
	
	const shared_ptr<InteractionContainer>& interactions=scene->interactions;
	scene->interactions->iterColliderLastRun=scene->iter;

	sort(rank.begin(), rank.end(), xBoundComparator()); // sorting along X

	int id,id2; size_t j;
	shared_ptr<Interaction> interaction;
	for(int i=0,e=nbElements-1; i<e; ++i)
	{
<<<<<<< HEAD
//<<<<<<< HEAD
		id  = rank[i]->id;
		min = rank[i]->min;
		max = rank[i]->max;
		j=i;
		while(++j<nbElements)
		{
// =======
// 	    id  = rank[i]->id;
// 	    min = rank[i]->min; 
// 	    max = rank[i]->max; 
// 	    j=i;
// 	    while(++j<nbElements)
// 	    {
// if(l<100) { std::cerr<<"check min[i]="<<min<<"  max[i]="<<max<<"\n"; l++;};
// if(l<100) { std::cerr<<"check min[i]="<<rank[i]->min<<"  max[i]="<<rank[i]->max<<"\n"; l++;};
// if(l<100) { std::cerr<<"check min[j]="<<rank[j]->min<<"  max[j]="<<rank[j]->max<<"\n"; l++;};
// if(l<100) { std::cerr<<"j1: "<<j<<"\n"; l++;};
// >>>>>>> Sometimes interactions->insert(interaction);
			if ( rank[j]->min[0] > max[0]) break;
// if(l<100) { std::cerr<<"j2: "<<j<<"\n"; l++;};
=======
	    id  = rank[i]->id;
	    min = rank[i]->min; 
	    max = rank[i]->max; 
	    j=i;
	    while(++j<nbElements)
	    {
if(l<500) { std::cerr<<"check min[i]="<<min<<"  max[i]="<<max<<"\n"; l++;};
if(l<500) { std::cerr<<"check min[i]="<<rank[i]->min<<"  max[i]="<<rank[i]->max<<"\n"; l++;};
if(l<500) { std::cerr<<"check min[j]="<<rank[j]->min<<"  max[j]="<<rank[j]->max<<"\n"; l++;};
if(l<500) { std::cerr<<"j1: "<<j<<"\n"; l++;};
			if ( rank[j]->min[0] > max[0]) break;
if(l<500) { std::cerr<<"j2: "<<j<<"\n"; l++;};
>>>>>>> More debug info in SpatialQuickSortCollider
			if ( rank[j]->min[1] < max[1]
			&& rank[j]->max[1] > min[1]
			&& rank[j]->min[2] < max[2]
			&& rank[j]->max[2] > min[2])
			{
<<<<<<< HEAD
// if(l<100) { std::cerr<<"j3: "<<j<<"\n"; l++;};
				id2=rank[j]->id;
// if(l<100) { std::cerr<<"id="<<id<<" id2="<<id2<<"\n"; l++;};
=======
if(l<500) { std::cerr<<"j3: "<<j<<"\n"; l++;};
				id2=rank[j]->id;
if(l<500) { std::cerr<<"id="<<id<<" id2="<<id2<<"\n"; l++;};
>>>>>>> More debug info in SpatialQuickSortCollider
				if ( (interaction = interactions->find(Body::id_t(id),Body::id_t(id2))) == 0)
				{
					interaction = shared_ptr<Interaction>(new Interaction(id,id2) );
					interactions->insert(interaction);
<<<<<<< HEAD
// if(l<100) { std::cerr<<"inserted id,id2, search now:"<< (  interactions->find(Body::id_t(id),Body::id_t(id2))  ) <<"\n"; l++;};
=======
if(l<500) { std::cerr<<"inserted "<<id<<" "<<id2<<" search now:"<< (  interactions->find(Body::id_t(id),Body::id_t(id2))  ) <<"\n"; l++;};
>>>>>>> More debug info in SpatialQuickSortCollider
				}
				interaction->iterLastSeen=scene->iter; 
			}
		}
	}
if(l<500) { std::cerr<<"search now 0,1:"<< (  interactions->find(Body::id_t(0),Body::id_t(1))  ) <<"\n"; l++;};
if(l<500) { std::cerr<<"search now 0,2:"<< (  interactions->find(Body::id_t(0),Body::id_t(2))  ) <<"\n"; l++;};
if(l<500) { std::cerr<<"search now 1,2:"<< (  interactions->find(Body::id_t(1),Body::id_t(2))  ) <<"\n"; l++;};
}

