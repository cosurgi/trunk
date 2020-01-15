/*************************************************************************
*  Copyright (C) 2006 by Bruno Chareyre                                  *
*  bruno.chareyre@grenoble-inp.fr                                            *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

#include"GlobalStiffnessTimeStepper.hpp"
#include<pkg/dem/FrictPhys.hpp>
#include<pkg/dem/ScGeom.hpp>
#include<pkg/dem/DemXDofGeom.hpp>
#include<core/Interaction.hpp>
#include<core/Scene.hpp>
#include<core/Clump.hpp>
#include<pkg/common/Sphere.hpp>
#include<pkg/dem/Shop.hpp>
#include<pkg/dem/ViscoelasticPM.hpp>
#ifdef YADE_MPI 
	#include <mpi.h> 
	#include <core/Subdomain.hpp>
#endif


namespace yade { // Cannot have #include directive inside.

CREATE_LOGGER(GlobalStiffnessTimeStepper);
YADE_PLUGIN((GlobalStiffnessTimeStepper));

GlobalStiffnessTimeStepper::~GlobalStiffnessTimeStepper() {}

void GlobalStiffnessTimeStepper::findTimeStepFromBody(const shared_ptr<Body>& body, Scene * /*ncb*/)
{
	State* sdec=body->state.get();
	Vector3r&  stiffness= stiffnesses[body->getId()];
	Vector3r& Rstiffness=Rstiffnesses[body->getId()];
	if(body->isClump()) {// if clump, we sum stifnesses of all members
		const shared_ptr<Clump>& clump=YADE_PTR_CAST<Clump>(body->shape);
		FOREACH(Clump::MemberMap::value_type& B, clump->members){
			const shared_ptr<Body>& b = Body::byId(B.first,scene);
			stiffness+=stiffnesses[b->getId()];
			Rstiffness+=Rstiffnesses[b->getId()];
			if (viscEl == true){
				viscosities[body->getId()]+=viscosities[b->getId()];
				Rviscosities[body->getId()]+=Rviscosities[b->getId()];
			}
		}
	}
	Real dt; 
	if(!sdec || stiffness==Vector3r::Zero()){
		// No interaction on this body, return. If using density scaline fallback to PWaveTimestep-like equation for dt.
		if (densityScaling) {
			if (body->material and body->shape) {
				shared_ptr<ElastMat> ebp=YADE_PTR_DYN_CAST<ElastMat>(body->material);
				shared_ptr<Sphere> s=YADE_PTR_DYN_CAST<Sphere>(body->shape);
				if(!ebp || !s) dt=defaultDt;
				Real density=body->state->mass/((4/3.)*Mathr::PI*pow(s->radius,3));
				dt=s->radius/sqrt(ebp->young/density);
// 				dt=defaultDt;
			} else {
				dt=defaultDt;
			}
			if (sdec->densityScaling<=0)  sdec->densityScaling = timestepSafetyCoefficient*pow(dt/targetDt,2.0);
			else sdec->densityScaling = min(1.01*sdec->densityScaling, timestepSafetyCoefficient*pow(dt/targetDt,2.0));}
		return; // not possible to compute!
	} else {
		//Normal case: determine the elastic minimum eigenperiod (and if required determine also the viscous one separately and take the minimum of the two)
		Real dtx, dty, dtz;
		dt = max( max (stiffness.x(), stiffness.y()), stiffness.z() );
		if (dt!=0) {dt = sdec->mass/dt;  computedSomething = true;}//dt = squared eigenperiod of translational motion 
		else dt = Mathr::MAX_REAL;
		if (Rstiffness.x()!=0) {dtx = sdec->inertia.x()/Rstiffness.x();  computedSomething = true;}//dtx = squared eigenperiod of rotational motion around x
		else dtx = Mathr::MAX_REAL;
		if (Rstiffness.y()!=0) {dty = sdec->inertia.y()/Rstiffness.y();  computedSomething = true;}
		else dty = Mathr::MAX_REAL;
		if (Rstiffness.z()!=0) {dtz = sdec->inertia.z()/Rstiffness.z();  computedSomething = true;}
		else dtz = Mathr::MAX_REAL;
		
		Real Rdt =  std::min( std::min (dtx, dty), dtz );//Rdt = smallest squared eigenperiod for elastic rotational motions
		dt = 1.41044*timestepSafetyCoefficient*std::sqrt(std::min(dt,Rdt));//1.41044 = sqrt(2)
	}
	
	//Viscous 
	if (viscEl == true){
		Vector3r&  viscosity = viscosities[body->getId()];
		Vector3r& Rviscosity = Rviscosities[body->getId()];
		Real dtx_visc, dty_visc, dtz_visc;
		Real dt_visc = max(max(viscosity.x(), viscosity.y()), viscosity.z() );
		if (dt_visc!=0) {
			dt_visc = sdec->mass/dt_visc;  computedSomething = true;}//dt = eigenperiod of the viscous translational motion 
		else {dt_visc = Mathr::MAX_REAL;}

		if (Rviscosity.x()!=0) {
			dtx_visc = sdec->inertia.x()/Rviscosity.x();  computedSomething = true;}//dtx = eigenperiod of viscous rotational motion around x
		else dtx_visc = Mathr::MAX_REAL;	
		if (Rviscosity.y()!=0) {
			dty_visc = sdec->inertia.y()/Rviscosity.y();  computedSomething = true;}
		else dty_visc = Mathr::MAX_REAL;
		if (Rviscosity.z()!=0) {
			dtz_visc = sdec->inertia.z()/Rviscosity.z();  computedSomething = true;}
		else dtz_visc = Mathr::MAX_REAL;
	
		Real Rdt_visc =  std::min( std::min (dtx_visc, dty_visc), dtz_visc );//Rdt = smallest squared eigenperiod for viscous rotational motions
		dt_visc = 2*timestepSafetyCoefficient*std::min(dt_visc,Rdt_visc);

		//Take the minimum between the elastic and viscous minimum eigenperiod. 
		dt = std::min(dt,dt_visc);
	}

	//if there is a target dt, then we apply density scaling on the body, the inertia used in Newton will be mass/scaling, the weight is unmodified
	if (densityScaling) {
		if (sdec->densityScaling>0)  sdec->densityScaling = min(sdec->densityScaling*1.05, pow(dt /targetDt,2.0));
		else sdec->densityScaling = pow(dt /targetDt,2.0);
		newDt=targetDt;
	}
	//else we update dt normaly
	else {newDt = std::min(dt,newDt);}   
}

