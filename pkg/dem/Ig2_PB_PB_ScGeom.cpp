/* CWBoon 2016 */
/* Please cite: */
/* CW Boon, GT Houlsby, S Utili (2012).  A new algorithm for contact detection between convex polygonal and polyhedral particles in the discrete element method.  Computers and Geotechnics 44, 73-82. */
/* The numerical library is changed from CPLEX to CLP because subscription to the academic initiative is required to use CPLEX for free */

#ifdef YADE_POTENTIAL_BLOCKS

#include "Ig2_PB_PB_ScGeom.hpp"
#include <pkg/dem/ScGeom.hpp>
//#include <pkg/dem/PotentialBlock.hpp>
//#include<yade/lib-base/yadeWm3Extra.hpp>
#include <pkg/dem/KnKsPBLaw.hpp>

#include <core/Scene.hpp>
#include <lib/base/Math.hpp>
#include <pkg/common/InteractionLoop.hpp>
#include <core/Omega.hpp>

#include <iostream>
#include <fstream>
//#include <iomanip>

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/QR>

#include <ctime>
#include <cstdlib>


namespace yade { // Cannot have #include directive inside.


YADE_PLUGIN((Ig2_PB_PB_ScGeom)
//#ifdef YADE_OPENGL
//		(Gl1_Ig2_PB_PB_ScGeom)
//	#endif
);

CREATE_LOGGER(Ig2_PB_PB_ScGeom);

/* ***************************************************************************************************************************** */
bool Ig2_PB_PB_ScGeom::go(const shared_ptr<Shape>& cm1,const shared_ptr<Shape>& cm2,const State& state1, const State& state2, const Vector3r& shift2, const bool& force,const shared_ptr<Interaction>& c)
{
	PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	PotentialBlock *s2=static_cast<PotentialBlock*>(cm2.get());

	/* Short circuit if both particles are boundary particles */
	if( (s1->isBoundary == true) && (s2->isBoundary == true) ) {return false;}
	if( (s1->isLining   == true) && (s2->isLining   == true) ) {return false;} //2D cases only
	if( (s1->erase      == true) && (s2->isLining   == true) ) {return false;} //2D cases only
	if( (s1->isLining   == true) && (s2->erase      == true) ) {return false;} //2D cases only

	/* Short circuit if both particles are fixed */
	if ( (state1.blockedDOFs==State::DOF_ALL) && (state2.blockedDOFs==State::DOF_ALL) ) { return false;}

	TIMING_DELTAS_START();

	//TODO: The parameters: jointLength, calJointLength, isLining are only used if phys->twoDimension=true. We should nest them inside "if" statements, to reduce unecessary overhead for 3D contacts

	bool hasGeom = false;
	Vector3r contactPt(0,0,0);
	shared_ptr<ScGeom> scm;
	shared_ptr<KnKsPBPhys> phys;

	Real stepBisection = 0.001*std::min(s1->R,s2->R);
	if( stepBisection < pow(10,-6) ){ std::cout<<"R1: "<<s1->R<<", R2: "<<s2->R<<", stepBisection: "<<stepBisection<<", id1: "<<c->getId1()<<", id2: "<<c->getId2()<<endl; }

	bool contact = false;   // Whether contact is established, using: startingPointFeasibilityCLP
	bool converge = false;  // Whether the analytic center calculation converged, using: customSolveAnalyticCentre

	Real fA = 0.0, fB = 0.0;
	Vector3r normalP1(0,0,0), normalP2(0,0,0), avgNormal(0,0,0), ptOnP1(0,0,0), ptOnP2(0,0,0), shearDir(0,0,0);

	if(c->geom){
		hasGeom = true;
		scm=YADE_PTR_CAST<ScGeom>(c->geom);
		if( scm->penetrationDepth > stepBisection ){ stepBisection = 0.5*scm->penetrationDepth; }
		if( stepBisection < pow(10,-6)            ){ std::cout<<"stepBisection: "<<stepBisection<<", penetrationDepth: "<<scm->penetrationDepth<<endl; }
		contactPt = scm->contactPoint;
	}else{
		scm=shared_ptr<ScGeom>(new ScGeom());
		c->geom=scm;
		contactPt = 0.5*(state1.pos+state2.pos+shift2);
	}

	bool hasPhys=false;
	if(c->phys){
		phys=YADE_PTR_CAST<KnKsPBPhys>(c->phys); hasPhys=true; shearDir = phys->shearDir; //normalization should take place in the Contact Law
	}

	if(s1->isLining == true ||  s2->isLining==true){
		Real overlap = 0.0; //overlap represents the penetrationDepth
		if (s1->isLining == true){
			contactPt = state1.pos;
			fA = evaluatePB(cm2, state2, Vector3r(0,0,0) /* shift2*/ , contactPt); //Periodic Boundary conditions are not considered for the RockLiningGlobal code
			if(fA<0.0){
				contact = true;
				avgNormal = contactPt;  //getNormal(cm2,state2,contactPt);  //
				avgNormal.normalize();
				//avgNormal = -avgNormal;
				Vector3r step = avgNormal*stepBisection;
				//int locationStuck = 5;
				getPtOnParticle2(cm2, state2, Vector3r(0,0,0) /* shift2*/ , contactPt, -1.0*step, ptOnP2);
				overlap = (ptOnP2-contactPt).norm();// overlap;
				//avgNormal = contactPt;
				//avgNormal.normalize();
			}else{contact=false;}
		}else{
			contactPt = state2.pos;
			fB = evaluatePB(cm1, state1, Vector3r(0,0,0) /* shift2*/ , contactPt);
			if (fB<0.0){
				contact = true;
				avgNormal = contactPt;  //getNormal(cm1,state1,contactPt); //
				avgNormal.normalize();
				avgNormal = -avgNormal;
				Vector3r step = avgNormal*stepBisection;
				//int locationStuck = 5;
				getPtOnParticle2(cm1, state1, Vector3r(0,0,0) /* shift2*/ , contactPt, 1.0*step, ptOnP1);
				overlap = (ptOnP1-contactPt).norm();// overlap;
				//avgNormal = contactPt;
				//avgNormal.normalize();
				//avgNormal = -avgNormal;
			}else{contact=false;}
		}

		if (contact == true || c->isReal() || force){
			if (contact){
				scm->precompute(state1,state2,scene,c,avgNormal,!(hasGeom),shift2,false /* avoidGranularRatcheting */); //Assign contact point and normal after precompute!!!!
				scm->contactPoint = contactPt; scm->penetrationDepth=overlap; if(std::isnan(avgNormal.norm())){std::cout<<"avgNormal: "<<avgNormal<<endl;}
				scm->normal = avgNormal;
				if(hasPhys){
//					phys->normal = avgNormal;
					/*phys->ptOnP1 = contactPt; */ phys->ptOnP1 = ptOnP1; phys->ptOnP2 = ptOnP2;// phys->initial1 = scm->contactPoint;
	//				phys->gap = scm->penetrationDepth;
					phys->tension = std::max(s1->tension[0],s2->tension[0]);
					phys->cohesion = std::max(s1->cohesion[0],s2->cohesion[0]);
					phys->jointLength = std::max(s1->liningLength,s2->liningLength);
					phys->contactArea = phys->jointLength*unitWidth2D;
				}
			}else{
				//scm->normal = Vector3r(0,0,0);
				scm->precompute(state1,state2,scene,c,avgNormal,!(hasGeom),shift2,false /* avoidGranularRatcheting */); //Assign contact point and normal after precompute!!!!
				scm->contactPoint = contactPt;
				scm->penetrationDepth = -1.0;
				//scm->normal = Vector3r(0,0,0);
			}
			return true;
		}else{
			scm->contactPoint = contactPt;
			scm->penetrationDepth = -1.0;
			scm->normal = Vector3r(0,0,0);
			return false;
		}
	}

	TIMING_DELTAS_CHECKPOINT("Setup");

//	bool convergeFeasibility = true;
//	fA = evaluatePB(cm1, state1, contactPt);
//	fB = evaluatePB(cm2, state2, contactPt);

	//if (fA < -pow(10.0,-6) && fB < -pow(10.0,-6) ){
	//	contact = true;
	//}else{
		contact = startingPointFeasibilityCLP( cm1, state1, cm2, state2, shift2, contactPt/*, convergeFeasibility */);
	//}

	TIMING_DELTAS_CHECKPOINT("End of startingPointFeasibilityCLP");

	//FIXME: Check how necessary the following check of the sign of fA*fB is. Here, we check the updated contactPt after running the feasibility check and before calculating the analytic centre and we end up calculating the fA,fB for two candidate contact points @vsangelidakis
	fA = evaluatePB(cm1, state1, Vector3r(0,0,0), contactPt);
	fB = evaluatePB(cm2, state2, shift2         , contactPt);
	if(fA*fB < 0.0){ std::cout<<"after clp fA: "<<fA<<", fB: "<<fB<<", contact: "<<contact<< /* ", convergeFeasibility: "<<convergeFeasibility */ endl; }

	if(contact/* && convergeFeasibility == true */){
		converge = customSolveAnalyticCentre(cm1, state1, cm2, state2, shift2, contactPt);
		fA = evaluatePB(cm1, state1, Vector3r(0,0,0), contactPt);
		fB = evaluatePB(cm2, state2, shift2         , contactPt);
		if (converge == false){
			contact = false;
			contactPt = 0.5*(state1.pos+state2.pos);
			std::cout<<"analytic centre did not converge"<<endl;
		}else if (fA < 0.0 && fB < 0.0){
			contact = true;
		}else{
			contact = false;
			std::cout<<"One outside, fA: "<<fA<<", fB: "<<fB<<",s1->isBoundary: "<<s1->isBoundary<<", s2->isBoundary: "<<s2->isBoundary<<endl;
			contactPt = 0.5*(state1.pos+state2.pos);
		}
	}

	TIMING_DELTAS_CHECKPOINT("End of customSolveAnalyticCentre");

	////////////////////////////////////////////////* PASS VARIABLES TO ScGeom and Phys Functor *////////////////////////////////////////////////
	//* 1. Get overlap *//
	//* 2. Get information on active planes.  If active planes are not the same as the previous, get new physical parameters.  Otherwise keep the same parameters *//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (contact == true || c->isReal() || force){
		if (contact){

			normalP1 = getNormal(cm1,state1,Vector3r(0,0,0),contactPt,twoDimension); normalP1.normalize();
			normalP2 = getNormal(cm2,state2,shift2,         contactPt,twoDimension); normalP2.normalize();
			//if(s1->isBoundary==true){avgNormal=normalP1;}else if(s2->isBoundary==true){avgNormal=-normalP2;}else{avgNormal = (normalP1 - normalP2);}

			avgNormal = (normalP1 - normalP2); avgNormal.normalize();

			if(s1->fixedNormal){ avgNormal =  s1->boundaryNormal; }
			if(s2->fixedNormal){ avgNormal = -s2->boundaryNormal; }

			Vector3r step = avgNormal*stepBisection;
			//int locationStuck = 2;
			getPtOnParticle2(cm1, state1, Vector3r(0,0,0), contactPt,      step, ptOnP1);
			getPtOnParticle2(cm2, state2, shift2,          contactPt, -1.0*step, ptOnP2);

			Real penetrationDepth = (ptOnP2-ptOnP1).norm();// overlap;
				//std::cout<<"contactpoint: "<<contactPt<<", penetrationDepth: "<<penetrationDepth<<", avgNormal: "<<avgNormal<<endl;

			if(hasPhys){
//				phys->normal = avgNormal;
				phys->ptOnP1 = ptOnP1; phys->ptOnP2 = ptOnP2;// phys->initial1 = scm->contactPoint;
//				phys->gap = scm->penetrationDepth;
				/*bool calJointLength = phys->calJointLength;*/ Real jointLength = phys->jointLength; int smallerID = 1; //bool twoD = phys->twoDimension;
				shearDir = phys->shearDir; shearDir.normalize();

				/* Get contact area */
//				phys->prevJointLength = jointLength; //Real prevContactArea = phys->contactArea;
				if(calContactArea) { //calculate jointLength for 2-D contacts and contactArea for 2-D and 3-D contacts
					phys->contactArea = getAreaPolygon2(cm1, state1, cm2, state2, shift2, contactPt, avgNormal, smallerID, shearDir, jointLength, twoDimension, unitWidth2D);
					/* phys->smallerID = smallerID; */
					if(twoDimension) {
						phys->jointLength = jointLength;
					}
				} else { //don't calculate jointLength or contactArea; assume constant linear stiffness in both normal and shear directions
					phys->jointLength = 1.0;
					if (twoDimension == true) {
						phys->contactArea=phys->jointLength*unitWidth2D;
					} else {
						phys->contactArea = 1.0;
					}
				}

				/* Get physical properties of the contact from the physical properties of the involved (intersecting) particle faces */
				if(phys->useFaceProperties){
					Real phi_b1=0.0, phi_b2=0.0, phi_r1=0.0, phi_r2=0.0, cohesion1=0.0, cohesion2=0.0, tension1=0.0, tension2=0.0;
					bool intactRock1=false, intactRock2=false;
					int jointType1=0, jointType2=0, noActive1=0, noActive2=0;
					/* Real JRC1=0.0, JRC2=0.0, JSC1=0.0, JSC2=0.0, sigmaC1=0.0, sigmaC2=0.0, asperity1=0.0, asperity2=0.0; */
					/* Real lambda01=0.0, lambda02=0.0, heatCapacity1=0.0, heatCapacity2=0.0, hwater1 = 0.0, hwater2 = 0.0; */

					Real f1 = evaluatePhys(cm1, state1, Vector3r(0,0,0), contactPt, phi_b1, phi_r1, /* JRC1, JSC1, */ cohesion1, /* sigmaC1, asperity1, */ tension1, /* lambda01, heatCapacity1, hwater1, */ intactRock1, noActive1, jointType1);
					Real f2 = evaluatePhys(cm2, state2, shift2         , contactPt, phi_b2, phi_r2, /* JRC2, JSC2, */ cohesion2, /* sigmaC2, asperity2, */ tension2, /* lambda02, heatCapacity2, hwater2, */ intactRock2, noActive2, jointType2);

					f1+=0; f2+=0; //FIXME: These two statements are added to mute compiler warnings for unused parameters f1, f2. To be amended soon, by changing the type of evaluatePhys to "void", instead of "Real"

					if(s1->isBoundary == true){
						phys->phi_b = phi_b1;
						phys->phi_r = phi_r1;
						phys->cohesion = cohesion1;
						phys->tension = tension1;
						phys->jointType = jointType1;
						/* phys->JRC = JRC1; phys->JCS = JSC1; phys->sigmaC = sigmaC1; phys->asperity = asperity1; */
						/* phys->lambda0 = lambda01; phys->heatCapacities = heatCapacity1; phys->hwater = hwater1; */
					}else if(s2->isBoundary == true){
						phys->phi_b = phi_b2;
						phys->phi_r = phi_r2;
						phys->cohesion = cohesion2;
						phys->tension = tension2;
						phys->jointType = jointType2;
						/* phys->JRC = JRC2; phys->JCS = JSC2; phys->sigmaC = sigmaC2; phys->asperity = asperity2; */
						/* phys->lambda0 = lambda02; phys->heatCapacities = heatCapacity2; phys->hwater = hwater2; */
					}else{
						phys->phi_b = std::min(phi_b1,phi_b2);
						phys->phi_r = std::min(phi_r1,phi_r2);
						phys->cohesion = std::min(cohesion1,cohesion2);
						phys->tension = std::min(tension1,tension2);
						phys->jointType = std::max(jointType1,jointType2);
						/*phys->JRC = 0.5*(JRC1+JRC2); phys->JCS = 0.5*(JSC1+JSC2); */
						/*phys->sigmaC = 0.5*(sigmaC1 + sigmaC2); phys->asperity = 0.5*(asperity1+asperity2); */
						/*phys->lambda0 = std::max(lambda01,lambda02); phys->hwater = std::max(hwater1,hwater2); */
						/*phys->heatCapacities = std::max(heatCapacity1,heatCapacity2); */
						phys->intactRock = true;
						if(intactRock1==false || intactRock2 == false){ phys->intactRock = false;}
					}
//					if(noActive1 == 1 || noActive2 == 1){phys->rockJointContact = true;}else{phys->rockJointContact = false; }
				}

			}
			scm->precompute(state1,state2,scene,c,avgNormal,!(hasGeom),shift2,false /* avoidGranularRatcheting */); //Assign contact point and normal after precompute!!!!
			scm->contactPoint = contactPt;
			scm->penetrationDepth=penetrationDepth;
			if(std::isnan(avgNormal.norm())){std::cout<<"avgNormal: "<<avgNormal<<endl;}
			scm->normal = avgNormal;

			TIMING_DELTAS_CHECKPOINT("After contact normal, overlap, area");
		}else{
			//scm->normal = Vector3r(0,0,0);
			scm->precompute(state1,state2,scene,c,avgNormal,!(hasGeom),shift2,false /* avoidGranularRatcheting */); //Assign contact point and normal after precompute!!!!
			scm->contactPoint = contactPt;
			scm->penetrationDepth=-1.0;
//			scm->normal = Vector3r(0,0,0);
		}
		return true;
	}else{
		scm->contactPoint = contactPt;
		scm->penetrationDepth = -1.0;
		scm->normal = Vector3r(0,0,0);
		return false;
	}
	return false;
}


