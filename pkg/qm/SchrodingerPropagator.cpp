// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "SchrodingerPropagator.hpp"
#include "QMInteraction.hpp"
#include "QMPotential.hpp"
#include <core/Scene.hpp>
#include <unsupported/Eigen/FFT>
#include <functional>   // std::plus, std::multiplies
#include <algorithm>    // std::transform
#include <limits>

#ifdef YADE_FFTW3
#include <fftw3.h>
#endif

YADE_PLUGIN(
	(SchrodingerAnalyticPropagator)
	(SchrodingerKosloffPropagator)
	);

/*********************************************************************************
*
* A N A L Y T I C   P R O P A G A T I O N   of   S H R Ö D I N G E R  E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerAnalyticPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerAnalyticPropagator::~SchrodingerAnalyticPropagator(){};

void SchrodingerAnalyticPropagator::action()
{
	const Real& dt=scene->dt;
//	const Real& time=scene->time;
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
//		std::cerr << "SchrodingerAnalyticPropagator, " << b->state->getClassName() << "\n";
		QMStateAnalytic* analytic=dynamic_cast<QMStateAnalytic*>(b->state.get());
//		const Body::id_t& id=b->getId();
		if(analytic) {
			analytic->t += dt;
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerKosloffPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerKosloffPropagator::~SchrodingerKosloffPropagator(){};

Real SchrodingerKosloffPropagator::eMin()
{
	// FIXME - should be somewhere else!!!!!  ← this is for Koslofff eq.2.4 !!! FIXME FIXME FIXME FIXME,,,, FIXME, FIXME, FIXME, FIXME,
	// prepare the potential  ψᵥ
	NDimTable<Complexr> Vpsi={};
	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
		QMPotGeometry* igeom=dynamic_cast<QMPotGeometry*>(i->geom.get());
		if(igeom) {
			if(Vpsi.rank()==0) Vpsi =igeom->potentialValues;  // ψᵥ: V = ∑Vᵢ
			else               Vpsi+=igeom->potentialValues;  // ψᵥ: V = ∑Vᵢ
		}
	};
	// FIXME end
	return ((Vpsi.rank()!=0) ? (Vpsi.min()) : (0));
};

Real SchrodingerKosloffPropagator::eMax()
{
	Real ret(0); // assume that negative maximum energy is not possible
	FOREACH(const shared_ptr<Body>& b, *scene->bodies){
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(psi) {
			int rank = psi->tableValuesPosition.rank();
			Real Ekin(0);
			for(int dim=0 ; dim<rank ; dim++)
				Ekin += std::pow(psi->kMax(dim)* 1/* FIXME: must be `hbar` here */,2)/(2 /*FIXME: must be mass here psi->m */);
			ret=std::max(ret, Ekin );
		}
	};

	// FIXME - should be somewhere else!!!!!  ← this is for Koslofff eq.2.4 !!!
	// prepare the potential  ψᵥ
	NDimTable<Complexr> Vpsi={};
	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
		QMPotGeometry* igeom=dynamic_cast<QMPotGeometry*>(i->geom.get());
		if(igeom) {
			if(Vpsi.rank()==0) Vpsi =igeom->potentialValues;  // ψᵥ: V = ∑Vᵢ // FIXME chyba lepiej miec jakąś wavefunction obsługującą całość?
			else               Vpsi+=igeom->potentialValues;  // ψᵥ: V = ∑Vᵢ // FIXME i używając jej rozmiar bym tworzył potencjał?
		}
	};
	ret += ((Vpsi.rank()!=0) ? (Vpsi.max()) : (0));
	// FIXME end

	return ret;
}

