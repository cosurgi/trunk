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
#include <boost/tuple/tuple_comparison.hpp>

//#include <parallel/algorithm>
//#include <parallel/settings.h>

#ifdef YADE_FFTW3
#include <fftw3.h>
#endif

YADE_PLUGIN(
	(SchrodingerAnalyticPropagator)
	(SchrodingerKosloffPropagator)
	(GlExtra_QMEngine)
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


// XXX XXX ////////////////////////////////////////////////// dotąd przeczytane XXX XXX      ←←← NIEAKTUALNE ;)

const NDimTable<Complexr>& SchrodingerKosloffPropagator::get_full_potentialInteractionGlobal_psiGlobalTable(size_t mask_id)
{
//HERE2;
//std::cerr << "\nget_full_potentialInteractionGlobal_psiGlobalTable allChannelMasks["<< mask_id <<"]="<<allChannelMasks[mask_id]<<"\n";
	//static bool generatedPotential_not_depends_on_time(false);
//	static NDimTable<Complexr> Vpsi_static={};

	if(potentialCanChangeNOW_NOTstatic__ANYMORE.size() == 0) {
		potentialCanChangeNOW_NOTstatic__ANYMORE.resize(allChannelMasks.size(),false);
	}
	if(Vpsi_NOTstatic__ANYMORE.size() == 0) {
		Vpsi_NOTstatic__ANYMORE.resize(allChannelMasks.size());
	}

	if(not potentialCanChangeNOW_NOTstatic__ANYMORE[mask_id]) {
		//generatedPotential_not_depends_on_time = true;
		potentialCanChangeNOW_NOTstatic__ANYMORE[mask_id] = true; // FIXME - to bez sensu nazwewnictwo, ale zaraz mi się przyda.

		std::cerr << "W̲A̲R̲N̲I̲N̲G̲:̲ SchrodingerKosloffPropagator assuming that potential does not depend on time.\n";

	// FIXME - should be somewhere else!!!!!  ← this is for Koslofff eq.2.4 !!! FIXME FIXME FIXME FIXME,,,, FIXME, FIXME, FIXME, FIXME,
	// prepare the potential  ψᵥ
	std::set</*std::pair<*/boost::shared_ptr<QMStateDiscreteGlobal>/*,mask_t>*/ > allPotentials={};
	std::set<boost::tuple<boost::shared_ptr<QMStateDiscreteGlobal>,mask_t,mask_t> > allInteractionPotentials={};
	FOREACH(const shared_ptr<Interaction>& I, *scene->interactions){ // collect all potentials into one potential, but take care of entanglement
		QMIPhys* iphys=dynamic_cast<QMIPhys*>(I->phys.get());
		if(iphys
		  and ( // FIXME - perhaps should be checking groupMask here??????
			   dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get())->isNumeric() // -> groupMask1  == ...->groupMask2
			or dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id2]->state.get())->isNumeric()
		      )
		// groupMask , kilka kanałów
		  and ( ( not useGroupMasks ) or ( allChannelMasks[mask_id] and
				(		(*(scene->bodies))[I->id1]->maskCompatible(allChannelMasks[mask_id])
					and 	(*(scene->bodies))[I->id2]->maskCompatible(allChannelMasks[mask_id])
				)
			)
		      )
		  )
		{
			if(  (*(scene->bodies))[I->id1]->getGroupMask() == (*(scene->bodies))[I->id2]->getGroupMask() )
			{ // do potencjału indywidualnego "H" prznaczonego wyłacznie dla danej funkcji falowej, jeśli groupMask są identyczne
				std::cerr << "→→ id1 groupMask = " << (*(scene->bodies))[I->id1]->getGroupMask() << "\n";
				std::cerr << "→→ id2 groupMask = " << (*(scene->bodies))[I->id2]->getGroupMask() << "\n";
				allPotentials.insert(/*std::make_pair(*/iphys->potentialInteractionGlobal/*, XXX )*/); // XXX
			}
			else
			{ // jeśli nie są identyczne, to znaczy że mamy potencjał oddziaływania
				std::cerr << "→→ INTER id1 groupMask = " << (*(scene->bodies))[I->id1]->getGroupMask() << "\n";
				std::cerr << "→→ INTER id2 groupMask = " << (*(scene->bodies))[I->id2]->getGroupMask() << "\n";
				allInteractionPotentials.insert(boost::make_tuple(iphys->potentialInteractionGlobal , (*(scene->bodies))[I->id1]->getGroupMask() , (*(scene->bodies))[I->id2]->getGroupMask() ));
			}
		} else {
			if(timeLimit.messageAllowed(30))
				std::cerr << "W̲A̲R̲N̲I̲N̲G̲:̲ SchrodingerKosloffPropagator::get_full_potentialInteractionGlobal_psiGlobalTable can't find QMIPhys inside Interaction.\n";
		}
	};
//HERE2;

	if(allPotentials.size() > 1) {
		if(timeLimit.messageAllowed(10))
			std::cerr << "\n\nWARNING: SchrodingerKosloffPropagator::eMin may not work now with \
more than one globally entangled wavefunction (eg. two hydrogen atoms, four particles). This must be fixed later. But it works with \
several potential barriers affecting THE SAME particle.\nB̲T̲W̲ ̲-̲ ̲t̲h̲i̲s̲ ̲f̲u̲n̲c̲t̲i̲o̲n̲ ̲i̲s̲ ̲e̲x̲t̲r̲e̲m̲e̲l̲y̲ ̲i̲n̲e̲f̲f̲i̲c̲i̲e̲n̲t̲!̲ ̲C̲o̲n̲s̲t̲r̲u̲c̲t̲s̲ ̲t̲h̲e̲ ̲s̲a̲m̲e̲ ̲n̲e̲w̲ ̲N̲D̲i̲m̲T̲a̲b̲l̲e̲ ̲\
o̲n̲ ̲e̲a̲c̲h̲ ̲c̲a̲l̲l̲!̲ ̲I̲ ̲n̲e̲e̲d̲ ̲s̲o̲m̲e̲ ̲d̲i̲r̲t̲y̲ ̲f̲l̲a̲g̲.̲\n";
//		exit(1);
	}