/* ***************************************************************************************************************************** */
bool Ig2_PB_PB_ScGeom::goReverse(
		const shared_ptr<Shape>& cm1,
		const shared_ptr<Shape>& cm2,
		const State& state1,
		const State& state2,
		const Vector3r& shift2,
		const bool& force,
		const shared_ptr<Interaction>& c) {
			c->swapOrder();
			return go(cm2,cm1,state2,state1,-shift2,force,c); // This works with c->swapOrder()
//			return go(cm1,cm2,state2,state1,-shift2,force,c); // Alternativelly, this works without c->swapOrder()
}


/* ***************************************************************************************************************************** */
/* The evaluatePhys function detects the active planes, i.e. the planes of the intersecting particles which are involved in the current interaction, if the contact point is located between the actual particle and the inner potential particle (works only in this case). Then, the contact properties of the interaction are set as the minimal or average contact properties of the active planes. This assumes that the user has defined different contact properties for each face of each particle, in the vector parameters: phi_b, phi_r, tension, cohesion, jointType of the shape class. @vsangelidakis */
Real Ig2_PB_PB_ScGeom::evaluatePhys(const shared_ptr<Shape>& cm1, const State& state1, const Vector3r& shift2, const Vector3r newTrial, Real& phi_b, Real& phi_r, /* Real& JRC, Real& JSC, */ Real& cohesion, /* Real& sigmaC, Real& asperity, */ Real& tension, /* Real& lambda0,Real& heatCapacity, Real &hwater, */ bool &intactRock, int &activePlanesNo, int& jointType){

	PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	///////////////////Transforming trial values to local frame of particles//////////////////
	Vector3r tempP1 = newTrial - state1.pos - shift2;
	/* Direction cosines */
	Vector3r localP1 = state1.ori.conjugate()*tempP1;
	Real x = localP1.x();
	Real y = localP1.y();
	Real z = localP1.z();
	int planeNo = s1->a.size();
	if (s1->phi_b.empty()){ return 0.0; }
	Real pSum2 = 0.0, plane; int activeNo = 0; intactRock = true;
	for (int i=0; i<planeNo; i++){
		plane = s1->a[i]*x + s1->b[i]*y + s1->c[i]*z - s1->d[i];
		if ( plane < pow(10,-15) ){ plane = 0.0; }else{
			if(activeNo==0){
//				hwater = s1->hwater[i];
				phi_b = s1->phi_b[i];
				phi_r = s1->phi_r[i];
				tension = s1->tension[i];
				cohesion = s1->cohesion[i];
				jointType = s1->jointType[i];
//				heatCapacity =s1->heatCapacity[i];
			}
//			JRC += s1->JRC[i];
//			JSC += s1->JCS[i];

//			sigmaC += s1->sigmaC[i];
//			asperity += s1->asperity[i];
//			lambda0 +=s1->lambda0[i];

//			hwater = std::max(hwater,s1->hwater[i]);
//			heatCapacity =std::max(heatCapacity, s1->heatCapacity[i]);
			phi_b = std::min(phi_b,s1->phi_b[i]);
			phi_r = std::min(phi_r,s1->phi_r[i]);
			tension = std::min(tension,s1->tension[i]);
			cohesion = std::min(cohesion,s1->cohesion[i]);
			jointType = std::max(jointType,s1->jointType[i]);

			if(s1->intactRock[i] == false){intactRock = false;}
			activeNo++;
			//if(lambda0>pow(10,-5)){std::cout<<"lambda0: "<<lambda0<<",s1->lambda0: "<<s1->lambda0[i]<<", i: "<<i<<", activeNo: "<<activeNo<<endl;}
		}
		pSum2 += pow(plane,2);
	}
	if(activeNo>0){
		phi_b = phi_b;
		phi_r = phi_r;
		cohesion = cohesion;
		tension = tension;
		jointType = jointType;
//		JRC = JRC/static_cast<Real>(activeNo);
//		JSC = JSC/static_cast<Real>(activeNo);
//		sigmaC = sigmaC/static_cast<Real>(activeNo);
//		asperity = asperity/static_cast<Real>(activeNo);
//		lambda0 = lambda0/static_cast<Real>(activeNo);
//		heatCapacity = heatCapacity;
//		hwater = hwater;
	}else{
		phi_b = s1->phi_b[0];
		phi_r = s1->phi_r[0];
		cohesion = s1->cohesion[0];
		tension = s1->tension[0];
		jointType = jointType;
//		JRC = s1->JRC[0];
//		JSC = s1->JCS[0];
//		sigmaC = s1->sigmaC[0];
//		asperity = s1->asperity[0];
//		lambda0 = s1->lambda0[0];
//		heatCapacity = 0.0;
//		hwater = -1.0;
	}
	Real r = s1->r; //Real R = s1->R; Real k = s1->k;
	/* Additional sphere */
	//Real sphere = (pow(x,2) + pow(y,2) + pow(z,2))/pow(R,2);
	/* Complete potential particle */
	Real f = pSum2 - pow(r,2); //FIXME: In this function we calculate pSum2 and f but we don't use them. The type of the function will be modified to "void".
	activePlanesNo = activeNo;
	return f;
}


/* ***************************************************************************************************************************** */
Real Ig2_PB_PB_ScGeom::getSignedArea(const Vector3r pt1, const Vector3r pt2, const Vector3r pt3){
	/* if positive, counter clockwise, 2nd point makes a larger angle */
	/* if negative, clockwise, 3rd point makes a larger angle */
	Eigen::MatrixXd triangle(4,2);
	triangle(0,0) = pt1.x();  triangle(0,1) = pt1.y(); // triangle(0,2) = pt1.z();
	triangle(1,0) = pt2.x();  triangle(1,1) = pt2.y(); // triangle(1,2) = pt2.z();
	triangle(2,0) = pt3.x();  triangle(2,1) = pt3.y(); // triangle(2,2) = pt3.z();
	triangle(3,0) = pt1.x();  triangle(3,1) = pt1.y(); // triangle(3,2) = pt1.z();
	Real determinant = getDet(triangle);
	return determinant; //triangle.determinant();
}


/* ***************************************************************************************************************************** */
Real Ig2_PB_PB_ScGeom::getAreaPolygon2(const shared_ptr<Shape>& cm1, const State& state1, const shared_ptr<Shape>& cm2, const State& state2,  const Vector3r& shift2, const Vector3r contactPoint, const Vector3r contactNormal, int& smaller, Vector3r shearDir, Real& jointLength, const bool twoD, Real unitWidth2D){
	//const Real PI = 3.14159265358979323846;
	Real areaTri = 0.0;
	//PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	//PotentialBlock *s2=static_cast<PotentialBlock*>(cm2.get());
	Real bisectionStepSize = 1.0;//*std::min(s1->R, s2->R);

	if(!twoD){ //3D contact - search counter-clockwise
		int countParticleA = 0, countParticleB = 0; 
		Vector3r ptOnBoundary = contactPoint;
		// Matrix3r rotationMatrix; //int count = 1;
		Vector3r orthogonalDir = Vector3r(contactNormal.y(), -contactNormal.x(), 0.0);

		if(orthogonalDir.norm() < pow(10,-5)){ orthogonalDir = Vector3r(contactNormal.z(), 0.0, -contactNormal.x()); } //TODO: Optimise these two ifs into a nested one
		if(orthogonalDir.norm() < pow(10,-5)){ orthogonalDir = Vector3r(0.0, contactNormal.z(), -contactNormal.y()); }
		orthogonalDir.normalize();
		Real tol = pow(10,-8);
		Vector3r orthogonalDir2 = contactNormal.cross(orthogonalDir); orthogonalDir2.normalize();

		Vector3r prevPoint = contactPoint;
		Matrix3r area1; Matrix3r area2; Matrix3r area3;
		//http://en.wikipedia.org/wiki/Triangle#Using_coordinates
		area1(0,0) = contactPoint.x();		area2(0,0) = contactPoint.y();		area3(0,0) = contactPoint.z();
		area1(1,0) = contactPoint.y();		area2(1,0) = contactPoint.z();		area3(1,0) = contactPoint.x();
		area1(2,0) = 1.0;			area2(2,0) = 1.0;			area3(2,0) = 1.0;

		Vector3r searchDirOri = orthogonalDir;
		Vector3r normalDir(0,0,0);
		Vector3r searchDir = searchDirOri;
//		prevPoint = ptOnBoundary;
		Vector3r ptOnP1(0,0,0); Vector3r ptOnP2(0,0,0);
		int prevNoA = -1, prevNoB = -1; Vector3r searchDirA(0,0,0);
		int newNoA  = -1, newNoB  = -1; Vector3r searchDirB(0,0,0);

		if( getPtOnParticleAreaNormal(cm1, state1, Vector3r(0,0,0), contactPoint, searchDir, prevNoA, ptOnP1, searchDirA, newNoA))
			{ prevNoA = newNoA; } else { prevNoA = -1; }
		if( getPtOnParticleAreaNormal(cm2, state2, shift2         , contactPoint, searchDir, prevNoB, ptOnP2, searchDirB, newNoB))
			{ prevNoB = newNoB; } else { prevNoB = -1; }

		if(prevNoA == -1 && prevNoB == -1){std::cout<<"before loop fail"<<endl;}

		Vector3r vec1 = (ptOnP1 - contactPoint);
		Vector3r vec2 = (ptOnP2 - contactPoint);
		if( vec1.norm() < vec2.norm() ){
			ptOnBoundary = ptOnP1; countParticleA++;
			normalDir = searchDirA;
			prevNoB = -1;
		}else{
			ptOnBoundary = ptOnP2; countParticleB++;
			normalDir = searchDirB;
			prevNoA = -1;
		}

		/* choose the direction which makes the largest angle between prevSearchDir & planeNormal */
		Vector3r firstPoint = ptOnBoundary;
		Vector3r newPt(0,0,0); int count = 0;
//		Vector3r firstAxis(0,0,0);
		Vector3r secondPoint(0,0,0);
		Vector3r newSearchDir;
		Real distanceBackup = 0.0;

		Vector3r v1, v2, v3, ptOnP1a, ptOnP2a, p1, p2;
		Vector3r dirA, dirB;
		Real dotA, dotB;

		do {
			newSearchDir = normalDir.cross(contactNormal);
			newSearchDir.normalize(); //FIXME lose accuracy

			/* clockwise */
			v1 = ptOnBoundary;
			v2 = ptOnBoundary + newSearchDir;
			v3 = contactPoint;

			v1.x() = (ptOnBoundary.dot(orthogonalDir));			v1.y() = (ptOnBoundary.dot(orthogonalDir2));			v1.z() = 0.0;
			v2.x() = ((ptOnBoundary + newSearchDir).dot(orthogonalDir));	v2.y() = ((ptOnBoundary + newSearchDir).dot(orthogonalDir2));	v2.z() = 0.0;
			v3.x() = (contactPoint.dot(orthogonalDir));			v3.y() = (contactPoint.dot(orthogonalDir2));			v3.z() = 0.0;

			if (getSignedArea(v1,v2,v3) > 0.0){
				/*counter-clockwise */
				newSearchDir = -newSearchDir;
			}

//			if (count == 0){
//				firstAxis = searchDirOri.cross(newSearchDir);
//			}

			searchDirOri = newSearchDir;
			searchDir = searchDirOri;//*bisectionStepSize;

			prevPoint = ptOnBoundary;
			ptOnP1a = Vector3r(0,0,0); ptOnP2a = Vector3r(0,0,0);

			if( getPtOnParticleAreaNormal(cm1, state1, Vector3r(0,0,0) /* shift2 */, ptOnBoundary, searchDir, prevNoA, ptOnP1a, searchDirA, newNoA)){
				prevNoA = newNoA;
			}else{
				std::cout<<"no intersection A? "<<endl;
				prevNoA = -1;
			}
			if ( getPtOnParticleAreaNormal(cm2, state2, shift2, ptOnBoundary, searchDir, prevNoB, ptOnP2a, searchDirB, newNoB)){
				prevNoB = newNoB;
			}else{
				std::cout<<"no intersection B? "<<endl;
				prevNoB = -1;
			}
			if(prevNoA == -1 && prevNoB == -1){std::cout<<"loop fail"<<endl;}

			p1 = (ptOnP1a - prevPoint);
			p2 = (ptOnP2a - prevPoint);
			if ((p1-p2).norm() > pow(10,-6)) {
				if( p1.norm() < p2.norm() ){
					ptOnBoundary = ptOnP1a; countParticleA++;
					normalDir = searchDirA;
					prevNoB = -1;
				}else{
					ptOnBoundary = ptOnP2a; countParticleB++;
					normalDir = searchDirB;
					prevNoA = -1;
				}
			}else{
				dirA = searchDirA - (searchDirA.dot(contactNormal))*contactNormal; dirA.normalize();
				dirB = searchDirB - (searchDirB.dot(contactNormal))*contactNormal; dirB.normalize();
				dotA = dirA.dot(normalDir);
				dotB = dirB.dot(normalDir);
				if (dotA < dotB) { //the larger the angle, the smaller the dot product (perpendicular)
					ptOnBoundary = ptOnP1a; countParticleA++;
					normalDir = searchDirA;
					prevNoB = -1;
				}else{
					ptOnBoundary = ptOnP2a; countParticleB++;
					normalDir = searchDirB;
					prevNoA = -1;
				}
			}

			if(count >= 2){
				area1(0,1) = prevPoint.x();	area2(0,1) = prevPoint.y();	area3(0,1) = prevPoint.z();
				area1(1,1) = prevPoint.y();	area2(1,1) = prevPoint.z();	area3(1,1) = prevPoint.x();
				area1(2,1) = 1.0;		area2(2,1) = 1.0;		area3(2,1) = 1.0;

				area1(0,2) = ptOnBoundary.x();	area2(0,2) = ptOnBoundary.y();	area3(0,2) = ptOnBoundary.z();
				area1(1,2) = ptOnBoundary.y();	area2(1,2) = ptOnBoundary.z();	area3(1,2) = ptOnBoundary.x();
				area1(2,2) = 1.0;		area2(2,2) = 1.0;		area3(2,2) = 1.0;

				areaTri += 0.5*sqrt(( pow(area1.determinant(),2) + pow(area2.determinant(),2) + pow(area3.determinant(),2) ));
			}

			if(count==1){
				secondPoint = ptOnBoundary;
			}
			//#if 0
			if(count==10){
				distanceBackup = (newPt-secondPoint).norm(); areaTri = 0.0;
			}
			if(count>12){
				if ( fabs((newPt-secondPoint).norm() - distanceBackup )<pow(10,-7) ){break;} //FIXME: Here we should output a warning if the calculation for the contactArea is stopped
			}
			//#endif
			newPt = ptOnBoundary;

			count++;
			if(count>60){	std::cout<<"area polygon count: "<<count<<", distance: "<<(newPt-firstPoint).norm()<<", dist2nd: "<<(newPt-secondPoint).norm()<<", prevNoA: "<<prevNoA<<", prevNoB: "<<prevNoB<<endl;areaTri = 0.0; tol=100.0*tol; }
			//std::cout<<"area polygon count: "<<count<<", distance: "<<(newPt-firstPoint).norm()<<", dist2nd: "<<(newPt-secondPoint).norm()<<endl;
			if(count==70){break;} //FIXME: This can break the calculation of contactArea for particles with very complex shape, where the contact area is a polygon with more than 70 faces. Either increase the number or delete this line all together @vsangelidakis. EDIT: This check is supposed to prevent infinite loops, but anyway we should output a warning if we stop the calculation of the contactArea midway
		}while(count<4 || /*( */ (newPt-secondPoint).norm() > tol);// && (newPt-secondPoint).norm() > tol) );
		//std::cout<<"area polygon count: "<<count<<endl;
		if(countParticleA > countParticleB){smaller = 1;}else{smaller = 2;}
	} else { //2D contact (twoD=true)
		Vector3r searchDir = shearDir;
		Vector3r ptOnBoundary1(0,0,0), ptOnBoundary2(0,0,0);

		//if(searchDir.norm() < pow(10,-11)){
			searchDir = contactNormal.cross(twoDdir);
			if(searchDir.squaredNorm() < pow(10,-14) ){
				Vector3r xDir = Vector3r(1,0,0); //FIXME: Instead of the hardcoded Vector3r(1,0,0), here we should define a perpendicular direction to the twoDir attr in the 2D plane where the particles are defined.
				searchDir = xDir.cross(twoDdir);
			}
		//}
		searchDir.normalize();
		searchDir *= bisectionStepSize;
		//if(jointLength > pow(10,-5)){searchDir = jointLength*searchDir;}
		Vector3r ptOnP1(0,0,0); Vector3r ptOnP2(0,0,0);
		getPtOnParticle2(cm1, state1, Vector3r(0,0,0), contactPoint, searchDir, ptOnP1);
		getPtOnParticle2(cm2, state2, shift2,          contactPoint, searchDir, ptOnP2);

		if( (ptOnP1 - contactPoint).squaredNorm() < (ptOnP2 - contactPoint).squaredNorm() ){
			ptOnBoundary1 = ptOnP1;
		}else{
			ptOnBoundary1 = ptOnP2;
		}
		getPtOnParticle2(cm1, state1, Vector3r(0,0,0), contactPoint, -searchDir, ptOnP1);
		getPtOnParticle2(cm2, state2, shift2,          contactPoint, -searchDir, ptOnP2);

		if( (ptOnP1 - contactPoint).squaredNorm() < (ptOnP2 - contactPoint).squaredNorm() ){
			ptOnBoundary2 = ptOnP1;
		}else{
			ptOnBoundary2 = ptOnP2;
		}
		jointLength = (ptOnBoundary1- ptOnBoundary2).norm();


		if( std::isnan(jointLength) ) {jointLength = 1.0; /*std::min(s1->R,s2->R);*/} //FIXME: It's best we output a warning if this happens. Instead of setting the jointLength equal to 1.0,  we can alternativelly set it equal to its previous value or to the distance "R" of the smallest particle, as in the comment above.

		areaTri = unitWidth2D*jointLength; //Contact area of 2-D contact
	}
	//std::cout<<"searchDir: "<<searchDir<<", calJointLength: "<<calJointLength<<", jointLength: "<<jointLength<<", ptOnBoundary1: "<<ptOnBoundary1<<", ptOnBoundary2: "<<ptOnBoundary2<<endl;

	return areaTri;
}