bool GlobalStiffnessTimeStepper::isActivated()
{
      return  (active && ((!computedOnce) || (scene->iter % timeStepUpdateInterval == 0) || (scene->iter < (long int) 2) ));
	
}

void GlobalStiffnessTimeStepper::computeTimeStep(Scene* ncb)
{
	// for some reason, this line is necessary to have correct functioning (no idea _why_)
	// see scripts/test/compare-identical.py, run with or without active=active.
	active=active;
	if (defaultDt<0) defaultDt= timestepSafetyCoefficient*Shop::PWaveTimeStep(Omega::instance().getScene());
	computeStiffnesses(ncb);

	shared_ptr<BodyContainer>& bodies = ncb->bodies;
	newDt = Mathr::MAX_REAL;
	computedSomething=false;
	for (const auto &b : *bodies) {
		if (!b) {continue; } 
		if (b->isDynamic() && !b->isClumpMember()) findTimeStepFromBody(b, ncb);
	}
	if(densityScaling) (newDt=targetDt);
	if(computedSomething || densityScaling){
		previousDt = min ( min(newDt , maxDt), 1.05*previousDt );// at maximum, dt will be multiplied by 1.05 in one iterration, this is to prevent brutal switches from 0.000... to 1 in some computations
		scene->dt=previousDt;
		computedOnce = true;}
	else if (!computedOnce) scene->dt=defaultDt;
	
#ifdef YADE_MPI
	if (parallelMode){
		if (scene->iter % timeStepUpdateInterval == 0){
			Real recvDt; Real myDt = scene->dt; 
			MPI_Allreduce(&myDt,&recvDt,1, MPI_DOUBLE,MPI_MIN,scene->getComm()); 
			scene->dt = recvDt;  
		}
	}
#endif 
 
// 	LOG_INFO("computed timestep " << newDt <<
// 			(scene->dt==newDt ? string(", applied") :
// 			string(", BUT timestep is ")+boost::lexical_cast<string>(scene->dt))<<".");
}

