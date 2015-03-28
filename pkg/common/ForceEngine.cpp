// 2004 © Janek Kozicki <cosurgi@berlios.de> 
// 2009 © Václav Šmilauer <eudoxos@arcig.cz> 
// 2014 © Raphael Maurin <raphael.maurin@irstea.fr> 

#include"ForceEngine.hpp"
#include<core/Scene.hpp>
#include<pkg/common/Sphere.hpp>
#include<lib/smoothing/LinearInterpolate.hpp>
#include<pkg/dem/Shop.hpp>

#include<core/IGeom.hpp>
#include<core/IPhys.hpp>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

YADE_PLUGIN((ForceEngine)(InterpolatingDirectedForceEngine)(RadialForceEngine)(DragEngine)(LinearDragEngine)(HydroForceEngine));

void ForceEngine::action(){
	FOREACH(Body::id_t id, ids){
		if (!(scene->bodies->exists(id))) continue;
		scene->forces.addForce(id,force);
	}
}

void InterpolatingDirectedForceEngine::action(){
	Real virtTime=wrap ? Shop::periodicWrap(scene->time,*times.begin(),*times.rbegin()) : scene->time;
	direction.normalize(); 
	force=linearInterpolate<Real,Real>(virtTime,times,magnitudes,_pos)*direction;
	ForceEngine::action();
}

void RadialForceEngine::postLoad(RadialForceEngine&){ axisDir.normalize(); }

void RadialForceEngine::action(){
	FOREACH(Body::id_t id, ids){
		if (!(scene->bodies->exists(id))) continue;
		const Vector3r& pos=Body::byId(id,scene)->state->pos;
		Vector3r radial=(pos - (axisPt+axisDir * /* t */ ((pos-axisPt).dot(axisDir)))).normalized();
		if(radial.squaredNorm()==0) continue;
		scene->forces.addForce(id,fNorm*radial);
	}
}

void DragEngine::action(){
	FOREACH(Body::id_t id, ids){
		Body* b=Body::byId(id,scene).get();
		if (!b) continue;
		if (!(scene->bodies->exists(id))) continue;
		const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
		if (sphere){
			Real A = sphere->radius*sphere->radius*Mathr::PI;	//Crossection of the sphere
			Vector3r velSphTemp = b->state->vel;
			Vector3r dragForce = Vector3r::Zero();
			
			if (velSphTemp != Vector3r::Zero()) {
				dragForce = -0.5*Rho*A*Cd*velSphTemp.squaredNorm()*velSphTemp.normalized();
			}
			scene->forces.addForce(id,dragForce);
		}
	}
}

void LinearDragEngine::action(){
	FOREACH(Body::id_t id, ids){
		Body* b=Body::byId(id,scene).get();
		if (!b) continue;
		if (!(scene->bodies->exists(id))) continue;
		const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
		if (sphere){
			Vector3r velSphTemp = b->state->vel;
			Vector3r dragForce = Vector3r::Zero();
			
			Real b = 6*Mathr::PI*nu*sphere->radius;
			
			if (velSphTemp != Vector3r::Zero()) {
				dragForce = -b*velSphTemp;
			}
			scene->forces.addForce(id,dragForce);
		}
	}
}

