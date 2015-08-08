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

//#include <parallel/algorithm>
//#include <parallel/settings.h>

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
			// FIXME - problem polega na tym, że QMState służy do przechowywania funkcji falowych (które zwykle zależą od czasu), podczas
			// gdy St1_QMState służy do przechowywania potencjałów. Ale jednocześnie oba używają QMStateAnalytic, żeby trzymać parametry
			// do wzorów, oraz wzory.
			//
			// Rozwiązanie jest takie, że analitycznyść powinienem oddelegować do NDimTable i w ten sposób już nigdy sobie nie zawracać nią głowy
			// po prostu jako generator NDimTable będę podawał funkcję, w stylu [](){ return val;};
			if(analytic->changesWithTime_FIXME()) { analytic->wasGenerated = false; };
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

const NDimTable<Complexr>& SchrodingerKosloffPropagator::get_full_potentialInteractionGlobal_psiGlobalTable()
{
	static bool generatedPotential_not_depends_on_time(false);
	static NDimTable<Complexr> Vpsi_static={};
	if(not generatedPotential_not_depends_on_time) {
		generatedPotential_not_depends_on_time = true;

		std::cerr << "W̲A̲R̲N̲I̲N̲G̲:̲ SchrodingerKosloffPropagator assuming that potential does not depend on time.\n";

	// FIXME - should be somewhere else!!!!!  ← this is for Koslofff eq.2.4 !!! FIXME FIXME FIXME FIXME,,,, FIXME, FIXME, FIXME, FIXME,
	// prepare the potential  ψᵥ
	std::set<boost::shared_ptr<QMStateDiscreteGlobal> > allPotentials={};
	FOREACH(const shared_ptr<Interaction>& I, *scene->interactions){ // collect all potentials into one potential, but take care of entanglement
		QMIPhys* iphys=dynamic_cast<QMIPhys*>(I->phys.get());
		if(iphys
		  and ( // FIXME - perhaps should be checking groupMask here??????
			   dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get())->isNumeric() // -> groupMask1  == ...->groupMask2
			or dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id2]->state.get())->isNumeric()
		      )
		  )
		{
			allPotentials.insert(iphys->potentialInteractionGlobal);
		} else {
			if(timeLimit.messageAllowed(30))
				std::cerr << "W̲A̲R̲N̲I̲N̲G̲:̲ SchrodingerKosloffPropagator::get_full_potentialInteractionGlobal_psiGlobalTable can't find QMIPhys inside Interaction.\n";
		}
	};

	if(allPotentials.size() > 1) {
		if(timeLimit.messageAllowed(10))
			std::cerr << "\n\nWARNING: SchrodingerKosloffPropagator::eMin may not work now with \
more than one globally entangled wavefunction (eg. two hydrogen atoms, four particles). This must be fixed later. But it works with \
several potential barriers affecting THE SAME particle.\nB̲T̲W̲ ̲-̲ ̲t̲h̲i̲s̲ ̲f̲u̲n̲c̲t̲i̲o̲n̲ ̲i̲s̲ ̲e̲x̲t̲r̲e̲m̲e̲l̲y̲ ̲i̲n̲e̲f̲f̲i̲c̲i̲e̲n̲t̲!̲ ̲C̲o̲n̲s̲t̲r̲u̲c̲t̲s̲ ̲t̲h̲e̲ ̲s̲a̲m̲e̲ ̲n̲e̲w̲ ̲N̲D̲i̲m̲T̲a̲b̲l̲e̲ ̲\
o̲n̲ ̲e̲a̲c̲h̲ ̲c̲a̲l̲l̲!̲ ̲I̲ ̲n̲e̲e̲d̲ ̲s̲o̲m̲e̲ ̲d̲i̲r̲t̲y̲ ̲f̲l̲a̲g̲.̲\n";
//		exit(1);
	}

HERE2;
	for(auto& pot : allPotentials) {
	// FIXME - this is actually a little wrong. Sometimes I can't add together different potentials !!!
	//         I should perform whole separate SchrodingerKosloffPropagator integration for each of them!
	//                  → this is the case of two separate systems entangled together
	//           But sometimes I have just several different potential sources, which I should sum together
	//                  → e.g. several barriers
	//
HERE2;
		if(Vpsi_static.rank()==0 /* tzn. jeśli jest pusty, to wykonaj przypisanie (kopiuj) */ )
		{
HERE2;
			Vpsi_static =pot->psiGlobalTable;  // ψᵥ: V = ∑Vᵢ
HERE2;
		}
		else              /* else dodaj kolejny potencjał */
		{
HERE2;
			Vpsi_static +=pot->psiGlobalTable;  // ψᵥ: V = ∑Vᵢ
HERE2;
		}
	}
	// FIXME end

	//if(allPotentials.size() == 0)	return 0;
	//NDimTable<Complexr>& Vpsi=(*(allPotentials.begin()))->psiGlobalTable;