void GlobalStiffnessTimeStepper::computeStiffnesses(Scene* rb){
	/* check size */
	size_t size=stiffnesses.size();
	if(size<rb->bodies->size()){
		size=rb->bodies->size();
		stiffnesses.resize(size); Rstiffnesses.resize(size);
		if (viscEl == true){
			viscosities.resize(size); Rviscosities.resize(size);
			}
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
// this is to remove warning about manipulating raw memory
#pragma GCC diagnostic ignored "-Wclass-memaccess"
	/* reset stored values */
	memset(& stiffnesses[0],0,sizeof(Vector3r)*size);
	memset(&Rstiffnesses[0],0,sizeof(Vector3r)*size);
	if (viscEl == true){
		memset(& viscosities[0],0,sizeof(Vector3r)*size);
		memset(&Rviscosities[0],0,sizeof(Vector3r)*size);
	}
#pragma GCC diagnostic pop

	FOREACH(const shared_ptr<Interaction>& contact, *rb->interactions){
		if(!contact->isReal()) continue;

		GenericSpheresContact* geom=YADE_CAST<GenericSpheresContact*>(contact->geom.get()); assert(geom);
		NormShearPhys* phys=YADE_CAST<NormShearPhys*>(contact->phys.get()); assert(phys);

		// all we need for getting stiffness
		Vector3r& normal=geom->normal; Real& kn=phys->kn; Real& ks=phys->ks; Real& radius1=geom->refR1; Real& radius2=geom->refR2;
		Real fn = (static_cast<NormShearPhys *> (contact->phys.get()))->normalForce.squaredNorm();
		if (fn==0) continue;//Is it a problem with some laws? I still don't see why.
		
		//Diagonal terms of the translational stiffness matrix
		Vector3r diag_stiffness = Vector3r(std::pow(normal.x(),2),std::pow(normal.y(),2),std::pow(normal.z(),2));
		diag_stiffness *= kn-ks;
		diag_stiffness = diag_stiffness + Vector3r(1,1,1)*ks;

		//diagonal terms of the rotational stiffness matrix
		// Vector3r branch1 = currentContactGeometry->normal*currentContactGeometry->radius1;
		// Vector3r branch2 = currentContactGeometry->normal*currentContactGeometry->radius2;
		Vector3r diag_Rstiffness =
			Vector3r(std::pow(normal.y(),2)+std::pow(normal.z(),2),
				std::pow(normal.x(),2)+std::pow(normal.z(),2),
				std::pow(normal.x(),2)+std::pow(normal.y(),2));
		diag_Rstiffness *= ks;
		
		// If contact moments are present, add the diagonal of (n⊗n*k_twist + (I-n⊗n)*k_roll = (k_twist-k_roll)*n⊗n + I*k_roll ) :
		Vector3r kr = (static_cast<NormShearPhys *> (contact->phys.get()))->getRotStiffness(); //get the vector (k_twist,k_roll,k_roll)
		Vector3r nn (std::pow(normal.x(),2),std::pow(normal.y(),2),std::pow(normal.z(),2));//n⊗n 
		Vector3r diag_Mstiffness= (kr[0]-kr[1])*nn + Vector3r(1,1,1)*kr[1];

		stiffnesses [contact->getId1()]+=diag_stiffness;
		Rstiffnesses[contact->getId1()]+=diag_Rstiffness*pow(radius1,2)+ diag_Mstiffness;
		stiffnesses [contact->getId2()]+=diag_stiffness;
		Rstiffnesses[contact->getId2()]+=diag_Rstiffness*pow(radius2,2)+ diag_Mstiffness;

		//Same for the Viscous part, if required
		if (viscEl == true){
			ViscElPhys* viscPhys = YADE_CAST<ViscElPhys*>(contact->phys.get()); assert(viscPhys);
			Real& cn=viscPhys->cn; Real& cs=viscPhys->cs;
			//Diagonal terms of the translational viscous matrix
			Vector3r diag_viscosity = Vector3r(std::pow(normal.x(),2),std::pow(normal.y(),2),std::pow(normal.z(),2));
			diag_viscosity *= cn-cs;
			diag_viscosity = diag_viscosity + Vector3r(1,1,1)*cs;
			//diagonal terms of the rotational viscous matrix
			Vector3r diag_Rviscosity =
				Vector3r(std::pow(normal.y(),2)+std::pow(normal.z(),2),
					std::pow(normal.x(),2)+std::pow(normal.z(),2),
					std::pow(normal.x(),2)+std::pow(normal.y(),2));
			diag_Rviscosity *= cs;			
			
			// Add the contact stiffness matrix to the two particles one
			viscosities [contact->getId1()]+=diag_viscosity;
			Rviscosities[contact->getId1()]+=diag_Rviscosity*pow(radius1,2);
			viscosities [contact->getId2()]+=diag_viscosity;
			Rviscosities[contact->getId2()]+=diag_Rviscosity*pow(radius2,2);
		}

	}
}

} // namespace yade

