// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include "QMStateDiscreteGlobal.hpp"
#include "SchrodingerPropagator.hpp"
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
		if(analytic and /* FIXME? not sure... will it try to propagate potentials?? */ (analytic->isAnalytic()) )
		{
			analytic->t += dt;
			analytic->wasGenerated = false;
//std::cerr << "SchrodingerAnalyticPropagator t+=dt " << b->getId() << "\n";

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

NDimTable<Complexr> SchrodingerKosloffPropagator::get_full_potentialInteractionGlobal_psiGlobalTable()
{
	// previous loop was:   // FIXME - should be somewhere else!!!!!  ← this is for Koslofff eq.2.4 !!!
	// previous loop was:   // prepare the potential  ψᵥ
	// previous loop was:   NDimTable<Complexr> Vpsi={};
	// previous loop was:   FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
	// previous loop was:   	QMIGeom* igeom=dynamic_cast<QMIGeom*>(i->geom.get());
	// previous loop was:   	if(igeom) {
	// previous loop was:   		if(Vpsi.rank()==0) Vpsi =igeom->potentialMarginalDistribution;  // ψᵥ: V = ∑Vᵢ // FIXME chyba lepiej miec jakąś wavefunction obsługującą całość?
	// previous loop was:   		else               Vpsi+=igeom->potentialMarginalDistribution;  // ψᵥ: V = ∑Vᵢ // FIXME i używając jej rozmiar bym tworzył potencjał?
	// previous loop was:   	}
	// previous loop was:   };


	// FIXME - should be somewhere else!!!!!  ← this is for Koslofff eq.2.4 !!! FIXME FIXME FIXME FIXME,,,, FIXME, FIXME, FIXME, FIXME,
	// prepare the potential  ψᵥ
	std::set<boost::shared_ptr<QMStateDiscreteGlobal> > allPotentials={};
	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential, but take care of entanglement
		QMIPhys* iphys=dynamic_cast<QMIPhys*>(i->phys.get());
		if(iphys) {
			allPotentials.insert(iphys->potentialInteractionGlobal);
		} else {
			std::cerr << "\n\nW̲A̲R̲N̲I̲N̲G̲:̲ SchrodingerKosloffPropagator::eMin can't find QMIPhys inside Interaction.\n\n";
		}
	};

	if(allPotentials.size() > 1) {
		if(timeLimit.messageAllowed(10)) std::cerr << "\n\nWARNING: SchrodingerKosloffPropagator::eMin may not work now with \
more than one globally entangled wavefunction (eg. two hydrogen atoms, four particles). This must be fixed later. But it works with \
several potential barriers affecting THE SAME particle.\nB̲T̲W̲ ̲-̲ ̲t̲h̲i̲s̲ ̲f̲u̲n̲c̲t̲i̲o̲n̲ ̲i̲s̲ ̲e̲x̲t̲r̲e̲m̲e̲l̲y̲ ̲i̲n̲e̲f̲f̲i̲c̲i̲e̲n̲t̲!̲ ̲C̲o̲n̲s̲t̲r̲u̲c̲t̲s̲ ̲t̲h̲e̲ ̲s̲a̲m̲e̲ ̲n̲e̲w̲ ̲N̲D̲i̲m̲T̲a̲b̲l̲e̲ ̲\
o̲n̲ ̲e̲a̲c̲h̲ ̲c̲a̲l̲l̲!̲ ̲I̲ ̲n̲e̲e̲d̲ ̲s̲o̲m̲e̲ ̲d̲i̲r̲t̲y̲ ̲f̲l̲a̲g̲.̲\n";
	}

	NDimTable<Complexr> Vpsi={};
	for(auto& pot : allPotentials) {
	// FIXME - this is actually a little wrong. Sometimes I can't add together different potentials !!!
	//         I should perform whole separate SchrodingerKosloffPropagator integration for each of them!
	//                  → this is the case of two separate systems entangled together
	//           But sometimes I have just several different potential sources, which I should sum together
	//                  → e.g. several barriers
	//
		if(Vpsi.rank()==0) Vpsi =pot->psiGlobalTable;  // ψᵥ: V = ∑Vᵢ
		else               Vpsi+=pot->psiGlobalTable;  // ψᵥ: V = ∑Vᵢ
	}
	// FIXME end

	//if(allPotentials.size() == 0)	return 0;
	//NDimTable<Complexr>& Vpsi=(*(allPotentials.begin()))->psiGlobalTable;

//	for(int i=0;i<Vpsi.dim()[0];i++)
//		std::cerr << i << " " << Vpsi.at(i) << "\n";
//	for(int i=0;i<Vpsi.dim()[0];i++)
//		std::cerr << i << " " << Vpsi.at(i,Vpsi.dim()[0]/2) << "\n";

	return std::move(Vpsi);
};