//	for(int i=0;i<Vpsi.dim()[0];i++)
//		std::cerr << i << " " << Vpsi.at(i) << "\n";
//	for(int i=0;i<Vpsi.dim()[0];i++)
//		std::cerr << i << " " << Vpsi.at(i,Vpsi.dim()[0]/2) << "\n";


// FIXME - ale jesli potencjał jest tylko jeden to powinienem używać referencje.........
	}
	return Vpsi_static;
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
HERE2;
	NDimTable<Complexr> VGlobal( get_full_potentialInteractionGlobal_psiGlobalTable() );
	return ((VGlobal.rank()!=0) ? (VGlobal.minReal()) : (0));
};

Real SchrodingerKosloffPropagator::eKin()
{
	Real ret(0); // assume that negative maximum energy is not possible
	// FIXME                                                                                ↓ ?  bez sensu, że w obu to się nazywa psiGlobalTable ....
	boost::shared_ptr<QMStateDiscreteGlobal> psiGlobal( get_full_psiGlobal__________________psiGlobalTable() );
	if(psiGlobal) {
		int rank = psiGlobal->psiGlobalTable.rank();
		Real Ekin(0);
		for(int dim=0 ; dim<rank ; dim++)
			Ekin += std::pow(psiGlobal->kMax(dim)* 1/* FIXME: must be `hbar` here */,2)/(2 *FIXMEatomowe_MASS/*FIXME: must be mass here psi->m */);
		ret=std::max(ret, Ekin );
	}
	return ret;
}

Real SchrodingerKosloffPropagator::eMax()
{
	Real ret(eKin()); // assume that negative maximum energy is not possible
	// FIXME                                                                                ↓ ?  bez sensu, że w obu to się nazywa psiGlobalTable ....
HERE2;
	NDimTable<Complexr>     VGlobal( get_full_potentialInteractionGlobal_psiGlobalTable() );
	ret += ((VGlobal.rank()!=0) ? (VGlobal.maxReal()) : (0));
	return ret;
}