/* ***************************************************************************************************************************** */
bool Ig2_PB_PB_ScGeom::getPtOnParticleAreaNormal(const shared_ptr<Shape>& cm1, const State& state1, const Vector3r& shift2, const Vector3r previousPt, const Vector3r prevDir, const int prevNo, Vector3r& newPt, Vector3r& newNormal, int& newNo){

//intersect a line and a plane other than the ID of the previous plane
	PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	//Matrix3r Q1 = state1.ori.toRotationMatrix();
	Quaternionr Q1 = state1.ori;
	int planeNo = s1->a.size();
	Vector3r intersection (0,0,0);
	Real closestDistance = pow(10,11);
	newNo = -1;
	//Real fA = evaluatePB(cm1, state1, previousPt);
	//std::cout<<"fA: "<<fA<<endl;

	Vector3r planeNormal, newPoint;
	Real d, dotProd, u, testDistance;

	for (int i=0; i<planeNo; i++){
		if( i == prevNo){continue;}
		planeNormal = Vector3r(s1->a[i], s1->b[i], s1->c[i]);
		planeNormal = Q1*planeNormal;
		d = -1.0*( planeNormal.x()*(-state1.pos.x()-shift2[0]) + planeNormal.y()*(-state1.pos.y()-shift2[1]) + planeNormal.z()*(-state1.pos.z()-shift2[2]) - s1->d[i] -s1->r);
		dotProd = planeNormal.dot(prevDir);
		if( Mathr::Sign(dotProd) > 0.0 && fabs(dotProd) > pow(10.0,-3) ){ //planeNormal and searchDirection converges
			u = (planeNormal.dot(previousPt) - d)/(planeNormal.dot(-prevDir));
			newPoint = previousPt + u*prevDir;
			if (fabs(planeNormal.dot(newPoint) - d ) > pow(10,-5) ){
				std::cout<<"distance to plane: "<< fabs(planeNormal.dot(newPoint) - d ) <<", planeNormal: "<<planeNormal<<", prevDir: "<<prevDir<<", u: "<<u<<", previousPt: "<<previousPt<<", d: "<<d<<", (planeNormal.dot(-prevDir)): "<<(planeNormal.dot(-prevDir))<<", (planeNormal.dot(previousPt) - d): "<<(planeNormal.dot(previousPt) - d)<<endl;
			}

			#if 0
			Real check = planeNormal.dot(newPoint) - d;
			if(prevNo != -1){
				for(int j=0; j<planeNo; j++){
					Vector3r planeNormal1 (s1->a[j], s1->b[j], s1->c[j]);
					planeNormal1 = Q1*planeNormal1;
					Real d1 = -1.0*( planeNormal1.x()*(-state1.pos.x()) + planeNormal1.y()*(-state1.pos.y()) + planeNormal1.z()*(-state1.pos.z()) - s1->d[j] -s1->r);
					Real check1 = planeNormal1.dot(newPoint) - d1;
					Real check1ori = planeNormal1.dot(previousPt) - d1;
					Real dotCheck = planeNormal1.dot(prevDir);
					std::cout<<"j: "<<j<<", prevNo: "<<prevNo<<", prevDir: "<<prevDir<<", planeNormal1: "<<planeNormal1<<", d1: "<<d1<<", newPoint: "<<newPoint<<", check1: "<<check1<<", check1ori: "<<check1ori<<", dotCheck: "<<dotCheck<<", check: "<<check<<", u: "<<u<<", i: "<<i<<endl;
				}
			}
			#endif
			//if(fabs(check)>0.0001){std::cout<<"check : "<<check<<endl;}
			//std::cout<<"check : "<<check<<", planeNormal: "<<planeNormal<<", previousPt: "<<previousPt<<endl;
			testDistance = (newPoint-previousPt).norm();
			if ( testDistance < closestDistance){
				closestDistance = testDistance;
				intersection = newPoint;
				newNormal = planeNormal;
				newNo = i;
			}
		}
	}

	//std::cout<<"newNormal: "<<newNormal<<", newNo: "<<newNo<<", intersection: "<<intersection<<", previousPt: "<<previousPt<<endl;
	newPt = intersection;
	if (newNo<0){return false;}
	return true;
}


/* ***************************************************************************************************************************** */
//void Ig2_PB_PB_ScGeom::getPtOnParticleArea(const shared_ptr<Shape>& cm1, const State& state1, Vector3r midPoint, Vector3r normal, Vector3r& ptOnParticle){
//	//PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
//	ptOnParticle = midPoint;
//	Real f = evaluatePB(cm1, state1, ptOnParticle); //SecondOrderCorrection(cm1,state1, ptOnParticle);
//	Real fprevious = f;
//	Real fOri =evaluatePB(cm1, state1, ptOnParticle);
//	int counter = 0;
//	//normal.normalize();
//	Vector3r step = normal*Mathr::Sign(f) *-1.0;
//	Vector3r bracketA(0,0,0);
//	Vector3r bracketB(0,0,0);
//	Vector3r prevPt = ptOnParticle;

//	do{
//		prevPt = ptOnParticle;
//		ptOnParticle += step;
//		fprevious = f;
//		f = evaluatePB(cm1, state1, ptOnParticle); //SecondOrderCorrection(cm1,state1, ptOnParticle);
//		counter++;
//		if (counter == 50000 ){
//			//LOG_WARN("Initial point searching exceeded 500 iterations!");
//			std::cout<<"ptonparticleArea search exceeded 50000 iterations! step:"<<step<<", fOri: "<<fOri<<endl;
//		}
//	}while(Mathr::Sign(fprevious)*Mathr::Sign(f)*1.0> 0.0 );

//	bracketA = ptOnParticle;
//	bracketB = prevPt; //ptOnParticle -step;
//	Vector3r zero(0,0,0);
//	BrentZeroSurf(cm1,state1,bracketA, bracketB, zero);
//	ptOnParticle = zero;
//	//if( fabs(f)>0.1){std::cout<<"getInitial point f:"<<f<<endl;}
//}