//HERE2;
//std::cerr << "\n→→→→→→→→→→→      allChannelMasks["<< mask_id <<"]="<<allChannelMasks[mask_id]<<" allPotentials.size()="<<allPotentials.size()<<"\n\n";
	for(auto& pot : allPotentials) {
	// FIXME - this is actually a little wrong. Sometimes I can't add together different potentials !!!
	//         I should perform whole separate SchrodingerKosloffPropagator integration for each of them!
	//                  → this is the case of two separate systems entangled together
	//           But sometimes I have just several different potential sources, which I should sum together
	//                  → e.g. several barriers
	//
//HERE2;
		if(Vpsi_NOTstatic__ANYMORE[mask_id].rank()==0 /* tzn. jeśli jest pusty, to wykonaj przypisanie (kopiuj) */ )
		{
//HERE2;
			Vpsi_NOTstatic__ANYMORE[mask_id] =pot->psiGlobalTable;  // ψᵥ: V = ∑Vᵢ
//HERE2;
		}
		else              /* else dodaj kolejny potencjał */
		{
HERE2;
			Vpsi_NOTstatic__ANYMORE[mask_id] +=pot->psiGlobalTable;  // ψᵥ: V = ∑Vᵢ
HERE2;
		}
	}

	for(auto& ipot : allInteractionPotentials) {
		//std::set<boost::tuple<boost::shared_ptr<QMStateDiscreteGlobal>,mask_t,mask_t> > allInteractionPotentials={};
		boost::shared_ptr<QMStateDiscreteGlobal> qms = ipot.get<0>();
		mask_t mask1 = ipot.get<1>();
		mask_t mask2 = ipot.get<2>();
		bool istnieje_1_w_liscie_obslugiwanych_funkcji_falowych = std::find( allChannelMasks.begin(), allChannelMasks.end(), mask1) != allChannelMasks.end();
		bool istnieje_2_w_liscie_obslugiwanych_funkcji_falowych = std::find( allChannelMasks.begin(), allChannelMasks.end(), mask2) != allChannelMasks.end();
		if(istnieje_1_w_liscie_obslugiwanych_funkcji_falowych and istnieje_2_w_liscie_obslugiwanych_funkcji_falowych) {
			std::cerr << "\nERROR: (allInteractionPotentials) oddziaływanie pomiędzy funkcjami falowymi nie moze mieć mask_t jednocześnie potencjału oddziałującego oraz funkcji falowej w liście f.falowych na które oddziałuje, to bez sensu.\n\n";
			exit(1);
		}
		if(not istnieje_1_w_liscie_obslugiwanych_funkcji_falowych and not istnieje_2_w_liscie_obslugiwanych_funkcji_falowych) {
			std::cerr << "\nERROR: (allInteractionPotentials) powienien być dokładnie jeden\n\n";
			exit(1);
		}
		if(istnieje_2_w_liscie_obslugiwanych_funkcji_falowych) {
			std::swap(mask1,mask2);
		}
		// OK, mam dokładnie jeden z nich w mask1.
		// - mask1 to maska funkcji falowej.
		// - mask2 to maska potencjału oddziałującego.
		// czyli mask2 to jest maska z jakimi funkcjami falowymi ma to działać.
		if(VpsiInteraction_NOTstatic__ANYMORE.find(mask1) != VpsiInteraction_NOTstatic__ANYMORE.end() ) {
			// jakieś dla tej funkcji falowej już istnieją
			if(VpsiInteraction_NOTstatic__ANYMORE[mask1].find(mask2) != VpsiInteraction_NOTstatic__ANYMORE[mask1].end() )
			{ // ten konkretny istnieje
				if(VpsiInteraction_NOTstatic__ANYMORE[mask1][mask2].rank() == 0) {
					// istnieje ale ma zerowy rozmiar, skąd więc się tam wziął?
					std::cerr << "\n// istnieje ale ma zerowy rozmiar, skąd więc się tam wziął?\n\n";
HERE2;
					exit(1);
HERE2;
				} else { // skoro istnieje, to robimy dodawanie.
					std::cerr << "\n==============================\nWARNING: adding several interaction potentials together. This is rarely intended behaviour.\n\n";
HERE2;
					VpsiInteraction_NOTstatic__ANYMORE[mask1][mask2] += qms->psiGlobalTable;
HERE2;
				}
			} else { // istnieją inne dla mask1, ale nie ten dla mask2, więc go wrzucamy
HERE2;
				VpsiInteraction_NOTstatic__ANYMORE[mask1][mask2] = qms->psiGlobalTable;
HERE2;
			}
		} else { // skoro nie istnieją żadne dla tej funkcji falowej to go wrzucamy.
//HERE2;
			VpsiInteraction_NOTstatic__ANYMORE[mask1][mask2] = qms->psiGlobalTable;
//HERE2;
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
//HERE2;
	return Vpsi_NOTstatic__ANYMORE[mask_id];
};
		
std::vector<NDimTable<Complexr> >	 SchrodingerKosloffPropagator::get_full_potentialCoupledInteractionGlobal_psiGlobalTable(size_t mask_id)
{
	std::vector<NDimTable<Complexr> > ret;
	ret.resize(allChannelMasks.size());
	for(size_t i = 0 ; i<allChannelMasks.size() ; i++) {
		if(i != mask_id) {
			// tylko pozadiagonalne elementy nas interesują
			if(VpsiInteraction_NOTstatic__ANYMORE.find(allChannelMasks[i]) != VpsiInteraction_NOTstatic__ANYMORE.end() )
			// tylko te które istnieją
			{
				for(std::map<mask_t,NDimTable<Complexr> >::iterator it = VpsiInteraction_NOTstatic__ANYMORE[allChannelMasks[i]].begin() ; it!=VpsiInteraction_NOTstatic__ANYMORE[allChannelMasks[i]].end() ; it++)
				{
					if( (it->first & allChannelMasks[i]) != 0) {
						// ale tylko te które faktycznie oddziałują z mask_id
						ret[i] = it->second;
					}
				}
			}
		}
	}
	return ret;
}

boost::shared_ptr<QMStateDiscreteGlobal> SchrodingerKosloffPropagator::get_full_psiGlobal__________________psiGlobalTable(size_t mask_id)
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

		// groupMask , kilka kanałów
		if(useGroupMasks and (not b->maskCompatible(allChannelMasks[mask_id])))
			continue;

		QMStateDiscrete* psiLocal=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(    psiLocal and psiLocal->isNumeric() and psiLocal->getPsiGlobalExists() )
			allPsiGlobals.insert(psiLocal->getPsiGlobalExisting());
	};
	if(allPsiGlobals.size() > 1) {
		std::cerr << "\n\nERROR: SchrodingerKosloffPropagator::get_full_psiGlobal__________________psiGlobalTable() two separate global wavefunctions!\n=========\n=========\n";
	//         I should perform whole separate SchrodingerKosloffPropagator integration for each of them!
		exit(1);
	}
	if(allPsiGlobals.size() == 0) {
		std::cerr << "\n\nERROR: SchrodingerKosloffPropagator::get_full_psiGlobal__________________psiGlobalTable() acts on nothing!\n=========\n=========\n";
		exit(1);
	}
	return *allPsiGlobals.begin();
};