boost::shared_ptr<QMStateDiscreteGlobal> SchrodingerKosloffPropagator::get_full_psiGlobal__________________psiGlobalTable()
{
	// previous loop was:   FOREACH(const shared_ptr<Body>& b, *scene->bodies){
	// previous loop was:   	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
	// previous loop was:   	if(psi) {
	// previous loop was:   		int rank = psi->psiMarginalDistribution.rank();
	// previous loop was:   		Real Ekin(0);
	// previous loop was:   		for(int dim=0 ; dim<rank ; dim++)
	// previous loop was:   			Ekin += std::pow(psi->kMax(dim)* 1/* FIXME: must be `hbar` here */,2)/(2 /*FIXME: must be mass here psi->m */);
	// previous loop was:   		ret=std::max(ret, Ekin );
	// previous loop was:   	}
	// previous loop was:   };
	
	std::set<boost::shared_ptr<QMStateDiscreteGlobal> > allPsiGlobals={};
	FOREACH(const shared_ptr<Body>& b , *scene->bodies){
		QMStateDiscrete* psiLocal=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(    psiLocal and psiLocal->isNumeric() and psiLocal->getPsiGlobalExists() )
			allPsiGlobals.insert(psiLocal->getPsiGlobalExisting());
	};
	if(allPsiGlobals.size() != 1) {
		std::cerr << "\n\nERROR: SchrodingerKosloffPropagator::get_full_psiGlobal__________________psiGlobalTable() two separate global wavefunctions!\n\n";
	//         I should perform whole separate SchrodingerKosloffPropagator integration for each of them!
		exit(1);
	}
	return *allPsiGlobals.begin();
};

// Virial theorem: 2*<E_kin> = -<E_pot>

void SchrodingerKosloffPropagator::virialTheorem_Grid_check()
{
	if(virialCheck) {
		Real E_pot = abs(eMin()); // FIXME - tu powinna być liczona średnia! A nie MAX.
		Real E_kin = abs(eMax()); // FIXME - tu powinna być liczona średnia! A nie MAX.
		if(2*E_kin > E_pot) {
			std::cerr << "ERROR: too big potential energy for this gridsize\n";
			//exit(0);
		};
	};
};

Real SchrodingerKosloffPropagator::eMin()
{
	NDimTable<Complexr> VGlobal( get_full_potentialInteractionGlobal_psiGlobalTable() );
	return ((VGlobal.rank()!=0) ? (VGlobal.minReal()) : (0));
};

Real SchrodingerKosloffPropagator::eMax()
{
	Real ret(0); // assume that negative maximum energy is not possible
	// FIXME                                                                                ↓ ?  bez sensu, że w obu to się nazywa psiGlobalTable ....
	boost::shared_ptr<QMStateDiscreteGlobal> psiGlobal( get_full_psiGlobal__________________psiGlobalTable() );
	if(psiGlobal) {
		int rank = psiGlobal->psiGlobalTable.rank();
		Real Ekin(0);
		for(int dim=0 ; dim<rank ; dim++)
			Ekin += std::pow(psiGlobal->kMax(dim)* 1/* FIXME: must be `hbar` here */,2)/(2 /*FIXME: must be mass here psi->m */);
		ret=std::max(ret, Ekin );
	}
	// FIXME                                                                                ↓ ?  bez sensu, że w obu to się nazywa psiGlobalTable ....
	NDimTable<Complexr>                        VGlobal( get_full_potentialInteractionGlobal_psiGlobalTable() );
	ret += ((VGlobal.rank()!=0) ? (VGlobal.maxReal()) : (0));
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
		hasTable=true;
		// FIXME!!!!!  wow, gdy będę miał dobry warunek na krok siatki, to będę mógł automatycznie zagęszczać i rozluźniać siatkę!
		//             wystarczy że nowa siatka będzie interpolowana ze starej siatki, i co jakiś czas będzie automatycznie skalowana
		kTable.becomeMinusKSquaredTable( [&psi](Real i,int d)->Real{ return psi->iToK(i,d);});
//? ← zakomentowane to oznacza, że mnożenie przez -k² odbywa się na "złej" wersji przestrzeni odwrotnej |         ,-.         | vs. |-.                  ,|
//    ale o dziwno wszystkie wyniki są dobre, pytanie: dlaczego?                                        |________/___\________| vs. |__\________________/_|
//? kTable.shiftByHalf();
	}
	// FIXME,FIXME ↑ -- this should be somewhere else. Still need to find a good place.
	
	// prepare the potential  ψᵥ
	NDimTable<Complexr> Vpsi( get_full_potentialInteractionGlobal_psiGlobalTable() );
	
	// previous loop was:   NDimTable<Complexr> Vpsi(psi_0.dim(),0);
	// previous loop was:   FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
	// previous loop was:   	QMIGeom* igeom=dynamic_cast<QMIGeom*>(i->geom.get());
	// previous loop was:   	if(igeom) {
	// previous loop was:   		Vpsi+=igeom->potentialMarginalDistribution;  // ψᵥ: V = ∑Vᵢ
	// previous loop was:   	}
	// previous loop was:   };

	if(Vpsi.rank() != 0)
		Vpsi    .multMult(psi_0,dt/(hbar*R));// ψᵥ: ψᵥ=(dt V ψ₀)/(ℏ R)