/* ***************************************************************************************************************************** */
bool Ig2_PB_PB_ScGeom::customSolveAnalyticCentre(const shared_ptr<Shape>& cm1, const State& state1, const shared_ptr<Shape>& cm2, const State& state2, const Vector3r& shift2, Vector3r& contactPt){

	bool converge = true;
	PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	PotentialBlock *s2=static_cast<PotentialBlock*>(cm2.get());
	int planeNoA = s1->a.size();
	int planeNoB = s2->a.size();
	int totalPlanes = planeNoA+planeNoB;
	Matrix3r QA = state1.ori.toRotationMatrix(); /*direction cosine */
	Matrix3r QB = state2.ori.toRotationMatrix(); /*direction cosine */
	Real blasQ1[3*3]; Real blasQ2[3*3];
	Real blasP1Q [planeNoA*3]; Real d1 [planeNoA];
	Real blasP2Q [planeNoB*3]; Real d2 [planeNoB];
	int blasCount = 0;
	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			blasQ1[blasCount]=QA(j,i);
			blasQ2[blasCount]=QB(j,i);
			blasCount++;
		}
	}
	Real blasP1[planeNoA*3];
	for(int i=0; i<planeNoA; i++){
		blasP1[3*i]=s1->a[i]; blasP1[3*i+1]=s1->b[i]; blasP1[3*i+2]=s1->c[i]; d1[i] = s1->d[i] + s1->r;
	}
	Real blasP2[planeNoB*3];
	for(int i=0; i<planeNoB; i++){
		blasP2[3*i]=s2->a[i]; blasP2[3*i+1]=s2->b[i]; blasP2[3*i+2]=s2->c[i]; d2[i] = s2->d[i] + s2->r;
	}
	int incx =1; int incy=1;
	int blas3 = 3; char blasNT = 'N'; char blasT= 'T'; Real blas0 = 0.0; Real blas1 = 1.0; Real blasNeg1 = -1.0; Real HessTemp[3*totalPlanes];
	dgemm_(&blasNT, &blasNT, &blas3, &planeNoA, &blas3, &blas1, &blasQ1[0], &blas3, &blasP1[0], &blas3, &blas0, &blasP1Q[0], &blas3); /*Matrix mulitplication Q1*P1 */
	dgemm_(&blasNT, &blasNT, &blas3, &planeNoB, &blas3, &blas1, &blasQ2[0], &blas3, &blasP2[0], &blas3, &blas0, &blasP2Q[0], &blas3); /*Matrix mulitplication Q2*P2 */

	for(int k=0; k<planeNoA; k++){
		d1[k] = -1.0*(blasP1Q[3*k]*(contactPt.x()-state1.pos.x()) + blasP1Q[3*k+1]*(contactPt.y()-state1.pos.y()) + blasP1Q[3*k+2]*(contactPt.z()-state1.pos.z())-d1[k] );
	}
	for(int k=0; k<planeNoB; k++){
		d2[k] = -1.0*(blasP2Q[3*k]*(contactPt.x()-state2.pos.x()-shift2[0]) + blasP2Q[3*k+1]*(contactPt.y()-state2.pos.y()-shift2[1]) + blasP2Q[3*k+2]*(contactPt.z()-state2.pos.z()-shift2[2])-d2[k] );
	}

	int iter=0;
	Real xx [3]; xx[0]=0.0; xx[1]=0.0; xx[2]=0.0; Real blasStep[3];
	Real grad [3];
	Real Hess [9];
	Real Atranspose[totalPlanes*3];
	Real B [totalPlanes]; Real D [totalPlanes]; Real Dinvert [totalPlanes];
	Real Ddiag[totalPlanes*totalPlanes]; Real oriMinD = 9999999;
	memset(Ddiag,0.0,sizeof(Ddiag));
	//int planeNoA3 = planeNoA*3; int planeNoB3 = planeNoB*3; int totalPlanes3 = totalPlanes*3;

	for (int i=0; i<planeNoA; i++){
		Atranspose[3*i] = blasP1Q[3*i];
		Atranspose[3*i+1] = blasP1Q[3*i+1];
		Atranspose[3*i+2] = blasP1Q[3*i+2];
		B[i] = d1[i];
	}
	for (int i=0; i<planeNoB; i++){
		Atranspose[3*(planeNoA+i)] =blasP2Q[3*i];
		Atranspose[3*(planeNoA+i)+1] = blasP2Q[3*i+1];
		Atranspose[3*(planeNoA+i)+2] = blasP2Q[3*i+2];
		B[planeNoA+i] = d2[i];
	}


	//TODO: Below, we need to move some variable type declarations outside the "while" and "for" loops & replace the C arrays with std::vector<>
	Real val, a, b, c, orival;
	Real backtrack;  Real blasFprime; Real minD;
	Real initbacktrack;
	Vector3r solution;

	while(iter<50){
		val = 0.0; //Real Dinit[totalPlanes];
		for (int i=0; i<totalPlanes; i++){
			a = Atranspose[3*i]; b = Atranspose[3*i+1]; c = Atranspose[3*i+2];
			D[i] = B[i]-a*xx[0] -b*xx[1] -c*xx[2];
			if(iter==0 && D[i] < oriMinD){ oriMinD = D[i]; }
			//Dinit[i]=D[i];
			val -= log(D[i]);
			Dinvert[i] = 1.0/D[i];
			Ddiag[i*totalPlanes+i] = pow(Dinvert[i],2);
		}
		orival = val;
		if(iter==0 && Mathr::Sign(oriMinD)*1.0<0.0 ){std::cout<<"oriMinD: "<<oriMinD<<endl; converge = false; break;}
		/* g = A(T)*(1./d) */
		memset(grad,0.0,sizeof(grad));
		dgemv_(&blasNT, &blas3, &totalPlanes, &blas1, &Atranspose[0], &blas3, &Dinvert[0], &incx, &blas0, &grad[0], &incy);
		/* H = A(t)*diag(1/d^2)*A */
		memset(HessTemp,0.0,sizeof(HessTemp));
		memset(Hess,0.0,sizeof(Hess));
		dgemm_(&blasNT, &blasNT, &blas3, &totalPlanes, &totalPlanes, &blas1, &Atranspose[0], &blas3, &Ddiag[0], &totalPlanes, &blas0, &HessTemp[0], &blas3);
		dgemm_(&blasNT, &blasT, &blas3, &blas3, &totalPlanes, &blas1, &HessTemp[0], &blas3, &Atranspose[0], &blas3, &blas0, &Hess[0], &blas3);

		/* Cholesky factorization */
		//#if 0
		/* L */
			int varNo = 3;
			int info; char UPLO ='L'; int KD = varNo-1; int nrhs=1; //FIXME: varNo, UPLO, KD, nrhs could be defined as "const" variables
			Real HessianChol[varNo][varNo];
			for( int j=1; j<=varNo ; j++){
				blasStep[j-1]=-grad[j-1];
				for (int i=j; i<=j+KD && i<=varNo; i++){
					HessianChol[j-1][1+i-j-1] = Hess[(i-1)+varNo*(j-1)];
				}
			}
		//#endif

		#if 0
			/* U */
			for( int j=1; j<=varNo ; j++){
				blasStep[j-1]=-blasGrad[j-1];
				for (int i=std::max(1,j-KD); i<=j ; i++){
					HessianChol[j-1][KD+1+i-j-1] = blasHess[(i-1)+varNo*(j-1)]; //H(i-1,j-1);
				}
			}
		#endif
		//#if 0
			dpbsv_( &UPLO, &varNo, &KD, &nrhs, &HessianChol[0][0], &varNo, blasStep, &varNo, &info );
			if(info != 0){
			//	std::cout<<"chol error iter:"<<iter<<endl;
				//return false;
				/* LU factorization */
				for (int i=0; i<varNo; i++ ){
					blasStep[i]=-1.0*grad[i]; //g[i];
				}
				int ipiv[varNo]; int bColNo=1;
				dgesv_( &varNo, &bColNo, Hess, &varNo, ipiv, blasStep, &varNo, &info);
			}
		//#endif
		#if 0
			/* QR */
			for (int i=0; i<varNo; i++ ){
					blasStep[i]=-1.0*grad[i]; //g[i];
			}
			Real work[30]; int lwork = 30;
			dgels_(&blasT, &blas3, &blas3, &nrhs, Hess, &blas3, blasStep, &blas3, work, &lwork, &info);
		#endif
			if (info!=0){
				std::cout<<"linear algebra error, iter:"<<iter<<endl; converge=false;break;
			}
		//std::cout<<"blasStep: "<<blasStep[0]<<", "<<blasStep[1]<<", "<<blasStep[2]<<endl;
		 /*Linesearch */
		 backtrack = 1.0; Real blasNewX[3];
		 dcopy_(&varNo, &xx[0], &incx, &blasNewX[0], &incy);
		 daxpy_(&varNo, &backtrack, &blasStep[0], &incx, &blasNewX[0], &incy);
		 //fprime = step.transpose()*g;
		 blasFprime = ddot_(&varNo, &blasStep[0], &incx, &grad[0], &incy);
		 if (-blasFprime*0.5 < pow(10,-8) ){
			//std::cout<<"blasFprime: "<<blasFprime<<", blasStep: "<<blasStep[0]<<", "<<blasStep[1]<<", "<<blasStep[2]<<endl;
			break;
		 }
		 minD = 9999999999;
		 /* d = B - A*x	*/
		 dcopy_(&totalPlanes, &B[0], &incx, &D[0], &incy);
		 dgemv_(&blasT, &blas3, &totalPlanes, &blasNeg1, &Atranspose[0], &blas3, &blasNewX[0], &incx, &blas1, &D[0], &incy);
		 for(int i=0; i<totalPlanes; i++){
			if(D[i] < minD){
				minD = D[i];
			}
		 }
		 while (Mathr::Sign(minD)*1.0 < 0.0){
			 backtrack *= 0.5;
			 dcopy_(&varNo, &xx[0], &incx, &blasNewX[0], &incy);
			 daxpy_(&varNo, &backtrack, &blasStep[0], &incx, &blasNewX[0], &incy);
			 minD = 9999999999;
			 /* d = B - A*x */
			 dcopy_(&totalPlanes, &B[0], &incx, &D[0], &incy);
			 dgemv_(&blasT, &blas3, &totalPlanes, &blasNeg1, &Atranspose[0], &blas3, &blasNewX[0], &incx, &blas1, &D[0], &incy);

			 for(int i=0; i<totalPlanes; i++){
				if(D[i] < minD){
					minD = D[i];
				}
			 }
			if(backtrack<pow(10,-15)){std::cout<<"backtrack: "<<backtrack<<", iter: "<<iter<<", blasStep: "<<blasStep[0]<<","<<blasStep[1]<<","<<blasStep[2]<<", minD: "<<minD<<", oriMinD: "<<oriMinD<<", grad: "<<grad[0]<<","<<grad[1]<<","<<grad[2]<<", xx: "<<xx[0]<<","<<xx[1]<<","<<xx[2]<<", Hess: "<<Hess[0]<<","<<Hess[1]<<","<<Hess[2]<<","<<Hess[3]<<","<<Hess[4]<<","<<Hess[5]<<","<<Hess[6]<<","<<Hess[7]<<","<<Hess[8]<<endl;converge=false;break;}
		 }

		/* d = B - A*x */
		dcopy_(&totalPlanes, &B[0], &incx, &D[0], &incy);
		dgemv_(&blasT, &blas3, &totalPlanes, &blasNeg1, &Atranspose[0], &totalPlanes, &blasNewX[0], &incx, &blas1, &D[0], &incy);

		 val = 0.0;
		 for(int i=0; i<totalPlanes; i++){
			val -= log(D[i]);
		 }

		 initbacktrack = backtrack;
		 while (val > orival+ backtrack*0.01*blasFprime){

			 backtrack *= 0.5;
			 dcopy_(&varNo, &xx[0], &incx, &blasNewX[0], &incy);
			 daxpy_(&varNo, &backtrack, &blasStep[0], &incx, &blasNewX[0], &incy);
			 /* d = B - A*x */
			// dcopy_(&totalPlanes, &B[0], &incx, &D[0], &incy);
			// dgemv_(&blasT, &blas3, &totalPlanes, &blasNeg1, &Atranspose[0], &blas3, &blasNewX[0], &incx, &blas1, &D[0], &incy);
			 val=0.0;
			 for(int i=0; i<totalPlanes; i++){
				a = Atranspose[3*i]; b = Atranspose[3*i+1]; c = Atranspose[3*i+2];
				D[i] = B[i]-a*blasNewX[0] -b*blasNewX[1] -c*blasNewX[2];
				val -= log(D[i]);
			 }
			 if(backtrack<pow(10,-15)){
				std::cout<<"initbacktrack: "<<initbacktrack<<", backtrack: "<<backtrack<<", iter: "<<iter<<", blasStep: "<<blasStep[0]<<","<<blasStep[1]<<","<<blasStep[2]<<", blasFprime: "<<blasFprime<<", val: "<<val<<", orival: "<<orival<<", oriMinD: "<<oriMinD<<", minD: "<<minD<<endl;

				converge=false;break;
			 }
		 }

		dcopy_(&blas3, &blasNewX[0], &incx, &xx[0], &incy);
		iter++;

		if(iter>49){std::cout<<"custom analytic center iter: "<<iter<<", blasFprime: "<<blasFprime<<endl;converge=false;}
		if(converge==false){break;}
	 }

	solution = Vector3r( (xx[0]-0.0), (xx[1]-0.0), (xx[2]-0.0) );

	if(converge){ contactPt = solution+contactPt; }
	return converge;
}


/* ***************************************************************************************************************************** */
Real Ig2_PB_PB_ScGeom::getDet(const Eigen::MatrixXd A){
	/* if positive, counter clockwise, 2nd point makes a larger angle */
	/* if negative, clockwise, 3rd point makes a larger angle */
	int rowNo = A.rows(); Real firstTerm = 0.0, secondTerm = 0.0;
	for(int i=0; i<rowNo-1; i++){
		firstTerm += A(i,0)*A(i+1,1);
		secondTerm += A(i,1)*A(i+1,0);
	}
	return firstTerm-secondTerm;
}


/* ***************************************************************************************************************************** */
bool Ig2_PB_PB_ScGeom::startingPointFeasibilityCLP(const shared_ptr<Shape>& cm1, const State& state1, const shared_ptr<Shape>& cm2, const State& state2, const Vector3r& shift2, Vector3r &contactPoint/*, bool &convergeFeasibility*/){
//timingDeltas->start();
//FIXME: rescale variable below might be unnecessary
  Vector3r xGlobal (0,0,0); Real rescale = 1.0; //bool converge = true;
/* minimise s */
/* s.t. Ax - s <= d*/
  PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
  PotentialBlock *s2=static_cast<PotentialBlock*>(cm2.get());

/* Parameters for particles A and B */
  int planeNoA = s1->a.size();
  int planeNoB = s2->a.size();
  Matrix3r Q1 = state1.ori.toRotationMatrix().transpose(); //(state1.ori.conjugate()).toRotationMatrix();
  Matrix3r Q2 = state2.ori.toRotationMatrix().transpose(); //(state2.ori.conjugate()).toRotationMatrix();
  Eigen::MatrixXd A1 = Eigen::MatrixXd::Zero(planeNoA,3);
  for (int i=0; i < planeNoA; i++){
	A1(i,0) = s1->a[i]/rescale; A1(i,1) = s1->b[i]/rescale; A1(i,2) = s1->c[i]/rescale;
  }
  Eigen::MatrixXd A2 = Eigen::MatrixXd::Zero(planeNoB,3);
  for (int i=0; i < planeNoB; i++){
	A2(i,0) = s2->a[i]/rescale; A2(i,1) =s2->b[i]/rescale; A2(i,2) = s2->c[i]/rescale;
  }
  Eigen::MatrixXd AQ1 = A1*Q1;
  Eigen::MatrixXd AQ2 = A2*Q2;

  Eigen::MatrixXd pos1(3,1);
  pos1(0,0) = rescale*state1.pos.x();
  pos1(1,0) = rescale*state1.pos.y();
  pos1(2,0) = rescale*state1.pos.z();
  Eigen::MatrixXd Q1pos1 = AQ1*pos1;

  Eigen::MatrixXd pos2(3,1);
  pos2(0,0) = rescale * ( state2.pos.x() + shift2[0] );
  pos2(1,0) = rescale * ( state2.pos.y() + shift2[1] );
  pos2(2,0) = rescale * ( state2.pos.z() + shift2[2] );
  Eigen::MatrixXd Q2pos2 = AQ2*pos2;

 // Eigen::MatrixXd xinit(3,1);
  Eigen::MatrixXd b1 = Q1pos1;// - AQ1*xinit;
  Eigen::MatrixXd b2 = Q2pos2;// - AQ2*xinit;
  //Real constraintInit[planeNoA+planeNoB];
  //for (int i=0; i<planeNoA; i++){
  //	constraintInit[i] = b1(i,0);
  //}
  //for (int i=0; i<planeNoB; i++){
  //	constraintInit[planeNoA+i] = b2(i,0);
  //}
/* Parameters for particles A and B */
  Real s = 0.0; /* get value of x[3] after optimization */
  int NUMCON = planeNoA + planeNoB;
  int NUMVAR = 3/*3D*/ +1;


ClpSimplex  model2;
int numberRows    = NUMCON;
int numberColumns = NUMVAR;
model2.resize(0, numberColumns);

model2.setObjectiveCoefficient(0, 0.0);
model2.setObjectiveCoefficient(1, 0.0);
model2.setObjectiveCoefficient(2, 0.0);
model2.setObjectiveCoefficient(3, 1.0);

model2.setColumnLower(0, -COIN_DBL_MAX);
model2.setColumnLower(1, -COIN_DBL_MAX);
model2.setColumnLower(2, -COIN_DBL_MAX);
model2.setColumnLower(3, -COIN_DBL_MAX);

model2.setColumnUpper(0,  COIN_DBL_MAX);
model2.setColumnUpper(1,  COIN_DBL_MAX);
model2.setColumnUpper(2,  COIN_DBL_MAX);
model2.setColumnUpper(3,  COIN_DBL_MAX);

Real rowLower[numberRows];
Real rowUpper[numberRows];

// Rows
for( int i=0; i<planeNoA;   i++ ){ rowUpper[i]            = s1->d[i] + s1->r + Q1pos1(i,0); }
for( int i=0; i<planeNoB;   i++ ){ rowUpper[planeNoA + i] = s2->d[i] + s2->r + Q2pos2(i,0); }
for( int k=0; k<numberRows; k++ ){ rowLower[k] = -COIN_DBL_MAX; }

for( int i=0; i < planeNoA; i++ ){
	int rowIndex[] = {0, 1, 2, 3};
	Real rowValue[] = {AQ1(i,0), AQ1(i,1), AQ1(i,2), -1.0};
	model2.addRow(4, rowIndex, rowValue,rowLower[i], rowUpper[i]);
}
for( int i=0; i < planeNoB; i++ ){
	int rowIndex[] = {0, 1, 2, 3};
	Real rowValue[] = {AQ2(i,0), AQ2(i,1), AQ2(i,2), -1.0};
	model2.addRow(4, rowIndex, rowValue,rowLower[planeNoA+i], rowUpper[planeNoA+i]);
}


model2.setLogLevel(0);
   model2.scaling(0);
   model2.setPrimalTolerance(pow(10,-9)) ;
   model2.primal();
   //model2.writeMps("a.mps");

          // Alternatively getColSolution()
          Real * columnPrimal = model2.primalColumnSolution();

    xGlobal = Vector3r(columnPrimal[0],columnPrimal[1],columnPrimal[2])/rescale;
    contactPoint = xGlobal;
    s = columnPrimal[3];

   if( s>-pow(10,-15) || model2.status()!=0 ){
	return false;
   }else{
	return true;
   }
}