// Virial theorem: 2*<E_kin> = -<E_pot> (dla wodoru), więcej mam w notatkach ołówkiem na str. 30 w "quantum olecular dynamics on grids"
// oraz coś w http://www.physicspages.com/2013/01/18/virial-theorem-in-3-d/
//
// generalnie chodzi o wzór, że <T>=0.5*b<V>, gdzie b to potęga x w potencjale
//
// wtedy ten grid check mógłby sprawdzać sensowność dobranej siatki, ale musiałby znać rodzaj potencjału

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

Real SchrodingerKosloffPropagator::eMinSelectedChannel(size_t idx)
{
//HERE2;
	NDimTable<Complexr> VGlobal( get_full_potentialInteractionGlobal_psiGlobalTable(idx) );
	Real ret = ((VGlobal.rank()!=0) ? (VGlobal.minReal()) : (0));
	std::vector<NDimTable<Complexr> > VGlobal_int( get_full_potentialCoupledInteractionGlobal_psiGlobalTable(idx) );
	for(size_t i = 0 ; i<VGlobal_int.size() ; i++) {
		if(VGlobal_int[i].rank() != 0) {
			ret -= std::abs( VGlobal_int[i].minReal() );
			ret -= std::abs( VGlobal_int[i].maxReal() );
		}
	}
	return ret;
};

Real SchrodingerKosloffPropagator::eKinSelectedChannel(size_t idx)
{
	Real ret(0); // assume that negative maximum energy is not possible
	// FIXME                                                                                ↓ ?  bez sensu, że w obu to się nazywa psiGlobalTable ....
	boost::shared_ptr<QMStateDiscreteGlobal> psiGlobal( get_full_psiGlobal__________________psiGlobalTable(idx) );
	if(psiGlobal) {
		int rank = psiGlobal->psiGlobalTable.rank();
		Real Ekin(0);
		for(int dim=0 ; dim<rank ; dim++)
			Ekin += std::pow(psiGlobal->kMax(dim)* 1/* FIXME: must be `hbar` here */,2)/(2 *FIXMEatomowe_MASS/*FIXME: must be mass here psi->m */);
		ret=std::max(ret, Ekin );
	}
	return ret;
}

Real SchrodingerKosloffPropagator::eMaxSelectedChannel(size_t idx)
{
	Real ret(eKinSelectedChannel(idx)); // assume that negative maximum energy is not possible
	// FIXME                                                                                ↓ ?  bez sensu, że w obu to się nazywa psiGlobalTable ....
HERE2;
	NDimTable<Complexr>     VGlobal( get_full_potentialInteractionGlobal_psiGlobalTable(idx) );
	ret += ((VGlobal.rank()!=0) ? (VGlobal.maxReal()) : (0));
	std::vector<NDimTable<Complexr> > VGlobal_int( get_full_potentialCoupledInteractionGlobal_psiGlobalTable(idx) );
	for(size_t i = 0 ; i<VGlobal_int.size() ; i++) {
		if(VGlobal_int[i].rank() != 0) {
			ret += std::abs( VGlobal_int[i].maxReal() );
			ret += std::abs( VGlobal_int[i].minReal() );
		}
	}
	return ret;
}

Real SchrodingerKosloffPropagator::eMin()
{
	Real local_eMin = eMinSelectedChannel(0);
	// OK, to teraz zbieramy minimum z pozostałych kanałów.
	for(size_t i = 1 ; i<allChannelMasks.size() ; i++ )
	{
		local_eMin = std::min(local_eMin , eMinSelectedChannel(i));
	}
HERE2;
	return local_eMin;
}

Real SchrodingerKosloffPropagator::eMax()
{
	Real local_eMax = eMaxSelectedChannel(0);
	// OK, to teraz zbieramy maximum z pozostałych kanałów.
	for(size_t i = 1 ; i<allChannelMasks.size() ; i++ )
	{
		local_eMax = std::max(local_eMax , eMaxSelectedChannel(i));
	}
	return local_eMax;
}