void SchrodingerKosloffPropagator::calc_Hnorm_psi(const NDimTable<Complexr>& psi_0,NDimTable<Complexr>& psi_1,
	/*FIXME - remove*/QMStateDiscrete* psi)
{
//delay.printDelay("calc_Hnorm_psi");	//	1: 1.7s		2: 0s		3: 0s		4: 0s		5: 0s		6: 0s		7: 0s
	Real mass(FIXMEatomowe_MASS); // FIXME - this shouldn't be here
	Real dt=scene->dt;

	Real R   = calcKosloffR(dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(dt); // FIXME -  that's duplicate here, depends on dt !!

	// FIXME,FIXME ↓ -- this should be somewhere else. Still need to find a good place.
	static bool hasTable(false);
HERE;
	static NDimTable<Real    > kTable(psi_0.dim());
	if(! hasTable){
//delay.printDelay("make kTable");	//	1: 24.2s
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
HERE;
//delay.printDelay("copy Vpsi _________");//	1: 3.4s		2: 3.2s		3: 15.4s	4: 3.6s		5: 3.3s		6: 3.3s		7: 3.1s
	static NDimTable<Complexr> Vpsi = {}; // static to avoid creating and destroying, just copy
	Vpsi = get_full_potentialInteractionGlobal_psiGlobalTable();
	
	// previous loop was:   NDimTable<Complexr> Vpsi(psi_0.dim(),0);
	// previous loop was:   FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){ // collect all potentials into one potential
	// previous loop was:   	QMIGeom* igeom=dynamic_cast<QMIGeom*>(i->geom.get());
	// previous loop was:   	if(igeom) {
	// previous loop was:   		Vpsi+=igeom->potentialMarginalDistribution;  // ψᵥ: V = ∑Vᵢ
	// previous loop was:   	}
	// previous loop was:   };

//delay.printDelay("Vpsi mult");		//	1: 0.5s		2: 0.4s		3: 0.3s		4: 0.4s		5: 0.3s		6: 0.3s		7: 0.3s
	if(Vpsi.rank() != 0)
		Vpsi    .multMult(psi_0,dt/(FIXMEatomowe_hbar*R));// ψᵥ: ψᵥ=(dt V ψ₀)/(ℏ R)

//? NDimTable<Complexr> psi_0c(psi_0);
//? psi_0c.shiftByHalf();
	psi_1.set_num_threads(threadNum);
//delay.printDelay("FFT");		//	1: 7s		2: 23.5s	3: 3.7s		4: 3.7s		5: 3.5s		6: 3.8s		7: 3.6s
	psi_1.doFFT(psi_0,true);                 // ψ₁: ψ₁=              ℱ(ψ₀)
//? psi_1  = FFT(psi_0c);                 // ψ₁: ψ₁=              ℱ(ψ₀)
//? psi_1.shiftByHalf();
//delay.printDelay("kTable");		//	1: 0.3s		2: 0.3s		3: 0.4s		4: 0.3s		5: 0.2s		6: 0.4s		7: 0.3s
	psi_1 *= kTable;                     // ψ₁: ψ₁=           -k²ℱ(ψ₀)
//? psi_1.shiftByHalf();
//delay.printDelay("IFFT");		//	1: 3.8s		2: 4.0s		3: 4.4s		4: 4.1s		5: 3.8s		6: 4.0s		7: 3.9s
	psi_1   .IFFT();                     // ψ₁: ψ₁=       ℱ⁻¹(-k²ℱ(ψ₀))
//? psi_1.shiftByHalf();
//delay.printDelay("mult,etc");		//	1: 1.0s		2: 0.8s		3: 1.25s	4: 1.0s		5: 0.9s		6: 1.2s		7: 1.0s
	psi_1 *= dt*FIXMEatomowe_hbar/(R*2*mass);         // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	psi_1   .mult2Add(psi_0,(1+G/R));    // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀

	if(Vpsi.rank() != 0)
		psi_1 -= Vpsi;               // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
//delay.printDelay("mult,etc end");	//	1: 0.7s		2: 10.8s	3: 1.1s		4: 0.9s		5: 0.9s		6: 1.2s		7: 0.9s

	// FIXME: return std::move(psi_1);
}

Real SchrodingerKosloffPropagator::calcKosloffR(Real dt)
{ // calculate R parameter in Kosloff method

	static Real last_dt(dt);
	static Real ret_R( dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar) );

	if(dt != last_dt)
	{
		last_dt=dt;
		ret_R  = dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);
	}

	return ret_R; // dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);
};
Real SchrodingerKosloffPropagator::calcKosloffG(Real dt)
{ // calculate G parameter in Kosloff method
	static Real last_dt(dt);
	static Real ret_G( dt*eMin()/(2*FIXMEatomowe_hbar) );
	
	if(dt != last_dt)
	{
		last_dt=dt;
		ret_G  = dt*eMin()/(2*FIXMEatomowe_hbar);
	}

	return ret_G; // dt*eMin()/(2*FIXMEatomowe_hbar);
};
void SchrodingerKosloffPropagator::action()
{
//	__gnu_parallel::_Settings s;
//	s.algorithm_strategy = __gnu_parallel::force_parallel;
//	__gnu_parallel::_Settings::set(s);
//	omp_set_dynamic(false);
//	omp_set_num_threads(threadNum);
	

	//virialTheorem_Grid_check(); // FIXME - to powinno być chyba zależne od potencjału...
	timeLimit.readWallClock();
	static int maxIter(0);
	Real R   = calcKosloffR(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real R13 = 1.3*R;
	Real min = 10000.0*std::numeric_limits<Real>::min(); // get the numeric minimum, smallest number. To compare if anything is smaller than it, this one must be larger.
	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
// FIXME - for multiple entangled wavefunctions
//	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){

		boost::shared_ptr<QMStateDiscreteGlobal> psiGlobal( get_full_psiGlobal__________________psiGlobalTable() );
//		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(psiGlobal and doCopyTable) {
	////   ↓↓↓       FIXME: this is    ↓ only because with & it draws the middle of calculations
HERE;
			NDimTable<Complexr>/*&*/ psi_dt(psiGlobal->psiGlobalTable); // will become ψ(t+dt): ψ(t+dt) = ψ₀
			NDimTable<Complexr>  psi_0 (psi_dt);            // ψ₀
HERE;
			NDimTable<Complexr>  psi_1 = {};                // ψ₁
			NDimTable<Complexr>  psi_2 = {};                // ψ₂     :
			calc_Hnorm_psi(psi_0,psi_1,psiGlobal.get());    // ψ₁     : ψ₁     =(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
//						1: 0.7s
			Complexr ak0=calcAK(0,R);                       // a₀
			Complexr ak1=calcAK(1,R);                       // a₁
			psi_dt .mult1Mult2Add(ak0, psi_1,ak1);          // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
			int i(0);
			Complexr ak(1);
			// never stop when i < R*1.3, unless steps is positive. Auto stop expanding series based on std::numeric_limits<Real>::min()
			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
			{
				if(printIter!=0 and ((i%printIter) ==0)) std::cerr << ":::::: SchrodingerKosloffPropagator iter = " << i << " ak="<<ak<<"\n";
//						1: dotąd 0.7s
HERE;
				calc_Hnorm_psi(psi_1,psi_2,psiGlobal.get());//ψ₂  : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
//							1:	2: 10.8s	3: 1.1s		4: 0.9s		5: 0.9s		6: 1.2s
				psi_2  .mult1Sub(2,psi_0);              // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				psi_dt .mult2Add(psi_2,ak=calcAK(i,R)); // ψ(t+dt): ψ(t+dt)=ψ(t+dt) + aₖψₖ
//delay.printDelay("mult2 etc end");        //		1:	2: 0s		3: 0s		4: 0s		5: 0s		6: 0s
				//psi_0=std::move(psi_1);                 // ψ₀ ← ψ₁
				//psi_1=std::move(psi_2);                 // ψ₁ ← ψ₂
				std::swap(psi_0,psi_1);
				std::swap(psi_1,psi_2);
//delay.printDelay("swap end");             //		1:	2: 0s		3: 0s		4: 0s		5: 0s		6: 0s
			}
			psi_dt *= std::exp(-1.0*Mathr::I*(R+G));        // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)

			// FIXME: this is    ↓ only because with & it draws the middle of calculations
	/*//	↑↑↑*/	psiGlobal->psiGlobalTable = psi_dt;
			// FIXME: but it actually wastes twice more memory

			if(timeLimit.messageAllowed(4) or printIter!=0) std::cerr << "(not &) final |ak|=" << boost::lexical_cast<std::string>(std::abs(std::real(ak))+std::abs(std::imag(ak))) << " iterations: " << i-1 << "/" << steps << "\n";
			if(timeLimit.messageAllowed(6)) std::cerr << "Muszę wywalić hbar ze SchrodingerKosloffPropagator i używać to co jest w QMIPhys, lub obok.\n";
//std::cerr << "SchrodingerKosloffPropagator t+=dt (calculating) " << b->getId() << "\n";
	// FIXME - full duplicate of the block above, except for the reference
		} else if(psiGlobal and not doCopyTable) {
			NDimTable<Complexr>& psi_dt(psiGlobal->psiGlobalTable); // will become ψ(t+dt): ψ(t+dt) = ψ₀
			NDimTable<Complexr>  psi_0 (psi_dt);            // ψ₀
			NDimTable<Complexr>  psi_1 = {};                // ψ₁
			NDimTable<Complexr>  psi_2 = {};                // ψ₂     :
			calc_Hnorm_psi(psi_0,psi_1,psiGlobal.get());    // ψ₁     : ψ₁     =(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
			Complexr ak0=calcAK(0,R);                       // a₀
			Complexr ak1=calcAK(1,R);                       // a₁
			psi_dt .mult1Mult2Add(ak0, psi_1,ak1);          // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
			int i(0);
			Complexr ak(1);
			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
			{
				if(printIter!=0 and ((i%printIter) ==0)) std::cerr << ":::::: SchrodingerKosloffPropagator O.iter="<<(scene->iter)<<", loop iter=" << i << "/" << maxIter << " ak="<<ak<<"\n";
				calc_Hnorm_psi(psi_1,psi_2,psiGlobal.get());//ψ₂  : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
				psi_2  .mult1Sub(2,psi_0);              // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				psi_dt .mult2Add(psi_2,ak=calcAK(i,R)); // ψ(t+dt): ψ(t+dt)=ψ(t+dt) + aₖψₖ
//delay.printDelay("mult2 etc end");
				//psi_0=std::move(psi_1);                 // ψ₀ ← ψ₁
				//psi_1=std::move(psi_2);                 // ψ₁ ← ψ₂
				std::swap(psi_0,psi_1);
				std::swap(psi_1,psi_2);
//delay.printDelay("swap end");
			}
			psi_dt *= std::exp(-1.0*Mathr::I*(R+G));        // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)
			if(timeLimit.messageAllowed(4) or printIter!=0) std::cerr << "(use &) final |ak|=" << boost::lexical_cast<std::string>(std::abs(std::real(ak))+std::abs(std::imag(ak))) << " iterations: " << i-1 << "/" << steps << "\n";
			maxIter = std::max(i-1,maxIter);
			if(timeLimit.messageAllowed(6)) std::cerr << "Muszę wywalić hbar ze SchrodingerKosloffPropagator i używać to co jest w QMIPhys, lub obok.\n";
		}
// FIXME - for multiple entangled wavefunctions
//	} YADE_PARALLEL_FOREACH_BODY_END();


//	omp_set_num_threads(1);
}