/* ***************************************************************************************************************************** */
void Ig2_PB_PB_ScGeom::BrentZeroSurf(const shared_ptr<Shape>& cm1, const State& state1, const Vector3r& shift2, const Vector3r bracketA, const Vector3r bracketB, Vector3r& zero){

	Real a = 0.0; Real b = 1.0;
	Real t = pow(10,-16);
	Real m = 0.0;
	Real p = 0.0;
	Real q = 0.0;
	Real r = 0.0;
	Real s = 0.0;
	Real c = 0.0;
	Real d = 0.0;
	Real e = 0.0;
	Real h;
	Vector3r bracketRange = bracketB- bracketA;
	Real fa = evaluatePB(cm1, state1, shift2, bracketA); //evaluateFNoSphere(cm1,state1, bracketA); //
	Real fb = evaluatePB(cm1, state1, shift2, bracketB); //evaluateFNoSphere(cm1,state1, bracketB); //

	if(fa*fb > 0.00001){ //FIXME: Check whether we need to output a warning here, or else comment this statement. This is commented in the PPs
		std::cout<<"fa: "<<fa<<", fb: "<<fb<<endl;
	}
	//if(fabs(fa)<fabs(fb)){bracketRange *= -1.0; Vector3r temp = bracketA; bracketA= bracketB; bracketB = temp;}
	Real fc = 0.0;

	Real tol = 2.0*DBL_EPSILON*fabs(b) + t;
	int counter = 0;
	do {
		if(counter == 0){
			c = a; fc = fa; d = b-a; e = d;
			if (fabs(fc) < fabs(fb) ){
				a = b; b = c; c = a;
				fa = fb; fb = fc; fc = fa;
			}
		}else{
			if (fb*fc > 0.0){
				c = a; fc = fa; d = b-a; e = d;
			}
			if(fabs(fc) < fabs(fb) ){
				a = b; b = c; c = a;
				fa = fb; fb = fc; fc = fa;
			}
		}
		tol = 2.0*DBL_EPSILON*fabs(b) + t; m = 0.5*(c-b);
		if (fabs(m) > tol && fabs(fb) > pow(10,-13) ){
			if(fabs(e) < tol || fabs(fa) <= fabs(fb) ){
				d = m;
				e = d;
			}else{
				s = fb/fa;
				if(fabs(a - c)<pow(10,-15)){
					p = 2.0*m*s; q = 1.0 - s;
				}else{
					q = fa/fc; r = fb/fc;
					p = s*(2.0*m*q*(q-r) - (b-a)*(r-1.0));
					q = (q-1.0)*(r-1.0)*(s-1.0);
				}

				if (p > 0.0){
					q = -q;
				}else{
					p = -p;
				}

				s = e; e = d;

				if(( 2.0*p < (3.0*m*q - fabs(tol*q))) && (p < fabs(0.5*s*q)) ){
					d = p/q;
				}else{
					d = m;
					e = d;
				}
			}

			a = b; fa = fb;
			h = 0.0;
			if(fabs(d) >tol){
				h = d;
			}else if(m > 0.0){
				h = tol;
			}else{
				h = - tol;
			}
			b = b + h; // h*m_unitVec;

			zero = bracketA + b*bracketRange;
			fb = evaluatePB(cm1, state1, Vector3r(0,0,0), zero); //evaluateFNoSphere(cm1,state1, zero); //
		}else{
			zero = bracketA + b*bracketRange;
			//std::cout<<"b: "<<b<<", m: "<<m<<", tol: "<<tol<<", c: "<<c<<", a: "<<a<<endl;
			break;
		}

		counter++;
		if(counter==10000){ //FIXME: Check whether we need to output a warning here, or else comment this statement. This is commented in the PPs
			std::cout<<"counter: "<<counter<<", m.norm: "<<m<<", fb: "<<fb<<endl;
		}
	}while(1);
	//}while( m.norm() > tol && fabs(fb) > pow(10,-13) );

}


/* ***************************************************************************************************************************** */
/* Routine to get value of function (constraint) at a particular position */
Real Ig2_PB_PB_ScGeom::evaluatePB(const shared_ptr<Shape>& cm1, const State& state1, const Vector3r& shift2, const Vector3r newTrial){
	PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	///////////////////Transforming trial values to local frame of particles//////////////////
	Vector3r tempP1 = newTrial - state1.pos - shift2;
	/* Direction cosines */
	//state1.ori.normalize();
	Vector3r localP1 = state1.ori.conjugate()*tempP1;
	Real x = localP1.x();
	Real y = localP1.y();
	Real z = localP1.z();
	int planeNo = s1->a.size();

	Real r = s1->r;  int insideCount = 0; Real plane;
	for (int i=0; i<planeNo; i++){
		plane = s1->a[i]*x + s1->b[i]*y + s1->c[i]*z - s1->d[i] - r; //-pow(10,-10);
		if (Mathr::Sign(plane)*1.0<0.0){
			insideCount++;
		}
	}
	/* Complete potential particle */
	Real f = 1.0;
	if (insideCount == planeNo){ f = -1.0;}
	return f;
}


/* ***************************************************************************************************************************** */
Vector3r Ig2_PB_PB_ScGeom::getNormal(const shared_ptr<Shape>& cm1, const State& state1, const Vector3r& shift2, const Vector3r newTrial, const bool twoD){

	PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	///////////////////Transforming trial values to local frame of particles//////////////////
	Vector3r tempP1 = newTrial - state1.pos - shift2;

	/* Direction cosines */
	Vector3r localP1(0,0,0);
	localP1 = state1.ori.conjugate()*tempP1; //the body has been rotated by state.ori.  So I will need to rotate the frame by state.ori too
	Real x = localP1[0];
	Real y = localP1[1];
	Real z = localP1[2];

////////////////////////////// assembling potential planes particle 1//////////////////////////////
	int planeNo = s1->a.size();
	vector<Real>p; Real pSum2 = 0.0, plane; bool isSphere = true; int minNo = 0; Real closestDistance = pow(10,12);
	for (int i=0; i<planeNo; i++){
		plane = s1->a[i]*x + s1->b[i]*y + s1->c[i]*z -s1-> d[i]; //Distance aX-d: innerPP
		if (plane<pow(10,-15)){ //if trial point inside the inner PP
			if (fabs(plane)<closestDistance){ //find the plane closest to the contactPoint
				if (twoD){ //2-D
					Vector3r normal (s1->a[i],s1->b[i],s1->c[i]);
					Real dist = normal.dot(Vector3r(0,1,0)); //FIXME: Instead of the hardcoded Vector3r(0,1,0), here we should use "twoDir"
					if(fabs(dist)<0.99 ){minNo = i; closestDistance = fabs(plane); }
				}else{ //3-D
					minNo = i; closestDistance = fabs(plane);
				}
			}
			plane = 0.0;

		}else{isSphere = false;} //if the contactPoint is not fully inside the inner PP, isSphere=false and the contact normal is calculated as the normal of the plane closest to the contactPoint; to be revised @vsangelidakis
		p.push_back(plane); //FIXME: Instead of saving the value of "plane" inside the vector "p" and iterate through "p[i]" in a different loop below, we can calculate pdxSum, pdySum, pdzSum directly here, and avoid storing all this information in the vector "p". In this way, we don't need "p"
		pSum2 += pow(p[i],2);
	}
	//Real r = s1->r; Real R = s1->R; Real k = s1->k;
	/* Additional sphere */
	//Real sphere  = (pow(x,2) + pow(y,2) + pow(z,2))/pow(R,2);
	/* Complete potential particle */
	//Real f = (1.0-k)*(pSum2/pow(r,2)-1.0)+k*(sphere -1.0);

	//////////////////////////////// local first derivitive particle 1 /////////////////////////////////////
	Real pdxSum = 0.0, pdySum = 0.0, pdzSum = 0.0;
	for (int i=0; i<planeNo; i++){
		pdxSum += (s1->a[i]*p[i]);
		pdySum += (s1->b[i]*p[i]);
		pdzSum += (s1->c[i]*p[i]);
	}

	Real fdx = 2.0 * pdxSum;
	Real fdy = 2.0 * pdySum;
	Real fdz = 2.0 * pdzSum;
	if (isSphere){
		 fdx =  s1->a[minNo]; //2.0*x; //
		 fdy =  s1->b[minNo]; //2.0*y; //
		 fdz =  s1->c[minNo]; //2.0*z; //
	}
	///////////////////// Assembling rotation matrix Qt for particle 1 //////////////////////////////
	Matrix3r Q1=Eigen::Matrix3d::Zero();
	//if(state1.ori==Quaternionr(1,0,0,0)){
	//	Q1.setIdentity();
	//}else{
		Real q0 = state1.ori.w();
		Real q1 = state1.ori.x();
		Real q2 = state1.ori.y();
		Real q3 = state1.ori.z();
		Q1(0,0) = 2.0*pow(q0,2.0) -1.0 + 2.0*pow(q1,2.0);
		Q1(0,1) = 2.0*q1*q2 + 2.0*q0*q3;
		Q1(0,2) = 2.0*q1*q3 - 2.0*q0*q2;
		Q1(1,0) = 2.0*q1*q2 - 2.0*q0*q3;
		Q1(1,1) = 2.0*pow(q0,2.0) -1.0 + 2.0*pow(q2,2.0);
		Q1(1,2) = 2.0*q2*q3 + 2.0*q0*q1;
		Q1(2,0) = 2.0*q1*q3 + 2.0*q0*q2;
		Q1(2,1) = 2.0*q2*q3 - 2.0*q0*q1;
		Q1(2,2) = 2.0*pow(q0,2) -1.0 + 2.0*pow(q3,2.0);

		//Matrix3r Q1n = state1.ori.toRotationMatrix();
		//Matrix3r Q1c = state1.ori.conjugate().toRotationMatrix();
		//std::cout<<"Q1: "<<endl<<Q1<<endl<<"Q1n: "<<endl<<Q1n<<endl<<"Q1c: "<<endl<<Q1c<<endl<<endl;
	//}

	///////////////////// global first and second derivitive for particle 1 /////////////////////////
	Real Fdx = fdx * Q1(0,0) + fdy*Q1(1,0) + fdz*Q1(2,0);
	Real Fdy = fdx * Q1(0,1) + fdy*Q1(1,1) + fdz*Q1(2,1);
	Real Fdz = fdx * Q1(0,2) + fdy*Q1(1,2) + fdz*Q1(2,2);

	return Vector3r(Fdx,Fdy,Fdz);
}


/* ***************************************************************************************************************************** */
void Ig2_PB_PB_ScGeom::getPtOnParticle2(const shared_ptr<Shape>& cm1, const State& state1, const Vector3r& shift2, Vector3r midPoint, Vector3r searchDir, Vector3r& ptOnParticle){
	//PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
	ptOnParticle = midPoint;
	Real f = evaluatePB(cm1, state1, shift2, ptOnParticle);//evaluateFNoSphere(cm1,state1, ptOnParticle); //
	Real fprevious = f;
	int counter = 0;
	//normal.normalize();
	Vector3r step = searchDir*Mathr::Sign(f) *-1.0;
	Vector3r bracketA(0,0,0), bracketB(0,0,0);

	do{
		ptOnParticle += step;
		fprevious = f;
		f = evaluatePB(cm1, state1, shift2, ptOnParticle); //evaluateFNoSphere(cm1,state1, ptOnParticle); //
		counter++;
		if (counter == 50000 ){
			//LOG_WARN("Initial point searching exceeded 500 iterations!");
			std::cout<<"ptonparticle2 search exceeded 50000 iterations! step:"<<step<<endl;
		}

	}while(Mathr::Sign(fprevious)*Mathr::Sign(f)*1.0> 0.0 );
	bracketA = ptOnParticle;
	bracketB = ptOnParticle - step;
	Vector3r zero(0,0,0);
	BrentZeroSurf(cm1, state1, shift2, bracketA, bracketB, zero);
	ptOnParticle = zero;
	//if( fabs(f)>0.1){std::cout<<"getInitial point f:"<<f<<endl;}
}