void HydroForceEngine::action(){
	/* Velocity fluctuation determination (not usually done at each dt, that is why it not placed in the other loop) */
	if (velFluct == true){
		/* check size */
		size_t size=vFluctX.size();
		if(size<scene->bodies->size()){
			size=scene->bodies->size();
			vFluctX.resize(size);
			vFluctZ.resize(size);
		}
		/* reset stored values to zero */
		memset(& vFluctX[0],0,size);
		memset(& vFluctZ[0],0,size);

		/* Create a random number generator rnd() with a gaussian distribution of mean 0 and stdev 1.0 */
		/* see http://www.boost.org/doc/libs/1_55_0/doc/html/boost_random/reference.html and the chapter 7 of Numerical Recipes in C, second edition (1992) for more details */
		static boost::minstd_rand0 randGen((int)TimingInfo::getNow(true));
		static boost::normal_distribution<Real> dist(0.0, 1.0);
		static boost::variate_generator<boost::minstd_rand0&,boost::normal_distribution<Real> > rnd(randGen,dist);

		double rand1 = 0.0;
		double rand2 = 0.0;
		/* Attribute a fluid velocity fluctuation to each body above the bed elevation */
		FOREACH(Body::id_t id, ids){
			Body* b=Body::byId(id,scene).get();
			if (!b) continue;
			if (!(scene->bodies->exists(id))) continue;
			const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
			if (sphere){
				Vector3r posSphere = b->state->pos;//position vector of the sphere
				int p = floor((posSphere[2]-zRef)/deltaZ); //cell number in which the particle is
				// if the particle is inside the water and above the bed elevation, so inside the turbulent flow, evaluate a turbulent fluid velocity fluctuation which will be used to apply the drag.
				if ((p<nCell)&&(posSphere[2]-zRef>bedElevation)) { 
					Real uStar2 = simplifiedReynoldStresses[p];
					if (uStar2>0.0){
						Real uStar = sqrt(uStar2);
						rand1 = rnd();
						rand2 = -rand1 + rnd();
						vFluctZ[id] = rand1*uStar;
						vFluctX[id] = rand2*uStar;
					}
				}
			}
			
		}
		velFluct = false;
	}
	
	/* Application of hydrodynamical forces */
	if (activateAverage==true) averageProfile(); //Calculate the average fluid and velocity profile
	
	FOREACH(Body::id_t id, ids){
		Body* b=Body::byId(id,scene).get();
		if (!b) continue;
		if (!(scene->bodies->exists(id))) continue;
		const Sphere* sphere = dynamic_cast<Sphere*>(b->shape.get());
		if (sphere){
			Vector3r posSphere = b->state->pos;//position vector of the sphere
			int p = floor((posSphere[2]-zRef)/deltaZ); //cell number in which the particle is
			if ((p<nCell)&&(p>0)) {
				Vector3r liftForce = Vector3r::Zero();
				Vector3r dragForce = Vector3r::Zero();
				Vector3r vRel = Vector3r(vxFluid[p]+vFluctX[id],0.0,vFluctZ[id]) -  b->state->vel;//fluid-particle relative velocity
				//Drag force calculation
				if (vRel.norm()!=0.0) {
					dragForce = 0.5*densFluid*Mathr::PI*pow(sphere->radius,2.0)*(0.44*vRel.norm()+24.4*viscoDyn/(densFluid*sphere->radius*2))*pow(1-phiPart[p],-expoRZ)*vRel;
				}
				//lift force calculation due to difference of fluid pressure between top and bottom of the particle
				int intRadius = floor(sphere->radius/deltaZ);
				if ((p+intRadius<nCell)&&(p-intRadius>0)&&(lift==true)) {
					Real vRelTop = vxFluid[p+intRadius] -  b->state->vel[0]; // relative velocity of the fluid wrt the particle at the top of the particle
					Real vRelBottom = vxFluid[p-intRadius] -  b->state->vel[0]; // same at the bottom
					liftForce[2] = 0.5*densFluid*Mathr::PI*pow(sphere->radius,2.0)*Cl*(vRelTop*vRelTop-vRelBottom*vRelBottom);
				}
				//buoyant weight force calculation
				Vector3r buoyantForce = -4.0/3.0*Mathr::PI*pow(sphere->radius,3.0)*densFluid*gravity;
				//add the hydro forces to the particle
				scene->forces.addForce(id,dragForce+liftForce+buoyantForce);		
			}
		}
	}
}