void SchrodingerKosloffPropagator::calc_Hnorm_psi(const NDimTable<Complexr>& psi_0,NDimTable<Complexr>& psi_1,
	/*FIXME - remove*/QMStateDiscrete* psi)
{
	Real mass(1); // FIXME - this shouldn't be here
	Real dt=scene->dt;

	Real R   = calcKosloffR(dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(dt); // FIXME -  that's duplicate here, depends on dt !!

	// FIXME,FIXME ↓ -- this should be somewhere else. Still need to find a good place.
	static bool hasTable(false);
	static NDimTable<Real    > kTable(psi_0.dim());
	if(! hasTable){
		std::size_t rank = psi_0.rank();
		if(rank==1){
		      for(size_t i=0;i<kTable.size0(0);i++)
			      kTable.at(i  )=-std::pow(psi->iToK((i+kTable.size0(0)/2)%kTable.size0(0),0),2);
		} else
		if(rank==2){
		      for(size_t i=0;i<kTable.size0(0);i++)
		      for(size_t j=0;j<kTable.size0(1);j++)
			      kTable.at(i,j)=-std::pow(psi->iToK((i+kTable.size0(0)/2)%kTable.size0(0),0),2)
			                     -std::pow(psi->iToK((j+kTable.size0(1)/2)%kTable.size0(1),1),2);
		} //FIXME - should work for ALL, any number of !!!!!!!!!!!!!!!!!!!!! dimensions!!!!!!!!!!!!
		hasTable=true;
	//kTable.print();
	}
	
	// prepare the potential  ψᵥ
	NDimTable<Complexr> Vpsi(psi_0.dim(),0);
	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
		QMPotGeometry* igeom=dynamic_cast<QMPotGeometry*>(i->geom.get());
		if(igeom) {
			Vpsi+=igeom->potentialValues;  // ψᵥ: V = ∑Vᵢ
		}
	};
	Vpsi    .multMult(psi_0,dt/(hbar*R));// ψᵥ: ψᵥ=(dt V ψ₀)/(ℏ R)

	psi_1  = FFT(psi_0);                 // ψ₁: ψ₁=              ℱ(ψ₀)
	psi_1 *= kTable;                     // ψ₁: ψ₁=           -k²ℱ(ψ₀)
	psi_1   .IFFT();                     // ψ₁: ψ₁=       ℱ⁻¹(-k²ℱ(ψ₀))
	psi_1 *= dt*hbar/(R*2*mass);         // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	psi_1   .mult2Add(psi_0,(1+G/R));    // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀
	psi_1 -= Vpsi;                       // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)

	// FIXME: return std::move(psi_1);
}

void SchrodingerKosloffPropagator::action()
{
	timeLimit.readWallClock();
	Real R   = calcKosloffR(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real R13 = 1.3*R;
	Real min = 100.0*std::numeric_limits<Real>::min(); // get the numeric minimum, smallest number. To compare if anything is smaller than it, this one must be larger.
	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
//		std::cerr << "SchrodingerKosloffPropagator, " << b->state->getClassName() << "\n";
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
//		const Body::id_t& id=b->getId();
		if(psi) {// FIXME: this is   ↓ only because with & it draws the middle of calculations
			NDimTable<Complexr>/*&*/ psi_dt(psi->tableValuesPosition); // will become ψ(t+dt): ψ(t+dt) = ψ₀
			NDimTable<Complexr>  psi_0 (psi_dt);            // ψ₀
			NDimTable<Complexr>  psi_1 = {};                // ψ₁
			calc_Hnorm_psi(psi_0,psi_1,psi);                // ψ₁     : ψ₁     =(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
			Complexr ak0=calcAK(0,R);                       // a₀
			Complexr ak1=calcAK(1,R);                       // a₁
			psi_dt .mult1Mult2Add(ak0, psi_1,ak1);          // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
			int i(0);
			Complexr ak(1);
			// never stop when i < R*1.3, unless steps is positive. Auto stop expanding series based on std::numeric_limits<Real>::min()
			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
			{
				NDimTable<Complexr> psi_2;              // ψ₂     :
				calc_Hnorm_psi(psi_1,psi_2,psi);        // ψ₂     : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
				psi_2  .mult1Sub(2,psi_0);              // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				psi_dt .mult2Add(psi_2,ak=calcAK(i,R)); // ψ(t+dt): ψ(t+dt)=ψ(t+dt) + aₖψₖ
				psi_0=std::move(psi_1);                 // ψ₀ ← ψ₁
				psi_1=std::move(psi_2);                 // ψ₁ ← ψ₂
			}
			psi_dt *= std::exp(-1.0*Mathr::I*(R+G));        // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)
			psi->tableValuesPosition=psi_dt;
			if(timeLimit.messageAllowed(4)) std::cerr << "final |ak|=" << boost::lexical_cast<std::string>(std::abs(std::real(ak))+std::abs(std::imag(ak))) << " iterations: " << i-1 << "/" << steps << "\n";
			if(timeLimit.messageAllowed(6)) std::cerr << "Muszę wywalić hbar ze SchrodingerKosloffPropagator i używać to co jest w QMPotPhysics, lub obok.\n";
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