/* ***************************************************************************************************************************** */
bool Ig2_PB_PB_ScGeom::customSolve(const shared_ptr<Shape>& cm1, const State& state1, const shared_ptr<Shape>& cm2, const State& state2, const Vector3r& shift2, Vector3r &contactPt, bool warmstart){
  //timingDeltas->start();
  PotentialBlock *s1=static_cast<PotentialBlock*>(cm1.get());
  PotentialBlock *s2=static_cast<PotentialBlock*>(cm2.get());
  Vector3r xGlobal(0.0,0.0,0.0);
  int iter = 0;
  int totalIter = 0;

 /* Parameters for particles A and B */
  Real rA = s1->r;Real kA = s1->k;Real RA = s1->R;
  Real rB = s2->r;Real kB = s2->k;Real RB = s2->R;
  int planeNoA = s1->a.size();int planeNoB = s2->a.size();
  int varNo = 3+1+planeNoA+planeNoB;  int planeNoAB = planeNoA + planeNoB; int varNo2 = varNo*varNo;
  int planeNoA3 = 3+planeNoA; int planeNoB3=3+planeNoB; //int planeNoA2 =planeNoA*planeNoA; int planeNoB2=planeNoB*planeNoB;
  Matrix3r QA = state1.ori.conjugate().toRotationMatrix(); /*direction cosine */

  Matrix3r QB = state2.ori.conjugate().toRotationMatrix(); /*direction cosine */

  int blas3 = 3; char blasNT = 'N'; char blasT= 'T'; int blas1planeNoA = std::max(1,planeNoA); int blas1planeNoB = std::max(1,planeNoB); int blas1planeNoAB = std::max(1,planeNoAB); Real blas0 = 0.0; Real blas1 = 1.0; Real blasNeg1 = -1.0;

  Real blasQA[9];   Real blasQB[9];
  Real blasPosA[3]; Real blasPosB[3];
  Real blasContactPt[3];
  Real blasC[varNo];
  for (int i=0; i<varNo; i++){
	blasC[i] = 0.0;
  }

  blasC[3]=  1.0;
  int blasCount = 0;

  contactPt+=shift2;

  for(int i=0; i<3; i++){
	for(int j=0; j<3; j++){
		blasQA[blasCount]=QA(j,i);
		blasQB[blasCount]=QB(j,i);
		blasCount++;
	}
	blasPosA[i]=state1.pos[i];
	blasPosB[i]=state2.pos[i]+shift2[i];
	blasContactPt[i]=contactPt[i];
  }
//std::cout<<"QA: "<<QA<<endl;std::cout<<"blasQA: "<<endl;

//for (int i=0; i<9; i++){
//	std::cout<<blasQA[i]<<endl;
//}

 /* Variables to keep things neat */
  Real kAs = sqrt(kA)/RA;
  Real kAp = sqrt(1.0 - kA)/rA;
  Real kBs = sqrt(kB)/RB;
  Real kBp = sqrt(1.0 - kB)/rB;

  /* penalty */
  Real t = 1.0;  Real mu=10.0;  Real planePert = 0.1*rA;  Real sPert = 1.0; //+ 10.0*planePert*(planeNoA+planeNoB)/(rA*rA);
  if (warmstart){
	t = 0.01; mu= 50.0; planePert = 0.01; sPert = 0.01; ///* pow(10,-1)+ */ sqrt(planePert*planePert*(planeNoA+planeNoB)/(rA*rA));
  }
  /* s */
  Real s = 0.0;  Real m=2.0;  Real NTTOL = pow(10,-8);  Real tol = accuracyTol/*pow(10,-4)* RA*pow(10,-6)*/; Real gap =0.0;
  /* x */
  Real blasX[varNo]; Real blasNewX[varNo];
  blasX[0] = contactPt[0]; blasX[1] = contactPt[1]; blasX[2] = contactPt[2];

  //////////////////// evaluate fA and fB to initialize ////////////////////////////////
  /* fA */
  Real blasTemPB1[3]; Real blasLocalP1[3];
  for(int i=0; i<3; i++){
	blasTemPB1[i] = blasContactPt[i] - blasPosA[i];
  }
  char transA = 'N';  /* char transB = 'N'; int  blasM = 3; int  blasN = 3; */ int  blasK = 3;
  /* int blasLDA = 3; */ int  blasLDB = 3;    Real blasAlpha = 1.0;  Real blasBeta = 0.0;
  /* int blasLDC = 3; */ int incx=1; int incy=1;
//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasQA[0], &blasLDA, &blasTemPB1[0], &incx, &blasBeta, &blasLocalP1[0], &incy);
  dgemv_(&blasNT, &blas3, &blas3, &blas1, &blasQA[0], &blas3, &blasTemPB1[0], &incx, &blas0, &blasLocalP1[0], &incy);

  //Vector3r temPB1 = contactPt - posA;
  //Vector3r localP1 = QA*temPB1;
  /* P1Q */ //Eigen::MatrixXd P1 = Eigen::MatrixXd::Zero(planeNoA,3);
  /*d1*/    //Eigen::MatrixXd d1 = Eigen::MatrixXd::Zero(planeNoA,1);

  Real blasP1[planeNoA*3];
  Real blasD1[planeNoA];
  Real blasP1Q[planeNoA*3];
  Real pertSumA2 = 0.0;

  for (int i=0; i<planeNoA; i++){
	Real plane = s1->a[i]*blasLocalP1[0] + s1->b[i]*blasLocalP1[1] + s1->c[i]*blasLocalP1[2] - s1->d[i];
	if (plane<pow(10,-15)){
		plane = 0.0;
		blasX[4+i] = plane + planePert;
	}else{
		blasX[4+i] = plane + planePert;
	}
	pertSumA2 += pow((plane+planePert),2);
	blasP1[i] = s1->a[i];  blasP1[i+planeNoA] = s1->b[i];  blasP1[i+2*planeNoA] = s1->c[i];
	blasD1[i] = s1->d[i];
  }
  //Eigen::MatrixXd P1Q = P1*QA;
  //transA = 'N'; transB = 'N'; blasM = planeNoA;  blasN = 3; blasK = 3;
 // blasLDA = std::max(1,blasM);  blasLDC = std::max(1,blasM); blasLDB = 3; blasAlpha=1.0; blasBeta = 0.0;
  //dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasP1[0], &blasLDA, &blasQA[0], &blasLDB, &blasBeta, &blasP1Q[0], &blasLDC);
  dgemm_(&blasNT, &blasNT, &planeNoA, &blas3, &blas3, &blas1, &blasP1[0], &blas1planeNoA, &blasQA[0], &blasLDB, &blas0, &blasP1Q[0], &blas1planeNoA);

  Real sphereA  = (pow(blasLocalP1[0],2) + pow(blasLocalP1[1],2) + pow(blasLocalP1[2],2))/pow(RA,2);
  Real sA = (1.0-kA)*(pertSumA2/pow(rA,2) - 1.0)+kA*(sphereA -1.0);

  /* fB */
  Real blasTemPB2[3]; Real blasLocalP2[3];
  for(int i=0; i<3; i++){
	blasTemPB2[i] = blasContactPt[i] - blasPosB[i];
  }
  //transA = 'N'; blasM = 3;  blasN = 3;  blasLDA = 3;   blasAlpha=1.0; blasBeta = 0.0;
  //dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasQB[0], &blasLDA, &blasTemPB2[0], &incx, &blasBeta, &blasLocalP2[0], &incy);
  dgemv_(&blasNT, &blas3, &blas3, &blas1, &blasQB[0], &blas3, &blasTemPB2[0], &incx, &blas0, &blasLocalP2[0], &incy);

  // Vector3r temPB2 = contactPt - posB;
  // Vector3r localP2 = QB*temPB2;
  /*P2Q*/ //Eigen::MatrixXd P2 = Eigen::MatrixXd::Zero(planeNoB,3);
  /*d2*/  //Eigen::MatrixXd d2 = Eigen::MatrixXd::Zero(planeNoB,1);
  Real blasP2[planeNoB*3];
  Real blasD2[planeNoB];
  Real blasP2Q[planeNoB*3]; Real pertSumB2 = 0.0;
  for (int i=0; i<planeNoB; i++){
	Real plane = s2->a[i]*blasLocalP2[0] + s2->b[i]*blasLocalP2[1] + s2->c[i]*blasLocalP2[2] - s2->d[i];
	if (plane<pow(10,-15)){
		plane = 0.0;
		blasX[4+planeNoA+i] = plane+ planePert;
	}else{
		blasX[4+planeNoA+i] = plane + planePert;
	}
	pertSumB2 += pow((plane+planePert),2);
	blasP2[i] = s2->a[i];  blasP2[i+planeNoB] = s2->b[i];  blasP2[i+2*planeNoB] = s2->c[i];
	blasD2[i] = s2->d[i];
  }
 //Eigen::MatrixXd P2Q = P2*QB;
  //transA = 'N'; transB = 'N'; blasM = planeNoB;    blasN = 3; blasK = 3;
  //blasLDA = std::max(1,blasM);   blasLDC = std::max(1,blasM); blasLDB = 3; blasAlpha=1.0; blasBeta = 0.0;
 // dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasP2[0], &blasLDA, &blasQB[0], &blasLDB, &blasBeta, &blasP2Q[0], &blasLDC);
  dgemm_(&blasNT, &blasNT, &planeNoB, &blas3, &blas3, &blasAlpha, &blasP2[0], &blas1planeNoB, &blasQB[0], &blas3, &blasBeta, &blasP2Q[0], &blas1planeNoB);
  Real sphereB  = (pow(blasLocalP2[0],2) + pow(blasLocalP2[1],2) + pow(blasLocalP2[2],2))/pow(RB,2);
  Real sB = (1.0-kB)*(pertSumB2/pow(rB,2) - 1.0)+kB*(sphereB -1.0);
  //sPert = fabs(fA-fB);

  s = std::max(sqrt(fabs(sA+1.0)), sqrt(fabs(sB+1.0))) + sPert;  //sqrt(fA+fB+2.0)+sPert; //sqrt(std::max(fabs(fA+1.0), fabs(fB+1.0))) + sPert; //
  //x[3] = s;
  blasX[3]= s;

  ///////////////////  algebra formulation of the SOCP ///////////////////
  /* c */
 // Eigen::MatrixXd c=Eigen::MatrixXd::Zero(varNo,1);
 // c[3] = 1.0;

  Real blasA1[(3+planeNoA)*varNo];  Real blasA2[(3+planeNoB)*varNo];
  /* Second order cone constraints */
  /* A1 */
  //Eigen::MatrixXd A1(3+planeNoA,varNo);
  //Matrix3r QAs=kAs*QA; //cwise()
  Real blasQAs[9]; int noElements=9; Real scaleFactor = kAs;
  dcopy_(&noElements, &blasQA[0], &incx, &blasQAs[0], &incx);
  dscal_(&noElements, &scaleFactor, &blasQAs[0], &incx);

  // A1 << QAs,Eigen::MatrixXd::Zero(3,1+planeNoAB),
  // Eigen::MatrixXd::Zero(planeNoA,4),kAp*Eigen::MatrixXd::Identity(planeNoA, planeNoA),Eigen::MatrixXd::Zero(planeNoA,planeNoB);
  memset(blasA1,0.0,sizeof(blasA1));
  for (int i=0; i<3; i++){
	blasA1[i] = blasQAs[i];  blasA1[i+planeNoA3] = blasQAs[i+3];  blasA1[i+2*planeNoA3] = blasQAs[i+6];
  }
  for (int i=0; i<planeNoA; i++){
	blasA1[3+i+(planeNoA3)*(4+i)] = kAp;
  }


  /* A2 */
  //Eigen::MatrixXd A2(3+planeNoB,varNo);
  //Matrix3r QBs=kBs*QB; //cwise();
  Real blasQBs[9];  noElements=9;  scaleFactor = kBs;
  dcopy_(&noElements, &blasQB[0], &incx, &blasQBs[0], &incx);
  dscal_(&noElements, &scaleFactor, &blasQBs[0], &incx);

  // A2 << QBs,Eigen::MatrixXd::Zero(3,1+planeNoAB),
  // Eigen::MatrixXd::Zero(planeNoB,4),Eigen::MatrixXd::Zero(planeNoB,planeNoA),kBp*Eigen::MatrixXd::Identity(planeNoB, planeNoB);
  memset(blasA2,0.0,sizeof(blasA2));
  for (int i=0; i<3; i++){
	blasA2[i] = blasQBs[i];  blasA2[i+planeNoB3] = blasQBs[i+3];  blasA2[i+2*(planeNoB3)] = blasQBs[i+6];
  }

  for (int i=0; i<planeNoB; i++){
	blasA2[3+i+(planeNoB3)*(4+planeNoA+i)] = kBp;
  }



  /*b1*/
#if 0
  Eigen::MatrixXd b1=Eigen::MatrixXd::Zero(3+planeNoA,1);
  Eigen::Vector3d b1temp = QAs*posA;
  b1[0] = -b1temp[0];		b1[1]= -b1temp[1];	b1[2] = -b1temp[2];
#endif

  Real blasB1[planeNoA3]; Real blasB1temp[3];
  memset(blasB1,0.0,sizeof(blasB1));
 // blasM = 3;   blasN=3;
//  blasLDA = 3;    blasAlpha = -1.0;  blasBeta=0.0;  blasLDC = 3;
// dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasQAs[0], &blasLDA, &blasPosA[0], &incx, &blasBeta, &blasB1temp[0], &incy);
  dgemv_(&blasNT, &blas3, &blas3, &blasNeg1, &blasQAs[0], &blas3, &blasPosA[0], &incx, &blas0, &blasB1temp[0], &incy);
  blasB1[0]=blasB1temp[0]; blasB1[1]=blasB1temp[1]; blasB1[2]=blasB1temp[2];

  /*b2*/
#if 0
  Eigen::MatrixXd b2=Eigen::MatrixXd::Zero(3+planeNoB,1);
  Eigen::Vector3d b2temp = QBs*posB;
  b2[0] = -b2temp[0];		b2[1]= -b2temp[1];	b2[2] = -b2temp[2];
#endif

  Real blasB2[planeNoB3]; Real blasB2temp[3];
  memset(blasB2,0.0,sizeof(blasB2));
 //  blasM = 3;   blasN=3; blasLDA = 3; blasAlpha=-1.0; blasBeta=0.0;
 // dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasQBs[0], &blasLDA, &blasPosB[0], &incx, &blasBeta, &blasB2temp[0], &incy);
  dgemv_(&blasNT, &blas3, &blas3, &blasNeg1, &blasQBs[0], &blas3, &blasPosB[0], &incx, &blas0, &blasB2temp[0], &incy);
  blasB2[0]=blasB2temp[0]; blasB2[1]=blasB2temp[1]; blasB2[2]=blasB2temp[2];

  /*AL*/
 // Eigen::MatrixXd AL(planeNoAB,varNo);
 // AL<<P1Q, Eigen::MatrixXd::Zero(planeNoA,1), -1.0*Eigen::MatrixXd::Identity(planeNoA,planeNoA), Eigen::MatrixXd::Zero(planeNoA,planeNoB), //cwise()
//	      P2Q, Eigen::MatrixXd::Zero(planeNoB,1), Eigen::MatrixXd::Zero(planeNoB,planeNoA), -1.0*Eigen::MatrixXd::Identity(planeNoB,planeNoB);

  Real blasAL[planeNoAB*varNo];
  memset(blasAL,0.0,sizeof(blasAL));
  for (int i=0; i<planeNoA; i++){
	blasAL[i] = blasP1Q[i];  blasAL[i+planeNoAB] = blasP1Q[i+planeNoA];  blasAL[i+2*planeNoAB] = blasP1Q[i+2*planeNoA];
	blasAL[i+(4+i)*planeNoAB] = -1.0;
  }
  for (int i=0; i<planeNoB; i++){
	blasAL[planeNoA+i] = blasP2Q[i];  blasAL[planeNoA+i+planeNoAB] = blasP2Q[i+planeNoB];  blasAL[planeNoA+i+2*planeNoAB] = blasP2Q[i+2*planeNoB];
	blasAL[planeNoA+i+(4+planeNoA+i)*planeNoAB] = -1.0;
  }



  /*bL*/
#if 0
  Eigen::MatrixXd bL(planeNoAB,1);
  Eigen::MatrixXd pos1(3,1); pos1(0,0) = posA[0]; pos1(1,0) = posA[1]; pos1(2,0)=posA[2];
  Eigen::MatrixXd pos2(3,1); pos2(0,0) = posB[0]; pos2(1,0) = posB[1]; pos2(2,0)=posB[2];
  Eigen::MatrixXd btempU = P1Q*pos1 + d1;
  Eigen::MatrixXd btempL = P2Q*pos2 + d2;
  bL<<btempU,btempL;
#endif

  Real blasBL[planeNoAB];  Real blasBTempU[planeNoA]; Real blasBTempL[planeNoB];
   noElements = planeNoA;
  dcopy_(&noElements, &blasD1[0], &incx, &blasBTempU[0], &incx);
  //transA = 'N';	blasM = planeNoA;   blasN = 3;
  //blasLDA = std::max(1,planeNoA);      blasAlpha = 1.0;   blasBeta = 1.0;
  //dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasP1Q[0], &blasLDA,&blasPosA[0], &incx, &blasBeta, &blasBTempU[0], &incy);
  dgemv_(&blasNT, &planeNoA, &blas3, &blas1, &blasP1Q[0], &blas1planeNoA,&blasPosA[0], &incx, &blas1, &blasBTempU[0], &incy);
  noElements = planeNoB;
  dcopy_(&noElements, &blasD2[0], &incx, &blasBTempL[0], &incx);
  //transA = 'N';	blasM = planeNoB;   blasN = 3;
  //blasLDA = std::max(1,planeNoB);      blasAlpha = 1.0;   blasBeta = 1.0;
  //dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasP2Q[0], &blasLDA,&blasPosB[0], &incx, &blasBeta, &blasBTempL[0], &incy);
  dgemv_(&blasNT, &planeNoB, &blas3, &blas1, &blasP2Q[0], &blas1planeNoB,&blasPosB[0], &incx, &blas1, &blasBTempL[0], &incy);
  for (int i=0; i<planeNoA; i++){
	blasBL[i]=blasBTempU[i];
  }
  for (int i=0; i<planeNoB; i++){
	blasBL[planeNoA+i]=blasBTempL[i];
  }

  Real u1; Real u2;
  Real blasCCtranspose[varNo2];
  memset(blasCCtranspose,0.0,sizeof(blasCCtranspose));
  blasCCtranspose[3+varNo*3]=1.0;

   Real blasCa1[varNo2];
//#if 0
	#if 0
	  Eigen::MatrixXd ca1Transpose = ccTranspose - A1.transpose()*A1;
	  Eigen::MatrixXd ca2Transpose = ccTranspose - A2.transpose()*A2;
	#endif
	/* ca1Transpose */
	  noElements = varNo2;  incx =1;  incy=1;
	  dcopy_(&noElements, &blasCCtranspose[0], &incx, &blasCa1[0], &incy);

	  //transA = 'T';   transB = 'N';   blasM = varNo;   blasN = varNo;   blasK = 3+planeNoA;
	   //blasLDA = blasK;   blasLDB = blasK;     blasAlpha = -1.0;   blasBeta = 1.0;
	  //blasLDC = varNo;
	  //dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasA1[0], &blasLDA, &blasA1[0], &blasLDB, &blasBeta, &blasCa1[0], &blasLDC);
	  dgemm_(&blasT, &blasNT, &varNo, &varNo, &planeNoA3, &blasNeg1, &blasA1[0], &planeNoA3, &blasA1[0], &planeNoA3, &blas1, &blasCa1[0], &varNo);


	/* ca2Transpose */
	  Real blasCa2[varNo2]; blasCount = 0;
	  dcopy_(&noElements, &blasCCtranspose[0], &incx, &blasCa2[0], &incy);

	 //transA = 'T';   transB = 'N';   blasM = varNo;   blasN = varNo;  blasK = 3+planeNoB;
	 // blasLDA = blasK;   blasLDB = blasK;     blasAlpha = -1.0;   blasBeta = 1.0;
	 // blasLDC = varNo;
	 // dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasA2[0], &blasLDA, &blasA2[0], &blasLDB, &blasBeta, &blasCa2[0], &blasLDC);
	  dgemm_(&blasT, &blasNT, &varNo, &varNo, &planeNoB3, &blasNeg1, &blasA2[0], &planeNoB3, &blasA2[0], &planeNoB3, &blas1, &blasCa2[0], &varNo);

/* DL */
Real blasDL[planeNoAB*planeNoAB];  blasCount = 0;
memset(blasDL,0.0,sizeof(blasDL));

//#endif


  Real wLlogsum = 0.0;
  Real val = 0.0;
  Real newval = 0.0;
  /*Linesearch */
  Real backtrack = 1.0;
  Real penalty = 1.0/t;

/* LAPACK */
  int info; char UPLO ='L'; int KD = varNo-1; int nrhs=1;
  //Real gradient[varNo];
  Real HessianChol[varNo][varNo];


 Real blasGA[varNo]; Real blasGB[varNo]; Real blasGL[varNo];

 Real blasW1[3+planeNoA]; Real blasW2[3+planeNoB]; Real blasWL[planeNoAB]; Real blasVL[planeNoAB]; Real minWL=0.0;
 Real blasHA[varNo*varNo]; Real blasHB[varNo*varNo];  Real blasHL[varNo*varNo];  Real blasADAtemp[varNo*planeNoAB];
 noElements = varNo;
 Real blasW1dot=0.0; Real blasW2dot=0.0;
 Real blasGrad[varNo]; Real blasHess[varNo*varNo]; Real blasStep[varNo];  Real blasFprime = 0.0;

#if 0
//	MAKE SURE POINTS ARE FEASIBLE //
 /* temp variables */
  /* w1 = A1*x + b1; */
	noElements = 3+planeNoA;
  	dcopy_(&noElements, &blasB1[0], &incx, &blasW1[0], &incy);
	//transA = 'N';	blasM = 3+planeNoA;   blasN = varNo;
	//blasLDA = 3+planeNoA;      blasAlpha = 1.0;   blasBeta = 1.0;    incx =1;  incy=1;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA1[0], &blasLDA, &blasX[0], &incx, &blasBeta, &blasW1[0], &incy);
	dgemv_(&blasNT, &planeNoA3, &varNo, &blas1, &blasA1[0], &planeNoA3, &blasX[0], &incx, &blas1, &blasW1[0], &incy);

  /* w2 = A2*x + b2; */
	noElements = 3+planeNoB;
  	dcopy_(&noElements, &blasB2[0], &incx, &blasW2[0], &incy);
	//transA = 'N';	blasM = 3+planeNoB;   blasN = varNo;
	//blasLDA = 3+planeNoB;    blasAlpha = 1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA2[0], &blasLDA, &blasX[0], &incx, &blasBeta, &blasW2[0], &incy);
	dgemv_(&blasNT, &planeNoB3, &varNo, &blas1, &blasA2[0], &planeNoB3, &blasX[0], &incx, &blas1, &blasW2[0], &incy);

  /* u1 = s*s - w1.dot(w1); */
  /* u2 = s*s - w2.dot(w2); */
  	//noElements = 3+planeNoA;
  	blasW1dot = ddot_(&planeNoA3, &blasW1[0], &incx, &blasW1[0], &incy);
  	//noElements = 3+planeNoB;
  	blasW2dot = ddot_(&planeNoB3, &blasW2[0], &incx, &blasW2[0], &incy);

s = std::max(sqrt(fabs(blasW1dot)),sqrt(fabs(blasW2dot)))+0.1;
blasX[3]=s;
#endif

//timingDeltas->checkpoint("setup");


while(totalIter<500){
  penalty = 1.0/t;
  /* Newton's method */
  /* s=x[3]; */
 	s=blasX[3];

  /* temp variables */
  /* w1 = A1*x + b1; */
	noElements = 3+planeNoA;
  	dcopy_(&noElements, &blasB1[0], &incx, &blasW1[0], &incy);
	//transA = 'N';	blasM = 3+planeNoA;   blasN = varNo;
	//blasLDA = 3+planeNoA;      blasAlpha = 1.0;   blasBeta = 1.0;    incx =1;  incy=1;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA1[0], &blasLDA, &blasX[0], &incx, &blasBeta, &blasW1[0], &incy);
	dgemv_(&blasNT, &planeNoA3, &varNo, &blas1, &blasA1[0], &planeNoA3, &blasX[0], &incx, &blas1, &blasW1[0], &incy);

  /* w2 = A2*x + b2; */
	noElements = 3+planeNoB;
  	dcopy_(&noElements, &blasB2[0], &incx, &blasW2[0], &incy);
	//transA = 'N';	blasM = 3+planeNoB;   blasN = varNo;
	//blasLDA = 3+planeNoB;    blasAlpha = 1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA2[0], &blasLDA, &blasX[0], &incx, &blasBeta, &blasW2[0], &incy);
	dgemv_(&blasNT, &planeNoB3, &varNo, &blas1, &blasA2[0], &planeNoB3, &blasX[0], &incx, &blas1, &blasW2[0], &incy);

  /* u1 = s*s - w1.dot(w1); */
  /* u2 = s*s - w2.dot(w2); */
  	//noElements = 3+planeNoA;
  	blasW1dot = ddot_(&planeNoA3, &blasW1[0], &incx, &blasW1[0], &incy);
  	//noElements = 3+planeNoB;
  	blasW2dot = ddot_(&planeNoB3, &blasW2[0], &incx, &blasW2[0], &incy);
  	u1 =s*s -blasW1dot;
  	u2 =s*s -blasW2dot;

  /* wL = bL - AL*x; */
	noElements = planeNoAB;
  	dcopy_(&noElements, &blasBL[0], &incx, &blasWL[0], &incy);
	//transA = 'N';	blasN = varNo;
	//blasM = planeNoAB;     blasLDA = std::max(1,planeNoAB);      blasAlpha = -1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasAL[0], &blasLDA, &blasX[0], &incx, &blasBeta, &blasWL[0], &incy);
	dgemv_(&blasNT, &planeNoAB, &varNo, &blasNeg1, &blasAL[0], &blas1planeNoAB, &blasX[0], &incx, &blas1, &blasWL[0], &incy);

	for (int i=0; i<planeNoAB; i++ ){
		blasVL[i] = 1.0/blasWL[i];
		blasDL[i*planeNoAB+i] = blasVL[i]*blasVL[i];
	}

  /* Gradient */
//gA = -2.0/u1*(s*c - A1.transpose()*w1); //-2.0/u1*(s*c - tempG); //
//gB = -2.0/u2*(s*c - A2.transpose()*w2); //-2.0/u2*(s*c - tempG); //
// gL = AL.transpose()*vL; //tempG; //
// g = (c + penalty*(gA + gB + gL) );


  /* gA */
  	//noElements = varNo;
  	dcopy_(&varNo, &blasC[0], &incx, &blasGA[0], &incy);
	//transA = 'T';      blasM = 3+planeNoA;   blasLDA = 3+planeNoA;
  	blasAlpha = -1.0*-2.0/u1;   blasBeta = 1.0*-2.0/u1*s;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA1[0], &blasLDA, &blasW1[0], &incx, &blasBeta, &blasGA[0], &incy);
	dgemv_(&blasT, &planeNoA3, &varNo, &blasAlpha, &blasA1[0], &planeNoA3, &blasW1[0], &incx, &blasBeta, &blasGA[0], &incy);

  /* gB */
  	dcopy_(&varNo, &blasC[0], &incx, &blasGB[0], &incy);
	//transA = 'T';  blasM = 3+planeNoB;   blasLDA = 3+planeNoB;
  	blasAlpha = -1.0*-2.0/u2;   blasBeta = 1.0*-2.0/u2*s;
  	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA2[0], &blasLDA, &blasW2[0], &incx, &blasBeta, &blasGB[0], &incy);
	dgemv_(&blasT, &planeNoB3, &varNo, &blasAlpha, &blasA2[0], &planeNoB3, &blasW2[0], &incx, &blasBeta, &blasGB[0], &incy);

