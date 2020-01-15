// 2009 © Sergei Dorofeenko <sega@users.berlios.de>
#include"ViscoelasticPM.hpp"
#include<core/State.hpp>
#include<pkg/dem/ScGeom.hpp>
#include<core/Omega.hpp>
#include<core/Scene.hpp>
#include<pkg/common/Sphere.hpp>

#ifdef YADE_SPH
#include<pkg/common/SPHEngine.hpp>
#endif

#ifdef YADE_DEFORM
#include<boost/math/tools/roots.hpp>
#endif

namespace yade { // Cannot have #include directive inside.

using std::isfinite;
YADE_PLUGIN((ViscElMat)(ViscElPhys)(Ip2_ViscElMat_ViscElMat_ViscElPhys)(Law2_ScGeom_ViscElPhys_Basic));

/* ViscElMat */
ViscElMat::~ViscElMat(){}

/* ViscElPhys */
ViscElPhys::~ViscElPhys(){}

Real Ip2_ViscElMat_ViscElMat_ViscElPhys::epsilon = 1.0e-8;

/* Ip2_ViscElMat_ViscElMat_ViscElPhys */
void Ip2_ViscElMat_ViscElMat_ViscElPhys::go(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction) {
	// no updates of an existing contact 
	if(interaction->phys) return;
	shared_ptr<ViscElPhys> phys (new ViscElPhys());
	Calculate_ViscElMat_ViscElMat_ViscElPhys(b1, b2, interaction, phys);

#ifdef YADE_DEFORM
	const ViscElMat* mat1 = static_cast<ViscElMat*>(b1.get());
	const ViscElMat* mat2 = static_cast<ViscElMat*>(b2.get());
	phys->DeformEnabled = mat1->DeformEnabled && mat2->DeformEnabled;
#endif
	interaction->phys = phys;
}

/* Law2_ScGeom_ViscElPhys_Basic */
bool Law2_ScGeom_ViscElPhys_Basic::go(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I) {
	Vector3r force = Vector3r::Zero();
	Vector3r torque1 = Vector3r::Zero();
	Vector3r torque2 = Vector3r::Zero();
	if (computeForceTorqueViscEl(_geom, _phys, I, force, torque1, torque2) and (I->isActive)) {
		const int id1 = I->getId1();
		const int id2 = I->getId2();
		
		addForce (id1,-force,scene);
		addForce (id2, force,scene);
		addTorque(id1, torque1,scene);
		addTorque(id2, torque2,scene);
		return true;
	} else return false;
}

bool computeForceTorqueViscEl(shared_ptr<IGeom>& _geom, shared_ptr<IPhys>& _phys, Interaction* I, Vector3r & force, Vector3r & torque1, Vector3r & torque2) {
	ViscElPhys& phys=*static_cast<ViscElPhys*>(_phys.get());
	const ScGeom& geom=*static_cast<ScGeom*>(_geom.get());
	Scene* scene=Omega::instance().getScene().get();

#ifdef YADE_SPH
//=======================================================================================================
	if (phys.SPHmode) {
		if (computeForceSPH(_geom, _phys, I, force)) {
			return true;
		} else {
			return false;
		}
	}
//=======================================================================================================
#endif

	const int id1 = I->getId1();
	const int id2 = I->getId2();

	Real addDR = 0.;
#ifdef YADE_DEFORM
	const BodyContainer& bodies = *scene->bodies;
	const State& de1 = *static_cast<State*>(bodies[id1]->state.get());
	const State& de2 = *static_cast<State*>(bodies[id2]->state.get());
	addDR = de1.dR + de2.dR;
#endif

	if ((geom.penetrationDepth + addDR)<0) {
		return false;
	} else {
#ifndef YADE_DEFORM
		// These 3 lines were duplicated (see above) not to loose
		// runtime performance, if YADE_DEFORM is disabled and no
		// contact detected
		const BodyContainer& bodies = *scene->bodies;
		const State& de1 = *static_cast<State*>(bodies[id1]->state.get());
		const State& de2 = *static_cast<State*>(bodies[id2]->state.get());
#endif
		Vector3r& shearForce = phys.shearForce;
		if (I->isFresh(scene)) shearForce=Vector3r(0,0,0);
		const Real& dt = scene->dt;
		shearForce = geom.rotate(shearForce);
	
		// Handle periodicity.
		const Vector3r shift2 = scene->isPeriodic ? scene->cell->intrShiftPos(I->cellDist): Vector3r::Zero(); 
		const Vector3r shiftVel = scene->isPeriodic ? scene->cell->intrShiftVel(I->cellDist): Vector3r::Zero(); 
	
		const Vector3r c1x = (geom.contactPoint - de1.pos);
		const Vector3r c2x = (geom.contactPoint - de2.pos - shift2);
		
		const Vector3r relativeVelocity = (de1.vel+de1.angVel.cross(c1x)) - (de2.vel+de2.angVel.cross(c2x)) + shiftVel;
		const Real normalVelocity	= geom.normal.dot(relativeVelocity);
		const Vector3r shearVelocity	= relativeVelocity-normalVelocity*geom.normal;
		
		// As Chiara Modenese suggest, we store the elastic part 
		// and then add the viscous part if we pass the Mohr-Coulomb criterion.
		// See http://www.mail-archive.com/yade-users@lists.launchpad.net/msg01391.html
		shearForce += phys.ks*dt*shearVelocity; // the elastic shear force have a history, but
		Vector3r shearForceVisc = Vector3r::Zero(); // the viscous shear damping haven't a history because it is a function of the instant velocity 
	
	
		// Prevent appearing of attraction forces due to a viscous component
		// [Radjai2011], page 3, equation [1.7]
		// [Schwager2007]
		phys.Fn = phys.kn * (geom.penetrationDepth + addDR);
		phys.Fv = phys.cn * normalVelocity;
		const Real normForceReal = phys.Fn + phys.Fv;
		if (normForceReal < 0) {
			phys.normalForce = Vector3r::Zero();
		} else {
			phys.normalForce = normForceReal * geom.normal;
		}
		
		Vector3r momentResistance = Vector3r::Zero();
		if (phys.mR>0.0) {
			const Vector3r relAngVel  = de1.angVel - de2.angVel;
			relAngVel.normalized();
			
			if (phys.mRtype == 1) { 
				momentResistance = -phys.mR*phys.normalForce.norm()*relAngVel;																														// [Zhou1999536], equation (3)
			} else if (phys.mRtype == 2) { 
				momentResistance = -phys.mR*(c1x.cross(de1.angVel) - c2x.cross(de2.angVel)).norm()*phys.normalForce.norm()*relAngVel;			// [Zhou1999536], equation (4)
			}
		}
		
		const Real maxFs = phys.normalForce.squaredNorm() * std::pow(phys.tangensOfFrictionAngle,2);
		if( shearForce.squaredNorm() > maxFs )
		{
			// Then Mohr-Coulomb is violated (so, we slip), 
			// we have the max value of the shear force, so 
			// we consider only friction damping.
			const Real ratio = sqrt(maxFs) / shearForce.norm();
			shearForce *= ratio;
		} 
		else 
		{
			// Then no slip occurs we consider friction damping + viscous damping.
			shearForceVisc = phys.cs*shearVelocity; 
		}
		force = phys.normalForce + shearForce + shearForceVisc;
		torque1 = -c1x.cross(force)+momentResistance;
		torque2 =  c2x.cross(force)-momentResistance;
		return true;
	}
}

void Ip2_ViscElMat_ViscElMat_ViscElPhys::Calculate_ViscElMat_ViscElMat_ViscElPhys(const shared_ptr<Material>& b1, const shared_ptr<Material>& b2, const shared_ptr<Interaction>& interaction, shared_ptr<ViscElPhys> phys) {
	ViscElMat* mat1 = static_cast<ViscElMat*>(b1.get());
	ViscElMat* mat2 = static_cast<ViscElMat*>(b2.get());
	Real mass1 = 1.0;
	Real mass2 = 1.0;
	
	if ((isfinite(mat1->kn) and  not (isfinite(mat2->kn))) or
			(isfinite(mat2->kn) and  not (isfinite(mat1->kn))) or
			(isfinite(mat1->ks) and  not (isfinite(mat2->ks))) or
			(isfinite(mat2->ks) and  not (isfinite(mat1->ks))) or
			(isfinite(mat1->cn) and  not (isfinite(mat2->cn))) or
			(isfinite(mat2->cn) and  not (isfinite(mat1->cn))) or
			(isfinite(mat1->cs) and  not (isfinite(mat2->cs))) or
			(isfinite(mat2->cs) and  not (isfinite(mat1->cs))) or
			(isfinite(mat1->tc) and  not (isfinite(mat2->tc))) or
			(isfinite(mat2->tc) and  not (isfinite(mat1->tc))) or
			(isfinite(mat1->en) and  not (isfinite(mat2->en))) or
			(isfinite(mat2->en) and  not (isfinite(mat1->en))) or
			(isfinite(mat1->et) and  not (isfinite(mat2->et))) or
			(isfinite(mat2->et) and  not (isfinite(mat1->et)))) {
				throw runtime_error("Both materials should have the same defined set of variables e.g. tc, ks etc.!"); 
			}
			
	mass1 = Body::byId(interaction->getId1())->state->mass;
	mass2 = Body::byId(interaction->getId2())->state->mass;
	if (mass1 == 0.0 and mass2 > 0.0) {
		mass1 = mass2;
	} else if (mass2 == 0.0 and mass1 > 0.0) {
		mass2 = mass1;
	}
	
	// See [Pournin2001, just below equation (19)]
	const Real massR = mass1*mass2/(mass1+mass2);
	
	GenericSpheresContact* sphCont=YADE_CAST<GenericSpheresContact*>(interaction->geom.get());
	Real R1=sphCont->refR1>0?sphCont->refR1:sphCont->refR2;
	Real R2=sphCont->refR2>0?sphCont->refR2:sphCont->refR1;
	
	Real kn1 = 0.0; Real kn2 = 0.0;
	Real cn1 = 0.0; Real cn2 = 0.0;
	Real ks1 = 0.0; Real ks2 = 0.0;
	Real cs1 = 0.0; Real cs2 = 0.0;
	
	if (((isfinite(mat1->tc)) and (isfinite(mat1->en)) and (isfinite(mat1->et)))  or ((tc) and (en) and (et))) {
		//Set parameters according to [Pournin2001]
		
		const Real Tc = (tc) ? (*tc)(mat1->id,mat2->id) : (mat1->tc+mat2->tc)/2.0;
		const Real En = (en) ? (*en)(mat1->id,mat2->id) : (mat1->en+mat2->en)/2.0;
		const Real Et = (et) ? (*et)(mat1->id,mat2->id) : (mat1->et+mat2->et)/2.0;
    
    // Factor 2 at the end of each expression is necessary, because we calculate
    // individual kn1, kn2, ks1, ks2 etc., because kn1 = 2*kn, ks1 = 2*ks
    // http://www.mail-archive.com/yade-users@lists.launchpad.net/msg08778.html
    kn1 = kn2 = 1/Tc/Tc * ( Mathr::PI*Mathr::PI + pow(log(En),2) )*massR*2;
    cn1 = cn2 = -2.0 /Tc * log(En)*massR*2;
    ks1 = ks2 = 2.0/7.0 /Tc/Tc * ( Mathr::PI*Mathr::PI + pow(log(Et),2) )*massR*2;
    cs1 = cs2 = -4.0/7.0 /Tc * log(Et)*massR*2;
    //           ^^^
    // It seems to be an error in [Pournin2001] (22) Eq.4, missing factor 2
    // Thanks to Dominik Boemer for pointing this out
    // http://www.mail-archive.com/yade-users@lists.launchpad.net/msg08741.html

		if (std::abs(cn1) <= Mathr::ZERO_TOLERANCE ) cn1=0;
		if (std::abs(cn2) <= Mathr::ZERO_TOLERANCE ) cn2=0;
		if (std::abs(cs1) <= Mathr::ZERO_TOLERANCE ) cs1=0;
		if (std::abs(cs2) <= Mathr::ZERO_TOLERANCE ) cs2=0;
	} else if ((isfinite(mat1->kn)) and (isfinite(mat1->ks)) and (isfinite(mat1->cn)) and (isfinite(mat1->cs))) {
		//Set parameters explicitly
		kn1 = mat1->kn;
		kn2 = mat2->kn;
		ks1 = mat1->ks;
		ks2 = mat2->ks;
		cn1 = mat1->cn;
		cn2 = mat2->cn;
		cs1 = mat1->cs;
		cs2 = mat2->cs;
	} else {
		//Set parameters on the base of young modulus
		kn1 = 2*mat1->young*R1;
		kn2 = 2*mat2->young*R2;
		ks1 = kn1*mat1->poisson;
		ks2 = kn2*mat2->poisson;
		if ((isfinite(mat1->cn)) and (isfinite(mat1->cs))) {
			cn1 = mat1->cn;
			cn2 = mat2->cn;
			cs1 = mat1->cs;
			cs2 = mat2->cs;
		}
		else if( isfinite(mat1->en) and isfinite(mat1->et)) {
			const Real En = (en) ? (*en)(mat1->id,mat2->id) : (mat1->en+mat2->en)/2.0;
			cn1 = cn2 = 2.0*find_cn_from_en(En, massR,contactParameterCalculation(kn1,kn2),interaction);
			cs1 = cs2 = 0;
		}
	}
	
	const Real mR1 = mat1->mR;      const Real mR2 = mat2->mR; 
	const int mRtype1 = mat1->mRtype; const int mRtype2 = mat2->mRtype;
	
	
	phys->kn = contactParameterCalculation(kn1,kn2);
	phys->ks = contactParameterCalculation(ks1,ks2);
	phys->cn = contactParameterCalculation(cn1,cn2);
	phys->cs = contactParameterCalculation(cs1,cs2);

 	if ((mR1>0) or (mR2>0)) {
		phys->mR = 2.0/( ((mR1>0)?1/mR1:0) + ((mR2>0)?1/mR2:0) );
	} else {
		phys->mR = 0;
	}

	if (frictAngle) {
		phys->tangensOfFrictionAngle = std::tan((*frictAngle)(mat1->id,mat2->id));
	} else {
		phys->tangensOfFrictionAngle = std::tan(std::min(mat1->frictionAngle, mat2->frictionAngle));
	}
	phys->shearForce = Vector3r(0,0,0);
	
	if ((mRtype1 != mRtype2) or (mRtype1>2) or (mRtype2>2) or (mRtype1<1) or (mRtype2<1) ) {
		throw runtime_error("mRtype should be equal for both materials and have the values 1 or 2!");
	} else {
		phys->mRtype = mRtype1;
	}
#ifdef YADE_SPH
	if (mat1->SPHmode and mat2->SPHmode)  {
		phys->SPHmode=true;
		phys->mu=(mat1->mu+mat2->mu);
		phys->h=(mat1->h+mat2->h)/2.0;
	}
	
	phys->kernelFunctionCurrentPressure = returnKernelFunction (mat1->KernFunctionPressure, mat2->KernFunctionPressure, Grad);
	phys->kernelFunctionCurrentVisco    = returnKernelFunction (mat1->KernFunctionVisco, mat2->KernFunctionVisco, Lapl);
#endif
}

/* Contact parameter calculation function */
Real contactParameterCalculation(const Real& l1, const Real& l2){
  // If one of paramaters > 0. we DO NOT return 0
  Real a = (l1?1/l1:0) + (l2?1/l2:0);
  if (a) return 1/a;
  else return 0;
}

Real find_cn_from_en(const Real& en, const Real& m, const Real& kn, const shared_ptr<Interaction>& interaction){
	Real eps = Ip2_ViscElMat_ViscElMat_ViscElPhys::epsilon;
	Real cn = eps ; //initial small value
	Real en_temp=get_en_from_cn(cn,m ,kn);
	int i =0;
	Real error = 1.0/eps;
	while (error > 1.0e-2 or error!=error){
		if(i>15){
			cn = 0.;
			en_temp = 1.;
			cerr<<"Warning in ViscoelasticPM.cpp : Newton-Raphson algorithm did not converged within 15 iterations for contact between "<<interaction->id1<<" and "<<interaction->id2<<". Continue with values : cn="<<cn<<" en="<<en_temp<<endl;
			break;
		}
		i++;
		Real deriv=(get_en_from_cn(cn-eps,m ,kn)-get_en_from_cn(cn+eps,m ,kn))/(-2.*eps);
		deriv = fabs(deriv)>1e-15?deriv:1e-15;
		cn = cn - (en_temp-en)/deriv;
		en_temp=get_en_from_cn(cn,m ,kn);
		error = fabs(en_temp-en)/en;
	}
// 	cout<<"i="<<i<<" error="<<error<<endl;
	return cn;
}

Real get_en_from_cn(const Real& cn, const Real& m, const Real& kn){
	Real beta = 0.5*cn/m;
	Real omega0 = sqrt(kn/m);
	Real omega = sqrt(omega0*omega0-beta*beta);
	Real Omega = sqrt(beta*beta-omega0*omega0);
	if ( beta < omega0/sqrt(2.) )
		return exp(-beta/omega*(Mathr::PI-atan(2.*beta*omega/(omega*omega-beta*beta))));
	else if ( beta > omega0/sqrt(2.) and  beta < omega0)
		return exp(-beta/omega*atan(-2.*beta*omega/(omega*omega-beta*beta)));
	else if ( beta > omega0 )
		return exp(-beta/Omega*log((beta+Omega)/(beta-Omega)));
	else if ( beta == omega0/sqrt(2.) or beta == omega0 )
		return get_en_from_cn(cn + Ip2_ViscElMat_ViscElMat_ViscElPhys::epsilon, m, kn);
	else return 0;
}


#ifdef YADE_DEFORM
// The reference paper [Haustein2017]
// functor with Raji1999 Eq. 2.52
template <class T>
struct fkt_functor
{
  fkt_functor(T Radius, T tdR, vector<T>& distanceVector) : R(Radius), dR(tdR), coef(distanceVector) {}