void SchrodingerKosloffPropagator::calc_Hnorm_psi(
	  const NDimTable<Complexr>& psi_0
	,       NDimTable<Complexr>& psi_1
	, /*FIXME - remove*/QMStateDiscrete* psi
	, size_t mask_id
	, const std::vector<NDimTable<Complexr> >& all_psi_0
)
{
//delay.printDelay("calc_Hnorm_psi");	//	1: 1.7s		2: 0s		3: 0s		4: 0s		5: 0s		6: 0s		7: 0s
	Real mass(FIXMEatomowe_MASS); // FIXME - this shouldn't be here
	Real dt=scene->dt;

	Real R   = calcKosloffR(dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(dt); // FIXME -  that's duplicate here, depends on dt !!

	// FIXME,FIXME ↓ -- this should be somewhere else. Still need to find a good place.
	// FIXME,FIXME
	// FIXME,FIXME      LOL, po tym jak Domsta zrobił tą inną metodę liczenia drugiej pochodnej, to ewidentnie należy do osobnej klasy, tak żeby można było wybierać!!
	// FIXME,FIXME
	//static bool hasTable(false);
HERE;
	//static NDimTable<Real    > kTable(psi_0.dim());
	if(! haskTable_NOTstatic__ANYMORE[mask_id]){
//delay.printDelay("make kTable");	//	1: 24.2s
		haskTable_NOTstatic__ANYMORE[mask_id]=true;
		kTable_NOTstatic__ANYMORE[mask_id].resize(psi_0.dim());
		// FIXME!!!!!  wow, gdy będę miał dobry warunek na krok siatki, to będę mógł automatycznie zagęszczać i rozluźniać siatkę!
		//             wystarczy że nowa siatka będzie interpolowana ze starej siatki, i co jakiś czas będzie automatycznie skalowana
		kTable_NOTstatic__ANYMORE[mask_id].becomeMinusKSquaredTable( [&psi](Real i,int d)->Real{ return psi->iToK(i,d);});
//? ← zakomentowane to oznacza, że mnożenie przez -k² odbywa się na "złej" wersji przestrzeni odwrotnej |         ,-.         | vs. |-.                  ,|
//    ale o dziwno wszystkie wyniki są dobre, pytanie: dlaczego?                                        |________/___\________| vs. |__\________________/_|
//? kTable.shiftByHalf();
	}
	// FIXME,FIXME ↑ -- this should be somewhere else. Still need to find a good place.
	
	// prepare the potential  ψᵥ
HERE;
//delay.printDelay("copy Vpsi _________");//	1: 3.4s		2: 3.2s		3: 15.4s	4: 3.6s		5: 3.3s		6: 3.3s		7: 3.1s
//	static NDimTable<Complexr> Vpsi = {}; // static to avoid creating and destroying, just copy
//std::cerr << "\n calc_Hnorm_psi   useGroupThisMask="<<useGroupThisMask<<"\n";

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

	       NDimTable<Complexr> Vpsi = get_full_potentialInteractionGlobal_psiGlobalTable(mask_id);  // just copy (żebym mógł przemnażać przez psi itp)

		// to jest tablica o rozmiarze allChannelMasks.size(), zawiera potencjały przez które nalezy przemnożyć pozostałe składowe all_psi_0
		// muszę zrobić kopię aby możliwe było ich przemnażanie.
		std::vector<NDimTable<Complexr> > V12_21_etc = get_full_potentialCoupledInteractionGlobal_psiGlobalTable(mask_id);
		for(size_t i = 0 ; i<allChannelMasks.size() ; i++) {
			if(V12_21_etc[i].rank() != 0) {
				V12_21_etc[i] .multMult(all_psi_0[i],dt/(FIXMEatomowe_hbar*R));  // liczę   (dt Vⱼₖψₖ)/(ℏ R)
			}
		}
	
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
	psi_1 *= kTable_NOTstatic__ANYMORE[mask_id];                     // ψ₁: ψ₁=           -k²ℱ(ψ₀)
//? psi_1.shiftByHalf();
//delay.printDelay("IFFT");		//	1: 3.8s		2: 4.0s		3: 4.4s		4: 4.1s		5: 3.8s		6: 4.0s		7: 3.9s
	psi_1   .IFFT();                     // ψ₁: ψ₁=       ℱ⁻¹(-k²ℱ(ψ₀))
//? psi_1.shiftByHalf();
//delay.printDelay("mult,etc");		//	1: 1.0s		2: 0.8s		3: 1.25s	4: 1.0s		5: 0.9s		6: 1.2s		7: 1.0s
	psi_1 *= dt*FIXMEatomowe_hbar/(R*2*mass);         // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	psi_1   .mult2Add(psi_0,(1+G/R));    // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀

	if(Vpsi.rank() != 0)
		psi_1 -= Vpsi;               // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)

		for(size_t i = 0 ; i<allChannelMasks.size() ; i++) {
			if(V12_21_etc[i].rank() != 0) {
				psi_1 -= V12_21_etc[i];  // liczę   (dt)/(ℏ R) * ΣVⱼₖψₖ
			}
		}
//
//
//
// UWAGA: to chyba powyżej jakoś zniwelowałem że wywołuję J_k(R) zamiast J_k(-R), zmieniłem znaki w formule rekurencyjnej na tworzenie T_k:
// 
//        mam +(1+G/R)-V     a powinienem mieć
//            -(1+G/R)+V
//
//        chyba
//

//delay.printDelay("mult,etc end");	//	1: 0.7s		2: 10.8s	3: 1.1s		4: 0.9s		5: 0.9s		6: 1.2s		7: 0.9s

	// FIXME: return std::move(psi_1);
}

Real SchrodingerKosloffPropagator::calcKosloffR(Real dt)
{ // calculate R parameter in Kosloff method
// use last_R whenever possible
	if(last_dtR != dt) { kosloffR_needs_Update = true; };
	if(kosloffR_needs_Update) {
//
// FIXME: ojojoj, eMax() eMin() może mi się zmieniać :( nie tylko jak się zmieni dt, ale jak się zmieni funkcja falowa w drugim kanale
//        muszę to uwzględnić, na razie nie wiem jak.
//
		last_dtR = dt;
		kosloffR_needs_Update = false;
		last_R = dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);
	}
	return last_R;

/*
	static Real last_dt(dt);
	static Real ret_R( dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar) );

	if(dt != last_dt)
	{
		last_dt=dt;
		ret_R  = dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);
	}

	return ret_R; // dt*(eMax() - eMin())/(2*FIXMEatomowe_hbar);
*/
};
Real SchrodingerKosloffPropagator::calcKosloffG(Real dt)
{ // calculate G parameter in Kosloff method
// use last_G whenever possible
	if(last_dtG != dt) { kosloffG_needs_Update = true; };
	if(kosloffG_needs_Update) {
//
// FIXME: ojojoj, eMax() eMin() może mi się zmieniać :( nie tylko jak się zmieni dt, ale jak się zmieni funkcja falowa w drugim kanale
//        muszę to uwzględnić, na razie nie wiem jak.
//
		last_dtG = dt;
		kosloffG_needs_Update = false;
		last_G = dt*eMin()/(2*FIXMEatomowe_hbar);
	}
	return last_G;

/*
	static Real last_dt(dt);
	static Real ret_G( dt*eMin()/(2*FIXMEatomowe_hbar) );
	
	if(dt != last_dt)
	{
		last_dt=dt;
		ret_G  = dt*eMin()/(2*FIXMEatomowe_hbar);
	}

	return ret_G; // dt*eMin()/(2*FIXMEatomowe_hbar);
*/
};