/* gL */
  	//transA = 'T'; blasM = planeNoAB;   blasLDA = std::max(1,planeNoAB);      blasAlpha = 1.0;   blasBeta = 0.0;
  	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasAL[0], &blasLDA, &blasVL[0], &incx, &blasBeta, &blasGL[0], &incy);
	dgemv_(&blasT, &planeNoAB, &varNo, &blas1, &blasAL[0], &blas1planeNoAB, &blasVL[0], &incx, &blas0, &blasGL[0], &incy);


/* g */
  for (int i = 0; i<varNo; i++){
	blasGrad[i] = blasC[i] + penalty*(blasGA[i] + blasGB[i] + blasGL[i]);
  }

  /* Hessian */
	/* HA */ //Eigen::MatrixXd HA =  (-2.0/u1)*(ca1Transpose)+ gA*gA.transpose();
	//noElements = varNo2;
   	dcopy_(&varNo2, &blasCa1[0], &incx, &blasHA[0], &incy);
	//transA = 'N';   transB = 'T';   blasM = varNo;   blasN = varNo;   blasK = 1;blasLDA = blasM;   blasLDB = blasN;     blasAlpha = 1.0;  blasLDC = blasM;
        blasBeta = -2.0/(u1);  blasK = 1;
	//dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasGA[0], &blasLDA, &blasGA[0], &blasLDB, &blasBeta, &blasHA[0], &blasLDC);
	dgemm_(&blasNT, &blasT, &varNo, &varNo, &blasK, &blas1, &blasGA[0], &varNo, &blasGA[0], &varNo, &blasBeta, &blasHA[0], &varNo);

	/* HB */ // HB =  (-2.0/u2)*(ca2Transpose)+ gB*gB.transpose();
   	dcopy_(&varNo2, &blasCa2[0], &incx, &blasHB[0], &incy);
	//transA = 'N';   transB = 'T';   blasM = varNo;   blasN = varNo;   blasK = 1; blasLDA = blasM;   blasLDB = blasN;     blasAlpha = 1.0;      blasLDC = blasM;
	blasBeta = -2.0/(u2);
	//dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasGB[0], &blasLDA, &blasGB[0], &blasLDB, &blasBeta, &blasHB[0], &blasLDC);
	dgemm_(&blasNT, &blasT, &varNo, &varNo, &blasK, &blas1, &blasGB[0], &varNo, &blasGB[0], &varNo, &blasBeta, &blasHB[0], &varNo);


	/* HL */  //Eigen::MatrixXd HL1 = AL.transpose()*DL*AL;
	//transA = 'T';   transB = 'N';   blasM = varNo;   blasN = planeNoAB;   blasK = planeNoAB;
        //blasLDA = std::max(1,blasK);   blasLDB = std::max(1,blasK);     blasAlpha = 1.0;   blasBeta = 0.0; blasLDC = blasM;
	//dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasAL[0], &blasLDA, &blasDL[0], &blasLDB, &blasBeta, &blasADAtemp[0], &blasLDC);
	dgemm_(&blasT, &blasNT, &varNo, &planeNoAB, &planeNoAB, &blas1, &blasAL[0], &blas1planeNoAB, &blasDL[0], &blas1planeNoAB, &blas0, &blasADAtemp[0], &varNo);
    	//transA = 'N';   transB = 'N';   blasM = varNo;   blasN = varNo;   blasK = planeNoAB;
        //blasLDA = blasM;   blasLDB = std::max(1,blasK);     blasAlpha = 1.0;   blasBeta = 0.0;  blasLDC = blasM;
	//dgemm_(&transA, &transB, &blasM, &blasN, &blasK, &blasAlpha, &blasADAtemp[0], &blasLDA, &blasAL[0], &blasLDB, &blasBeta, &blasHL[0], &blasLDC);
	dgemm_(&blasNT, &blasNT, &varNo, &varNo, &planeNoAB, &blas1, &blasADAtemp[0], &varNo, &blasAL[0], &blas1planeNoAB, &blas0, &blasHL[0], &varNo);

	/* H */ // H = penalty*(HA + HB + HL);
  	for (int i = 0; i<varNo2; i++){
		blasHess[i] = penalty*(blasHA[i] + blasHB[i] + blasHL[i]);
  	}