  pair<T, T> operator()(T const& Rs)
  {
    // solve for radius of deformed sphere Rs
    T funktion = -R*R*R + Rs*Rs*Rs;        // Raji1999 Eq. 2.52 - Part outside of the sum
    T dfunktion = 3*Rs*Rs;                 // Derivation of Raji1999 Eq. 2.52 - Part outside of the sum

    // Summation over every contact distance dsi (C++11)
    for(auto const& dsi : coef)
    {
      funktion  += -0.25 * (Rs - dsi)*(Rs - dsi) * (2 * Rs + dsi) ;      // Raji1999 Eq. 2.51 - part in the sum
      dfunktion += 3.0/2.0 * ( Rs*Rs - Rs*dsi ) ;                        // Derivation of Raji1999 Eq. 2.52 - part in the sum
    }

        return make_pair(funktion, dfunktion);
  }
  private:
    T R;                    // radius of the undeformed sphere
    T dR;                   // dR of sphere
    vector <T> coef;        // vector of all contact distances dsi
};

// function for easy calling of Newton-Raphson method
template <class T>
T fkt(T R, T dR, vector<T> z)
{
  double guess = R + dR;       // start guess
  double min = guess * 0.99;   // minimum
  double max = guess * 1.05 ;  // maximum
  int digits = std::numeric_limits<T>::digits ;

  // use Newton-Raphson method for numerical solution
  return boost::math::tools::newton_raphson_iterate(fkt_functor<T>(R, dR, z), guess, min, max, digits);
}


YADE_PLUGIN((DeformControl));
void DeformControl::action()
{
  Scene* scene=Omega::instance().getScene().get();
  const BodyContainer& b = *scene->bodies;

  for(size_t i = 0; i < b.size(); ++i)
  {
    vector<double> dsi;
    if ( Sphere* s1 = dynamic_cast<Sphere*>(b[i]->shape.get()) )
    {
      double s1Rad = s1->radius ;
      State* s1_state = static_cast<State*>(b[i]->state.get());
      double s1dR  = s1_state->dR ;

      for(Body::MapId2IntrT::iterator it=b[i]->intrs.begin(),end=b[i]->intrs.end(); it!=end; ++it)
      {
        if(!it->second->isReal()) continue;

        unsigned int partnerID;
        if( it->second->getId1() == Body::id_t(i) )
        {
          partnerID = it->second->getId2();
        } else
        {
          partnerID = it->second->getId1();
        }

        // Sphere - Sphere contact
        if(Sphere* s2 = dynamic_cast<Sphere*>(b[partnerID]->shape.get() ))
        {
          double s2Rad = s2->radius;
          State* s2_state = static_cast<State*>(b[partnerID]->state.get());
          double s2dR  = s2_state->dR;

          if ( ScGeom* scg = dynamic_cast<ScGeom*>(it->second->geom.get())  )
          {
            double L = s1Rad + s2Rad - scg->penetrationDepth;
            double s1RdR = s1Rad + s1dR;
            double s2RdR = s2Rad + s2dR;
            double ds = (L*L + s1RdR*s1RdR - s2RdR*s2RdR) / (2.0 * L);
            dsi.push_back( ds );
          }
        }
        else   // Sphere - Facet / Wall contact
        {
          if ( ScGeom* scg = dynamic_cast<ScGeom*>(it->second->geom.get())  )
          {
            double ds = s1Rad - scg->penetrationDepth ;
            dsi.push_back( ds );
          }
        }
      }
      s1_state->dR = fkt(s1Rad, s1dR, dsi) - s1Rad;
    }
  }
}
#endif

} // namespace yade