//? NDimTable<Complexr> psi_0c(psi_0);
//? psi_0c.shiftByHalf();
	psi_1  = FFT(psi_0);                 // ψ₁: ψ₁=              ℱ(ψ₀)
//? psi_1  = FFT(psi_0c);                 // ψ₁: ψ₁=              ℱ(ψ₀)
//? psi_1.shiftByHalf();
	psi_1 *= kTable;                     // ψ₁: ψ₁=           -k²ℱ(ψ₀)
//? psi_1.shiftByHalf();
	psi_1   .IFFT();                     // ψ₁: ψ₁=       ℱ⁻¹(-k²ℱ(ψ₀))
//? psi_1.shiftByHalf();
	psi_1 *= dt*hbar/(R*2*mass);         // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	psi_1   .mult2Add(psi_0,(1+G/R));    // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀

	if(Vpsi.rank() != 0)
		psi_1 -= Vpsi;               // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)

	// FIXME: return std::move(psi_1);
}

void SchrodingerKosloffPropagator::action()
{
	virialTheorem_Grid_check(); // FIXME - to powinno być chyba zależne od potencjału...
	timeLimit.readWallClock();
	Real R   = calcKosloffR(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real R13 = 1.3*R;
	Real min = 100.0*std::numeric_limits<Real>::min(); // get the numeric minimum, smallest number. To compare if anything is smaller than it, this one must be larger.
	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
// FIXME - for multiple entangled wavefunctions
//	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){

		boost::shared_ptr<QMStateDiscreteGlobal> psiGlobal( get_full_psiGlobal__________________psiGlobalTable() );
//		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(psiGlobal) {
			// FIXME: this is    ↓ only because with & it draws the middle of calculations
			NDimTable<Complexr>/*&*/ psi_dt(psiGlobal->psiGlobalTable); // will become ψ(t+dt): ψ(t+dt) = ψ₀
			NDimTable<Complexr>  psi_0 (psi_dt);            // ψ₀
			NDimTable<Complexr>  psi_1 = {};                // ψ₁
			calc_Hnorm_psi(psi_0,psi_1,psiGlobal.get());    // ψ₁     : ψ₁     =(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
			Complexr ak0=calcAK(0,R);                       // a₀
			Complexr ak1=calcAK(1,R);                       // a₁
			psi_dt .mult1Mult2Add(ak0, psi_1,ak1);          // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
			int i(0);
			Complexr ak(1);
			// never stop when i < R*1.3, unless steps is positive. Auto stop expanding series based on std::numeric_limits<Real>::min()
			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
			{
				NDimTable<Complexr> psi_2;              // ψ₂     :
				calc_Hnorm_psi(psi_1,psi_2,psiGlobal.get());//ψ₂  : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
				psi_2  .mult1Sub(2,psi_0);              // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				psi_dt .mult2Add(psi_2,ak=calcAK(i,R)); // ψ(t+dt): ψ(t+dt)=ψ(t+dt) + aₖψₖ
				psi_0=std::move(psi_1);                 // ψ₀ ← ψ₁
				psi_1=std::move(psi_2);                 // ψ₁ ← ψ₂
			}
			psi_dt *= std::exp(-1.0*Mathr::I*(R+G));        // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)

			// FIXME: this is    ↓ only because with & it draws the middle of calculations
			psiGlobal->psiGlobalTable = psi_dt;
			// FIXME: but it actually wastes twice more memory

			if(timeLimit.messageAllowed(4)) std::cerr << "final |ak|=" << boost::lexical_cast<std::string>(std::abs(std::real(ak))+std::abs(std::imag(ak))) << " iterations: " << i-1 << "/" << steps << "\n";
			if(timeLimit.messageAllowed(6)) std::cerr << "Muszę wywalić hbar ze SchrodingerKosloffPropagator i używać to co jest w QMIPhys, lub obok.\n";
//std::cerr << "SchrodingerKosloffPropagator t+=dt (calculating) " << b->getId() << "\n";
		}
// FIXME - for multiple entangled wavefunctions
//	} YADE_PARALLEL_FOREACH_BODY_END();
}