void SchrodingerKosloffPropagator::findAllEligibleGroupMasks()
{
	// na niektórych kanałach może być sama funkcja falowa bez potencjału
	// A jeśli jest potencjał bez funkcji falowej to ignorujemy taki kanał.
	FOREACH(const shared_ptr<Body>& b , *scene->bodies){
		// groupMask , kilka kanałów
		if(useGroupMasks and (not b->maskCompatible(useGroupTheseMasks)))
			continue;

		QMStateDiscrete* psiLocal=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(    psiLocal and psiLocal->isNumeric() and psiLocal->getPsiGlobalExists() )
			allChannelMasks.push_back(b->groupMask);
	};
	std::cerr << "SchrodingerKosloffPropagator::findAllEligibleGroupMasks() found masks:\n";
	for(size_t i=0 ; i< allChannelMasks.size() ; i++) std::cerr << " → mask: " << allChannelMasks[i] << "\n";
}

void SchrodingerKosloffPropagator::action()
{
	// only once, find all channels of interest.
	if(allChannelMasks.size() == 0) {
		findAllEligibleGroupMasks();
		kTable_NOTstatic__ANYMORE   .resize(allChannelMasks.size());
		haskTable_NOTstatic__ANYMORE.resize(allChannelMasks.size(),false);
		dTable_NOTstatic__ANYMORE   .resize(allChannelMasks.size());
		hasdTable_NOTstatic__ANYMORE.resize(allChannelMasks.size(),false);
	}
	if(allChannelMasks.size() == 0) {
		std::cerr << "\n\nERROR: SchrodingerKosloffPropagator::action() has nothing to wrok with\n\n";
		exit(1);
	}

//	__gnu_parallel::_Settings s;
//	s.algorithm_strategy = __gnu_parallel::force_parallel;
//	__gnu_parallel::_Settings::set(s);
//	omp_set_dynamic(false);
//	omp_set_num_threads(threadNum);
	

	//virialTheorem_Grid_check(); // FIXME - to powinno być chyba zależne od potencjału...
	timeLimit.readWallClock();
	//static int maxIter(0);
	Real R   = calcKosloffR(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real G   = calcKosloffG(scene->dt); // FIXME -  that's duplicate here, depends on dt !!
	Real R13 = 1.3*R;
	Real min = 10000.0*std::numeric_limits<Real>::min(); // get the numeric minimum, smallest number. To compare if anything is smaller than it, this one must be larger.

	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
// FIXME - for multiple entangled wavefunctions


//	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//		size_t channel_mask_id = 0;
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

	std::vector<boost::shared_ptr<QMStateDiscreteGlobal> > psiGlobal;
	psiGlobal.resize(allChannelMasks.size());

	for(size_t channel_mask_id = 0 ; channel_mask_id < allChannelMasks.size() ; channel_mask_id++) {
		// szykuję dTable dla wszystkich kanałów.
		psiGlobal[channel_mask_id] = ( get_full_psiGlobal__________________psiGlobalTable(channel_mask_id) );
//		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());

		//static bool hasDampTable(false);  /////////////////// DAMPING !!!! ABC
		//static NDimTable<Real    > dTable(psiGlobal->psiGlobalTable.dim()); /////////////////// DAMPING !!!! ABC
		if((not hasdTable_NOTstatic__ANYMORE[channel_mask_id]) or (hasDampTableRegen)){
			hasdTable_NOTstatic__ANYMORE[channel_mask_id]=true;
			dTable_NOTstatic__ANYMORE[channel_mask_id].resize(psiGlobal[channel_mask_id]->psiGlobalTable.dim());
			hasDampTableRegen = false;
			if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { /////////////////// DAMPING !!!! ABC
				std::cerr << "\nWARNING ---- dampMarginBandMin and dampMarginBandMax are using only X size of mesh\n";
				if(not dampExponentUseLR) {
					dampExponentLeft = dampExponent;
					dampExponentRight= dampExponent;
				}
				dTable_NOTstatic__ANYMORE[channel_mask_id].becomeDampingTable(dampMarginBandMin,dampMarginBandMax,dampExponentLeft,dampExponentRight,dampFormulaSmooth
				, [&](Real i, int d)->Real    { return psiGlobal[channel_mask_id]->iToX(i,d);}
				, [&](int d        )->Real    { return psiGlobal[channel_mask_id]->start(d);}
				); // FIXME - powinien być std::vector<Real> dampMarginBandMin,dampMarginBandMax; tzn. osobne dla każdego wymiaru
				if(dampDebugPrint) {
					std::cout << std::setprecision(std::numeric_limits<double>::digits10+1);
					dTable_NOTstatic__ANYMORE[channel_mask_id].print();
				}
				// FIXME - rysowany jest tylko damping z ostatniego kanału
				global_dTable     = dTable_NOTstatic__ANYMORE[channel_mask_id];
				global_psiGlobal  = psiGlobal[channel_mask_id];
				hasDampTableCheck = true;
			};
		}
	}
	std::vector<NDimTable<Complexr> > psi_dt; psi_dt.resize(allChannelMasks.size());
	std::vector<NDimTable<Complexr> > psi_0 ; psi_0 .resize(allChannelMasks.size());
	std::vector<NDimTable<Complexr> > psi_1 ; psi_1 .resize(allChannelMasks.size());
	std::vector<NDimTable<Complexr> > psi_2 ; psi_2 .resize(allChannelMasks.size());
	if(psiGlobal[0] and doCopyTable) {
		for(size_t channel_mask_id = 0 ; channel_mask_id < allChannelMasks.size() ; channel_mask_id++) {
		// teraz muszę przygotować wszystkie kanały
	////   ↓↓↓       FIXME: this is          ↓ only because with & it draws the middle of calculations
HERE;
			/*NDimTable<Complexr>*//*&*/ psi_dt[channel_mask_id]=(psiGlobal[channel_mask_id]->psiGlobalTable); // will become ψ(t+dt): ψ(t+dt) = ψ₀
			/*NDimTable<Complexr>*/  psi_0[channel_mask_id] = psi_dt[channel_mask_id];            // ψ₀
HERE;
			/*NDimTable<Complexr>  psi_1 = {};*/                // ψ₁
			/*NDimTable<Complexr>  psi_2 = {};*/                // ψ₂     :
		}
		for(size_t channel_mask_id = 0 ; channel_mask_id < allChannelMasks.size() ; channel_mask_id++) {
			calc_Hnorm_psi(psi_0[channel_mask_id],psi_1[channel_mask_id],psiGlobal[channel_mask_id].get(), channel_mask_id , psi_0);    // ψ₁     : ψ₁     =(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
/* ?? */		if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { // ABC damping, Mandelshtam,Taylor 'Spectral projection approach to the quantum scattering calculations'
/* ?? */			psi_1[channel_mask_id] *= dTable_NOTstatic__ANYMORE[channel_mask_id]; };                     // ψ₁     : ψ₁     = e⁻ˠψ₁
//						1: 0.7s
			Complexr ak0=calcAK(0,R);                       // a₀
			Complexr ak1=calcAK(1,R);                       // a₁
			psi_dt[channel_mask_id] .mult1Mult2Add(ak0, psi_1[channel_mask_id],ak1);          // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
		}
		int i(0);
		Complexr ak(1);
		// never stop when i < R*1.3, unless steps is positive. Auto stop expanding series based on std::numeric_limits<Real>::min()
		for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
		{
			for(size_t channel_mask_id = 0 ; channel_mask_id < allChannelMasks.size() ; channel_mask_id++) {
				if(printIter!=0 and ((i%printIter) ==0)) std::cerr << ":::::: SchrodingerKosloffPropagator iter = " << i << " ak="<<ak<<"\n";
//						1: dotąd 0.7s
HERE;
				calc_Hnorm_psi(psi_1[channel_mask_id],psi_2[channel_mask_id],psiGlobal[channel_mask_id].get() , channel_mask_id, psi_1);      // ψ₂     : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
//1:2: 10.8s 3: 1.1s 4: 0.9s 5: 0.9s 6: 1.2s
/* ?? */			if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { // ABC damping, Mandelshtam,Taylor 'Spectral projection approach to the quantum scattering calculations'
/* ?? */				psi_0[channel_mask_id] *= dTable_NOTstatic__ANYMORE[channel_mask_id]; };                       // ψ₀     : ψ₀     = e⁻ˠψ₀
				psi_2[channel_mask_id]  .mult1Sub(2,psi_0[channel_mask_id]);                        // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
/* ?? */			if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { // ABC damping, Mandelshtam,Taylor 'Spectral projection approach to the quantum scattering calculations'
/* ?? */				psi_2[channel_mask_id] *= dTable_NOTstatic__ANYMORE[channel_mask_id]; };                       // ψ₂     : ψ₂     = e⁻ˠψ₂
			}
			for(size_t channel_mask_id = 0 ; channel_mask_id < allChannelMasks.size() ; channel_mask_id++) {
				psi_dt[channel_mask_id] .mult2Add(psi_2[channel_mask_id],ak=calcAK(i,R));           // ψ(t+dt): ψ(t+dt)= ψ(t+dt) + aₖψₖ
//delay.printDelay("mult2 etc end");//1: 2: 0s 3: 0s 4: 0s 5: 0s 6: 0s
				//psi_0=std::move(psi_1);                         // ψ₀ ← ψ₁
				//psi_1=std::move(psi_2);                         // ψ₁ ← ψ₂
				std::swap(psi_0[channel_mask_id],psi_1[channel_mask_id]);
				std::swap(psi_1[channel_mask_id],psi_2[channel_mask_id]);
//delay.printDelay("swap end");     //1: 2: 0s 3: 0s 4: 0s 5: 0s 6: 0s
			}
		}
		for(size_t channel_mask_id = 0 ; channel_mask_id < allChannelMasks.size() ; channel_mask_id++) {
		// na koniec mogę wszystkie wynikowe sumy przemnożyć
			psi_dt[channel_mask_id] *= std::exp(-1.0*Mathr::I*(R+G));        // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)

			// FIXME: this is    ↓ only because with & it draws the middle of calculations
	/*//	↑↑↑*/	psiGlobal[channel_mask_id]->psiGlobalTable = psi_dt[channel_mask_id];
			// FIXME: but it actually wastes twice more memory

			if(timeLimit.messageAllowed(4) or printIter!=0) std::cerr << "(not &) final |ak|=" << boost::lexical_cast<std::string>(std::abs(std::real(ak))+std::abs(std::imag(ak))) << " iterations: " << i-1 << "/" << steps << "\n";
			if(timeLimit.messageAllowed(6)) std::cerr << "Muszę wywalić hbar ze SchrodingerKosloffPropagator i używać to co jest w QMIPhys, lub obok.\n";
//std::cerr << "SchrodingerKosloffPropagator t+=dt (calculating) " << b->getId() << "\n";
	// FIXME - full duplicate of the block above, except for the reference
		}
	} else if(psiGlobal[0] and not doCopyTable) {
/*.. */  std::cerr << "\n\n\n ERROR: doCopyTable == false   is NOT IMPLEMENTED NOW ! bo robiłem obliczenia wielokanałowe i nie chciałem dublować wszystkiego!\n\n";
/*.. */  exit(1);
//.. //  			NDimTable<Complexr>& psi_dt(psiGlobal->psiGlobalTable); // will become ψ(t+dt): ψ(t+dt) = ψ₀
//.. //  			NDimTable<Complexr>  psi_0 (psi_dt);            // ψ₀
//.. //  			NDimTable<Complexr>  psi_1 = {};                // ψ₁
//.. //  			NDimTable<Complexr>  psi_2 = {};                // ψ₂     :
//.. //  			calc_Hnorm_psi(psi_0,psi_1,psiGlobal.get(), channel_mask_id);    // ψ₁     : ψ₁     =(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
//.. //  /* ?? */		if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { // ABC damping, Mandelshtam,Taylor 'Spectral projection approach to the quantum scattering calculations'
//.. //  /* ?? */			psi_1 *= dTable_NOTstatic__ANYMORE[channel_mask_id]; };                     // ψ₁     : ψ₁     = e⁻ˠψ₁
//.. //  			Complexr ak0=calcAK(0,R);                       // a₀
//.. //  			Complexr ak1=calcAK(1,R);                       // a₁
//.. //  			psi_dt .mult1Mult2Add(ak0, psi_1,ak1);          // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
//.. //  			int i(0);
//.. //  			Complexr ak(1);
//.. //  			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
//.. //  			{
//.. //  				if(printIter!=0 and ((i%printIter) ==0)) std::cerr << ":::::: SchrodingerKosloffPropagator O.iter="<<(scene->iter)<<", loop iter=" << i << "/" << maxIter_NOTstatic__ANYMORE << " ak="<<ak<<"\n";
//.. //  				calc_Hnorm_psi(psi_1,psi_2,psiGlobal.get(), channel_mask_id);//ψ₂  : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
//.. //  /* ?? */			if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { // ABC damping, Mandelshtam,Taylor 'Spectral projection approach to the quantum scattering calculations'
//.. //  /* ?? */				psi_0 *= dTable_NOTstatic__ANYMORE[channel_mask_id]; };                       // ψ₀     : ψ₀     = e⁻ˠψ₀
//.. //  				psi_2  .mult1Sub(2,psi_0);              // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
//.. //  /* ?? */			if(dampMarginBandMin > 0 and dampMarginBandMax > 0) { // ABC damping, Mandelshtam,Taylor 'Spectral projection approach to the quantum scattering calculations'
//.. //  /* ?? */				psi_2 *= dTable_NOTstatic__ANYMORE[channel_mask_id]; };                       // ψ₂     : ψ₂     = e⁻ˠψ₂
//.. //  				psi_dt .mult2Add(psi_2,ak=calcAK(i,R)); // ψ(t+dt): ψ(t+dt)=ψ(t+dt) + aₖψₖ
//.. //  //delay.printDelay("mult2 etc end");
//.. //  				//psi_0=std::move(psi_1);                 // ψ₀ ← ψ₁
//.. //  				//psi_1=std::move(psi_2);                 // ψ₁ ← ψ₂
//.. //  				std::swap(psi_0,psi_1);
//.. //  				std::swap(psi_1,psi_2);
//.. //  //delay.printDelay("swap end");
//.. //  			}
//.. //  			psi_dt *= std::exp(-1.0*Mathr::I*(R+G));        // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)
//.. //  			if(timeLimit.messageAllowed(4) or printIter!=0) std::cerr << "(use &) final |ak|=" << boost::lexical_cast<std::string>(std::abs(std::real(ak))+std::abs(std::imag(ak))) << " iterations: " << i-1 << "/" << steps << "\n";
//.. //  			maxIter_NOTstatic__ANYMORE = std::max(i-1,maxIter_NOTstatic__ANYMORE);
//.. //  			if(timeLimit.messageAllowed(6)) std::cerr << "Muszę wywalić hbar ze SchrodingerKosloffPropagator i używać to co jest w QMIPhys, lub obok.\n";
//.. //  
		}
// FIXME - for multiple entangled wavefunctions
//	} YADE_PARALLEL_FOREACH_BODY_END();

//	}

//	omp_set_num_threads(1);
}




/* simplified main loop

NDimTable<Complexr>& psi_dt(psiGlobal->psiGlobalTable); // will become ψ(t+dt): ψ(t+dt) = ψ₀
NDimTable<Complexr>  psi_0 (psi_dt);                    // ψ₀
NDimTable<Complexr>  psi_1 = {};                        // ψ₁
NDimTable<Complexr>  psi_2 = {};                        // ψ₂
calc_Hnorm_psi(psi_0,psi_1,psiGlobal.get());            // ψ₁     : ψ₁     =     (1+G/R)ψ₀+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) - (dt V ψ₀)/(ℏ R)
Complexr ak0=calcAK(0,R);                               // a₀
Complexr ak1=calcAK(1,R);                               // a₁
psi_dt .mult1Mult2Add(ak0, psi_1,ak1);                  // ψ(t+dt): ψ(t+dt)=a₀ψ₀+a₁ψ₁
int i(0);
Complexr ak(1);
for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
{
	calc_Hnorm_psi(psi_1,psi_2,psiGlobal.get());    // ψ₂     : ψ₂     =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) - (dt V ψ₁)/(ℏ R)
	psi_2  .mult1Sub(2,psi_0);                      // ψ₂     : ψ₂     = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) - (dt V ψ₁)/(ℏ R) ) - ψ₀
	psi_dt .mult2Add(psi_2,ak=calcAK(i,R));         // ψ(t+dt): ψ(t+dt)=ψ(t+dt) + aₖψₖ
	std::swap(psi_0,psi_1);                         // ψ₀ ← ψ₁
	std::swap(psi_1,psi_2);                         // ψ₁ ← ψ₂
}
psi_dt *= std::exp(-1.0*Mathr::I*(R+G));                // ψ(t+dt): ψ(t+dt)=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)

*/
/*

void SchrodingerKosloffPropagator::calc_Hnorm_psi(
	  const NDimTable<Complexr>& psi_0
	, NDimTable<Complexr>& psi_1,
	, QMStateDiscrete* psi)
{
	Real mass = psi->getMass();
	Real dt   = scene->dt;

	NDimTable<Real    >& kTable = psi->getkTable();
	NDimTable<Complexr>& Vpsi = psi->getPotential();
	
// ψᵥ: ψᵥ=(dt V ψ₀)/(ℏ R)
	if(Vpsi.rank() != 0)
		Vpsi    .multMult(psi_0,dt/(FIXMEatomowe_hbar*R));

// use all available processors in FFT calculations
	psi_1.set_num_threads(threadNum);
	psi_1.doFFT(psi_0,true);                 // ψ₁: ψ₁=              ℱ(ψ₀)
	psi_1 *= kTable;                     // ψ₁: ψ₁=           -k²ℱ(ψ₀)
	psi_1   .IFFT();                     // ψ₁: ψ₁=       ℱ⁻¹(-k²ℱ(ψ₀))
	psi_1 *= dt*hbar/(R*2*mass);         // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	psi_1   .mult2Add(psi_0,(1+G/R));    // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀

	if(Vpsi.rank() != 0)
		psi_1 -= Vpsi;               // ψ₁: ψ₁=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m) + (1+G/R)ψ₀ - (dt V ψ₀)/(ℏ R)
}
*/

#include "Gl1_QMGeometry.hpp"

// FIXME - ten tutaj to już jest prawdziwa bezsensowna proteza :( Na pewno w wielu przypadkach nie działa

CREATE_LOGGER(GlExtra_QMEngine);

void GlExtra_QMEngine::render(){
	// scene object changed (after reload, for instance), for re-initialization
	if(qmEngine and qmEngine->scene!=scene) qmEngine=boost::shared_ptr<SchrodingerKosloffPropagator>();

	if(!qmEngine){ FOREACH(boost::shared_ptr<Engine> e, scene->engines){ qmEngine=YADE_PTR_DYN_CAST<SchrodingerKosloffPropagator>(e); if(qmEngine) break; } }
	if(!qmEngine){ LOG_ERROR("No SchrodingerKosloffPropagator in O.engines, killing myself."); dead=true; return; }

	if(drawDTable and qmEngine->hasDampTableCheck) { // we are drawing dTable, because it was generated
		//std::cerr << "must draw here !\n";
		shared_ptr<QMDisplayOptions> curOpt=YADE_PTR_DYN_CAST<QMDisplayOptions>(dampDisplayOptions);
		if(curOpt) {

			curOpt->step=Vector3r(0,0,0);
			for(int i=0 ; i < std::min(3,(int)qmEngine->global_dTable.rank()) ; i++) {
				curOpt->renderGlobal_i[ i         ] = (int)(i);
				curOpt->renderAxis_i  [ i         ] = (i % 3 ); // we will have to draw along this axis: 0,1,2 = x,y,z
				curOpt->renderSize    [ i         ] = qmEngine->global_psiGlobal->getSpatialSizeGlobal()[i]; // it will have this size

				curOpt->step [i]=qmEngine->global_psiGlobal->getSpatialSizeGlobal()[curOpt->renderGlobal_i[i]]/((Real)(qmEngine->global_psiGlobal->gridSize[curOpt->renderGlobal_i[i]]));
				curOpt->start[i]=qmEngine->global_psiGlobal->start                 (curOpt->renderGlobal_i[i]);                  // FIXME? or not? problem is that N-nodes have (N-1) lines between: |---|---|---|---|---
				curOpt->end  [i]=qmEngine->global_psiGlobal->end                   (curOpt->renderGlobal_i[i])- curOpt->step[i]; // maybe change the start() and end() values in QMStateDiscrete??   ¹ 1 ² 2 ³ 3 ⁴ 4 ⁵ 5
			}

			GLViewInfo glinfo_ignored;
			Gl1_NDimTable worker;
			worker.drawNDimTable(
				// curOpt->marginalDistribution
				 qmEngine->global_dTable
				,curOpt,dampColor,dampForceWire,glinfo_ignored);
		} else {
			std::cerr << "problem with drawing damping, no dampDisplayOptions defined!\n";
		}
	}
};