void HydroForceEngine::averageProfile(){
	//Initialization
	int Np;
	int minZ;
	int maxZ;
	int numLayer;
	Real deltaCenter;
	Real zInf;
	Real zSup;
	Real volPart;
	Vector3r uRel = Vector3r::Zero();
	Vector3r fDrag  = Vector3r::Zero();

	int nMax = 2*nCell;
	vector<Real> velAverageX(nMax,0.0);
        vector<Real> velAverageY(nMax,0.0);
        vector<Real> velAverageZ(nMax,0.0);
	vector<Real> phiAverage(nMax,0.0);
	vector<Real> dragAverage(nMax,0.0);

	//Loop over the particles
	FOREACH(const shared_ptr<Body>& b, *Omega::instance().getScene()->bodies){
		shared_ptr<Sphere> s=YADE_PTR_DYN_CAST<Sphere>(b->shape); if(!s) continue;
		const Real zPos = b->state->pos[2]-zRef;
		int Np = floor(zPos/deltaZ);	//Define the layer number with 0 corresponding to zRef. Let the z position wrt to zero, that way all z altitude are positive. (otherwise problem with volPart evaluation)

		// Relative fluid/particle velocity using also the associated fluid vel. fluct. 
		if ((Np>=0)&&(Np<nCell)){
			uRel = Vector3r(vxFluid[Np]+vFluctX[b->id], 0.0,vFluctZ[b->id]) - b->state->vel;
			// Drag force with a Dallavalle formulation (drag coef.) and Richardson-Zaki Correction (hindrance effect)
			fDrag = 0.5*Mathr::PI*pow(s->radius,2.0)*densFluid*(0.44*uRel.norm()+24.4*viscoDyn/(densFluid*2.0*s->radius))*pow((1-phiPart[Np]),-expoRZ)*uRel;
		}
		else fDrag = Vector3r::Zero();

		minZ= floor((zPos-s->radius)/deltaZ);
		maxZ= floor((zPos+s->radius)/deltaZ);
		deltaCenter = zPos - Np*deltaZ;
	
		// Loop over the cell in which the particle is contained
		numLayer = minZ;
		while (numLayer<=maxZ){
			if ((numLayer>=0)&&(numLayer<nMax)){ //average under zRef does not interest us, avoid also negative values not compatible with the evaluation of volPart
				zInf=(numLayer-Np-1)*deltaZ + deltaCenter;
				zSup=(numLayer-Np)*deltaZ + deltaCenter;
				if (zInf<-s->radius) zInf = -s->radius;
				if (zSup>s->radius) zSup = s->radius;

				//Analytical formulation of the volume of a slice of sphere
				volPart = Mathr::PI*pow(s->radius,2)*(zSup - zInf +(pow(zInf,3)-pow(zSup,3))/(3*pow(s->radius,2)));

				phiAverage[numLayer]+=volPart;
				velAverageX[numLayer]+=volPart*b->state->vel[0];
                                velAverageY[numLayer]+=volPart*b->state->vel[1];
                                velAverageZ[numLayer]+=volPart*b->state->vel[2];
				dragAverage[numLayer]+=volPart*fDrag[0];
			}
			numLayer+=1;
		}
	}
	//Normalized the weighted velocity by the volume of particles contained inside the cell
	for(int n=0;n<nMax;n++){
		if (phiAverage[n]!=0){
			velAverageX[n]/=phiAverage[n];
                        velAverageY[n]/=phiAverage[n];
                        velAverageZ[n]/=phiAverage[n];
			dragAverage[n]/=phiAverage[n];
			//Normalize the concentration after
			phiAverage[n]/=vCell;
		}
		else {
			velAverageX[n] = 0.0;
                        velAverageY[n] = 0.0;
                        velAverageZ[n] = 0.0;
			dragAverage[n] = 0.0;
		}
	}
	//Assign the results to the global/public variables of HydroForceEngine
	phiPart = phiAverage;
	vxPart = velAverageX;
	vyPart = velAverageY;
        vzPart = velAverageZ;
	averageDrag = dragAverage;

	//desactivate the average to avoid calculating at each step, only when asked by the user
	activateAverage=false; 
}