//timingDeltas->checkpoint("assemble H and g");
//std::cout<<"before Chol, totalIter: "<<totalIter<<", s : "<<s<<", u1: "<<u1<<", u2: "<<u2<<", wLmincoeff: "<<minWL<<endl;

/* Cholesky factorization */
//#if 0
/* L */
	for( int j=1; j<=varNo ; j++){
		blasStep[j-1]=-blasGrad[j-1];
	    for (int i=j; i<=j+KD && i<=varNo; i++){
			HessianChol[j-1][1+i-j-1]    = blasHess[(i-1)+varNo*(j-1)];
	    }
	}
//#endif

#if 0
	/* U */
	for( int j=1; j<=varNo ; j++){
		blasStep[j-1]=-blasGrad[j-1];
	    	for (int i=std::max(1,j-KD); i<=j ; i++){
			HessianChol[j-1][KD+1+i-j-1]    = blasHess[(i-1)+varNo*(j-1)]; //H(i-1,j-1);
	    	}
	}
#endif
	dpbsv_( &UPLO, &varNo, &KD, &nrhs, &HessianChol[0][0], &varNo, blasStep, &varNo, &info );
	if(info != 0){
	  	std::cout<<"chol error"<<", planeA: "<<planeNoA<<", planeB: "<<planeNoB<<endl;
		//return false;
	 	/* LU factorization */
	  	for (int i=0; i<varNo; i++ ){
			blasStep[i]=-1.0*blasGrad[i]; //g[i];
	  	}
	 	int ipiv[varNo];  int bColNo=1;
	 	dgesv_( &varNo, &bColNo, blasHess, &varNo, ipiv, blasStep, &varNo, &info);
	}

	if (info!=0){
		std::cout<<"linear algebra error"<<endl;
	}

//timingDeltas->checkpoint("Cholesky");

  //fprime = step.transpose()*g;
  //noElements = varNo;
  blasFprime = ddot_(&varNo, &blasStep[0], &incx, &blasGrad[0], &incy);

  wLlogsum = 0.0;
  for (int i=0; i<planeNoAB; i++){
	//wLlogsum += log(wL[i]);
	wLlogsum += log(blasWL[i]);
  }

  val =  /* c.transpose()*/ blasX[3] -penalty*( log(u1) + log(u2) + wLlogsum );

  /*Linesearch */
  backtrack = 1.0;
  //noElements = varNo;
  dcopy_(&varNo, &blasX[0], &incx, &blasNewX[0], &incy);
  daxpy_(&varNo, &backtrack, &blasStep[0], &incx, &blasNewX[0], &incy);



  s = blasNewX[3];
 // w1 = A1*x + b1;

  	dcopy_(&planeNoA3, &blasB1[0], &incx, &blasW1[0], &incy);
	//transA = 'N';	blasAlpha = 1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA1[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasW1[0], &incy);
  	dgemv_(&blasNT, &planeNoA3, &varNo, &blas1, &blasA1[0], &planeNoA3, &blasNewX[0], &incx, &blas1, &blasW1[0], &incy);

 // w2 = A2*x + b2;
  	dcopy_(&planeNoB3, &blasB2[0], &incx, &blasW2[0], &incy);
	//transA = 'N';	blasAlpha = 1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &planeNoB3, &varNo, &blasAlpha, &blasA2[0], &planeNoB3, &blasNewX[0], &incx, &blasBeta, &blasW2[0], &incy);
	dgemv_(&blasNT, &planeNoB3, &varNo, &blas1, &blasA2[0], &planeNoB3, &blasNewX[0], &incx, &blas1, &blasW2[0], &incy);


  	blasW1dot = ddot_(&planeNoA3, &blasW1[0], &incx, &blasW1[0], &incy);
  	blasW2dot = ddot_(&planeNoB3, &blasW2[0], &incx, &blasW2[0], &incy);

  //u1 = s*s - w1.dot(w1);
  //u2 = s*s - w2.dot(w2);
  	u1 =s*s -blasW1dot;
  	u2 =s*s -blasW2dot;

  //wL = bL - AL*x;
  	dcopy_(&planeNoAB, &blasBL[0], &incx, &blasWL[0], &incy);
	//blasAlpha = -1.0;
	//blasM=planeNoAB; blasN=varNo; blasLDA=std::max(1,planeNoAB);
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasAL[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasWL[0], &incy);
	dgemv_(&transA, &planeNoAB, &varNo, &blasNeg1, &blasAL[0], &blas1planeNoAB, &blasNewX[0], &incx, &blas1, &blasWL[0], &incy);

  	minWL =0.00001;
  	if(planeNoAB>0){
  		minWL = blasWL[0];
	}
	for(int i=0; i<planeNoAB; i++){
		if(blasWL[i] < minWL){
			minWL = blasWL[i];
		}
	}

  int count = 0;
  while(s < 0.0 || u1<0.0 || u2<0.0 || minWL < 0.0){
	backtrack = 0.5*backtrack;
	dcopy_(&varNo, &blasX[0], &incx, &blasNewX[0], &incy);
	daxpy_(&varNo, &backtrack, &blasStep[0], &incx, &blasNewX[0], &incy);

	s = blasNewX[3];
	// w1 = A1*x + b1;

	dcopy_(&planeNoA3, &blasB1[0], &incx, &blasW1[0], &incy);
	//transA = 'N';	blasAlpha = 1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA1[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasW1[0], &incy);
	dgemv_(&blasNT, &planeNoA3, &varNo, &blas1, &blasA1[0], &planeNoA3, &blasNewX[0], &incx, &blas1, &blasW1[0], &incy);

 	// w2 = A2*x + b2;
  	dcopy_(&planeNoB3, &blasB2[0], &incx, &blasW2[0], &incy);
	//transA = 'N';	blasAlpha = 1.0;   blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA2[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasW2[0], &incy);
	dgemv_(&blasNT, &planeNoB3, &varNo, &blas1, &blasA2[0], &planeNoB3, &blasNewX[0], &incx, &blas1, &blasW2[0], &incy);
  	blasW1dot = ddot_(&planeNoA3, &blasW1[0], &incx, &blasW1[0], &incy);
  	blasW2dot = ddot_(&planeNoB3, &blasW2[0], &incx, &blasW2[0], &incy);

  	//u1 = s*s - w1.dot(w1);
 	//u2 = s*s - w2.dot(w2);
  	u1 =s*s -blasW1dot;
  	u2 =s*s -blasW2dot;

  	//wL = bL - AL*x;

  	dcopy_(&planeNoAB, &blasBL[0], &incx, &blasWL[0], &incy);
	// blasAlpha = -1.0;
	//blasM = planeNoAB;   blasLDA = std::max(1,planeNoAB);     blasN = varNo;    blasAlpha = -1.0;    blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasAL[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasWL[0], &incy);
	dgemv_(&blasNT, &planeNoAB, &varNo, &blasNeg1, &blasAL[0], &blas1planeNoAB, &blasNewX[0], &incx, &blas1, &blasWL[0], &incy);
 	if(planeNoAB>0){
  		minWL = blasWL[0];
	}
	for(int i=0; i<planeNoAB; i++){
		if(blasWL[i] < minWL){
			minWL = blasWL[i];
		}
	}
 	 count++;
	//std::cout<<"count: "<<count<<", s : "<<s<<", u1: "<<u1<<", u2: "<<u2<<", wLmincoeff: "<<minWL<<endl;
	 if(count==200){
		std::cout<<"count: "<<count<<", totalIter: "<<totalIter<<", backtrack: "<<backtrack<<"s : "<<s<<", u1: "<<u1<<", u2: "<<u2<<", wLmincoeff: "<<minWL<<endl;
		//std::cout<<"wL: "<<endl<<wL<<endl<<endl;
	 }
  }

//timingDeltas->checkpoint("barrier search");

  wLlogsum = 0.0;
  for (int i=0; i<planeNoAB; i++){
	wLlogsum += log(blasWL[i]);
  }


  newval = /*c.tranpose()*/blasNewX[3] -penalty*( log(u1) + log(u2) + wLlogsum );
  count = 0;
  while (newval > val + backtrack*0.01*blasFprime){
	backtrack = 0.5*backtrack;
 	//for (int i = 0; i<varNo; i++){
	//	blasNewX[i] = blasX[i] + backtrack*blasStep[i];
  	//}
	//noElements = varNo;
	dcopy_(&varNo, &blasX[0], &incx, &blasNewX[0], &incy);
	daxpy_(&varNo, &backtrack, &blasStep[0], &incx, &blasNewX[0], &incy);

	s = blasNewX[3];
 	// w1 = A1*x + b1;
	//noElements = 3+planeNoA;
  	dcopy_(&planeNoA3, &blasB1[0], &incx, &blasW1[0], &incy);
	//transA = 'N';	blasM = 3+planeNoA;   blasN = varNo;
	//blasLDA = 3+planeNoA;      blasAlpha = 1.0;   blasBeta = 1.0;    incx =1;  incy=1;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA1[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasW1[0], &incy);
	dgemv_(&blasNT, &planeNoA3, &varNo, &blas1, &blasA1[0], &planeNoA3, &blasNewX[0], &incx, &blas1, &blasW1[0], &incy);

 	// w2 = A2*x + b2;
	//noElements = 3+planeNoB;
  	dcopy_(&planeNoB3, &blasB2[0], &incx, &blasW2[0], &incy);
	//blasM = 3+planeNoB;   blasLDA = 3+planeNoB;     blasN = varNo;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasA2[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasW2[0], &incy);
	dgemv_(&blasNT, &planeNoB3, &varNo, &blas1, &blasA2[0], &planeNoB3, &blasNewX[0], &incx, &blas1, &blasW2[0], &incy);

 	//noElements = 3+planeNoA;
  	blasW1dot = ddot_(&planeNoA3, &blasW1[0], &incx, &blasW1[0], &incy);
  	//noElements = 3+planeNoB;
  	blasW2dot = ddot_(&planeNoB3, &blasW2[0], &incx, &blasW2[0], &incy);

  	//u1 = s*s - w1.dot(w1);
 	//u2 = s*s - w2.dot(w2);
  	u1 =s*s -blasW1dot;
  	u2 =s*s -blasW2dot;

  	//wL = bL - AL*x;
	//noElements = planeNoAB;
  	dcopy_(&planeNoAB, &blasBL[0], &incx, &blasWL[0], &incy);
	//blasM = planeNoAB;   blasLDA = std::max(1,planeNoAB);     blasN = varNo;    blasAlpha = -1.0;    blasBeta = 1.0;
	//dgemv_(&transA, &blasM, &blasN, &blasAlpha, &blasAL[0], &blasLDA, &blasNewX[0], &incx, &blasBeta, &blasWL[0], &incy);
	dgemv_(&blasNT, &planeNoAB, &varNo, &blasNeg1, &blasAL[0], &blas1planeNoAB, &blasNewX[0], &incx, &blas1, &blasWL[0], &incy);

 	count++;
	if(count==200){
		std::cout<<"count: "<<count<<", totalIter: "<<totalIter<<", backtrack: "<<backtrack<<"s : "<<s<<", u1: "<<u1<<", u2: "<<u2<<", wLmincoeff: "<<minWL<<endl;
		//std::cout<<"wL: "<<endl<<wL<<endl<<endl;
	}

	wLlogsum = 0.0;
	for (int i=0; i<planeNoAB; i++){
		wLlogsum += log(blasWL[i]);
	}

	newval = /* c.transpose()*/blasNewX[3] - penalty*( log(u1) + log(u2) + wLlogsum );

	count++;
	  if(backtrack<pow(10,-11) ){
		std::cout<<"iter: "<<iter<<", totalIter: "<<totalIter<<", backtrack: "<<backtrack<<", val: "<<val<<", newval: "<<newval<<", t: "<<t<<", fprime: "<<blasFprime<<endl;
		break;
	  }
  }
//timingDeltas->checkpoint("line search");

  noElements = varNo;
  dcopy_(&noElements, &blasNewX[0], &incx, &blasX[0], &incy);


  if(blasFprime >0.0){
	std::cout<<"count: "<<count<<", totalIter: "<<totalIter<<", blasFprime: "<<blasFprime<<endl;
  }


  if (-blasFprime*0.5 < NTTOL){
        gap = 2.0*m/t;
	//if (warmstart){break;}
        if (gap < tol){
	    contactPt[0] = blasX[0]; contactPt[1]=blasX[1]; contactPt[2]=blasX[2];
	    //if(warmstart){std::cout<<" totalIter : "<<totalIter<<endl;}
		Real fA = evaluatePB(cm1, state1, Vector3r(0,0,0), contactPt);
		Real fB = evaluatePB(cm2, state2, shift2         , contactPt);
		if(fabs(fA-fB)>0.001 ){std::cout<<"inside fA-fB: "<<fA-fB<<endl;}

//timingDeltas->checkpoint("newton");
            return true;
        }
        t = std::min(t*mu, (2.0*m+1.0)/tol);
        iter = 0;
        totalIter = totalIter+1;
  }
  if(totalIter>100){

	std::cout<<"totalIter: "<<totalIter<<", t: "<<t<<", gap: "<<2.0*m/t<<", blasFprime: "<<blasFprime<<endl;
	for (int i=0; i<varNo; i++){
			std::cout<<"blasStep, i"<<i<<", value: "<<blasStep[i]<<endl;
	}
	for (int i=0; i<varNo; i++){
			std::cout<<"blasGrad, i"<<i<<", value: "<<blasGrad[i]<<endl;
	}
	return false;
	//break;
  }
  iter++;
  totalIter++;

//timingDeltas->checkpoint("complete");
}


  return ( true );
}


} // namespace yade

#endif // YADE_POTENTIAL_BLOCKS


