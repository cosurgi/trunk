// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotentialCoulomb.hpp"

#include <lib/time/TimeLimit.hpp>
TimeLimit timeLimitC; // FIXME - remove all timeLimit when finshed fixing

YADE_PLUGIN( // FIXME - lots of duplicated code here...
	(QMParametersCoulomb)
	(QMParticleCoulomb)
	(QMStPotentialCoulomb)
	(St1_QMStPotentialCoulomb)
	(QMIPhysCoulomb)
	(QMIPhysCoulombParticles)
	(QMIPhysCoulombParticleInPotential)
	(Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb)
	(Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles)
	(Law2_QMIGeom_QMIPhysCoulomb)
	(Law2_QMIGeom_QMIPhysCoulombParticles)
	(Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles)
	(Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential)
	(Law2_QMIGeom_QMIPhysCoulombParticlesFree)
	(Law2_QMIGeom_QMIPhysCoulombParticleInPotential)
	);

/*********************************************************************************
*
* Q M   P O T E N T I A L   H A R M O N I C   parameters      QMParametersCoulomb
*
*********************************************************************************/
CREATE_LOGGER(QMParametersCoulomb);
// !! at least one virtual function in the .cpp file
QMParametersCoulomb::~QMParametersCoulomb(){};

/*********************************************************************************
*
* Q M   P A R T I C L E   with   H A R M O N I C   ,,charge''   QMParticleCoulomb
*
*********************************************************************************/
CREATE_LOGGER(QMParticleCoulomb);
// !! at least one virtual function in the .cpp file
QMParticleCoulomb::~QMParticleCoulomb(){};

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C                         QMStPotentialCoulomb
*
*********************************************************************************/
CREATE_LOGGER(QMStPotentialCoulomb);
// !! at least one virtual function in the .cpp file
QMStPotentialCoulomb::~QMStPotentialCoulomb(){};

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C   F U N C T O R         QMStPotentialCoulomb
*
*********************************************************************************/
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME - zrobić diff z QMPotentialHarmonic 
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME - zrobić diff z QMPotentialHarmonic 
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME - zrobić diff z QMPotentialHarmonic 
Complexr St1_QMStPotentialCoulomb::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	//const QMStPotentialCoulomb*        state = dynamic_cast <const QMStPotentialCoulomb*>(qms);
	const QMParametersCoulomb* harmonic = dynamic_cast<const QMParametersCoulomb*>(pm);
	if(not harmonic) { throw std::runtime_error("\n\nERROR: St1_QMStPotentialCoulomb nas no QMParametersCoulomb, but rather `"
		+std::string(pm?pm->getClassName():"")+"`.\n\n");};
	Real x =pos[0]                  ,y =pos[1]                  ,z =pos[2];
	Real Cx=harmonic->coefficient[0];//,Cy=harmonic->coefficient[1],Cz=harmonic->coefficient[2];
	Real MAXinv=abs(1.0/harmonic->potentialMaximum); // FIXME musiałem tak zrobić, ponieważ gdy próbowałem w 1d-coulomb.py wpisać 
	                     //       `potentialCenter   = [ 0.01 ,0  ,0  ]` to powodowało to przesunięcie całeś siatki, i owszem - liczyło się, ponieważ
			     //        węzły siatki potencjału rozmijały się z węzłami cząstki, więc nie było dzielenia przez zero, ale w samej siatce
			     //        i tak było dzielenie przez zero (chyba?!?!) bo to cała siatka była przesunięta - chwycona za środek i przesunięta
	Real r  =0;
	auto och=[&](Real r){std::cerr << "Ochrona przed dzieleniem przez zero! r="<<r<<", potentialMaximum"<< harmonic->potentialMaximum << ", MAXinv" << MAXinv <<"\n";};
	switch(harmonic->dim) {
// FIXME: it's only for display, so this should go to Gl1_QMIGeom or Gl1_QMIGeomHarmonic (?) or Gl1_QMIPhys or Gl1_QMIPhysCoulomb
//        but then - the potential itself shall be drawn just like before: as a Box ??
	//	case 1 : return Cx/std::abs(x);            //if(r>MAXinv) return Cx/r; else return (harmonic->potentialMaximum>0)?(Cx/MAXinv):0;
	//	case 2 : return Cx/std::sqrt(x*x+y*y);     //if(r>MAXinv) return Cx/r; else return (harmonic->potentialMaximum>0)?(Cx/MAXinv):0;
	//	case 3 : return Cx/std::sqrt(x*x+y*y+z*z); //if(r>MAXinv) return Cx/r; else return (harmonic->potentialMaximum>0)?(Cx/MAXinv):0;
		case 1 : r=std::abs(x);            if(r>MAXinv) return Cx/r; else { och(r); return (harmonic->potentialMaximum>0)?(Cx/MAXinv):0; };
		case 2 : r=std::sqrt(x*x+y*y);     if(r>MAXinv) return Cx/r; else { och(r); return (harmonic->potentialMaximum>0)?(Cx/MAXinv):0; };
		case 3 : r=std::sqrt(x*x+y*y+z*z); if(r>MAXinv) return Cx/r; else { och(r); return (harmonic->potentialMaximum>0)?(Cx/MAXinv):0; };
		default: break;
	};
	throw std::runtime_error("\n\nSt1_QMStPotentialCoulomb::getValPos - wrong number of dimensions.\n\n");
};

/*********************************************************************************
*
* Q M   interaction   H A R M O N I C   P O T E N T I A L          QMIPhysCoulomb
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysCoulomb);
// !! at least one virtual function in the .cpp file
QMIPhysCoulomb::~QMIPhysCoulomb(){};

/*********************************************************************************
*
* Q M particle interaction via harmonic potential         QMIPhysCoulombParticles
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysCoulombParticles);
// !! at least one virtual function in the .cpp file
QMIPhysCoulombParticles::~QMIPhysCoulombParticles(){};

/*********************************************************************************
*
* Q M particle interaction via harmonic potential         QMIPhysCoulombParticleInPotential
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysCoulombParticleInPotential);
// !! at least one virtual function in the .cpp file
QMIPhysCoulombParticleInPotential::~QMIPhysCoulombParticleInPotential(){};

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersCoulomb  →  QMIPhysCoulomb
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb);

void Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitC.messageAllowed(12)) std::cerr << "####### Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::go  START!\n";

	shared_ptr<QMIPhysCoulomb> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysCoulomb>(I->phys);
	else { pot=shared_ptr<QMIPhysCoulomb>(new QMIPhysCoulomb()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	//const QMParameters*        qm1 = dynamic_cast<QMParameters*       >(m1.get());
	const QMParametersCoulomb* qm2 = dynamic_cast<QMParametersCoulomb*>(m2.get());
	if(not qm2) {
		HERE_ERROR("no QMParametersCoulomb found");
		exit(1);
	}

	pot->coefficient = qm2->coefficient;
	pot->potentialMaximum= qm2->potentialMaximum;
}

void Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	std::cerr << "\n\n####### Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::goReverse <----------------- START!\n\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Ip2   QMParticleCoulomb   QMParticleCoulomb  →  QMIPhysCoulombParticles
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles);

void Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitC.messageAllowed(12)) std::cerr << "####### Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles::go  START!\n";

	shared_ptr<QMIPhysCoulombParticles> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysCoulombParticles>(I->phys);
	else { pot=shared_ptr<QMIPhysCoulombParticles>(new QMIPhysCoulombParticles()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	const QMParticleCoulomb* qm1 = dynamic_cast<QMParticleCoulomb*>(m1.get());
	const QMParticleCoulomb* qm2 = dynamic_cast<QMParticleCoulomb*>(m2.get());
	if(not qm1 or not qm2) {
		HERE_ERROR("missing qm1, or qm2");
		exit(1);
	};

	pot->coefficient1 = qm1->coefficient;
	pot->coefficient2 = qm2->coefficient;

// FIXME: create here QMIPhys::potentialInteractionGlobal
//        then call 
//		Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);
//        to assign (or calcMarginalDistribution??? ← no, better leave that for Gl1_drawer), so I don't actually need to assign!
//		harmonic->potentialInteractionGlobal->psiGlobalTable = val;

}

void Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	std::cerr << "\n\n####### Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::goReverse <-------------------- START!\n\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulomb  :  H A R M O N I C   P O T E N T I A L
*
*********************************************************************************/

CREATE_LOGGER(Law2_QMIGeom_QMIPhysCoulomb);

bool Law2_QMIGeom_QMIPhysCoulomb::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
{

	QMIGeom*         qmigeom  = dynamic_cast<QMIGeom*        >(ig.get());
	QMIPhysCoulomb* harmonic = dynamic_cast<QMIPhysCoulomb*>(ip.get());
	if(not qmigeom or not harmonic) {
		HERE_ERROR("not qmigeom or not harmonic");
		exit(1);
	};

	// FIXME, but how?? I need this equation somehow.
	QMParametersCoulomb FIXME_param;
	FIXME_param.dim=harmonic->dim; FIXME_param.hbar = harmonic->hbar; FIXME_param.coefficient = harmonic->coefficient;
	// było grubsze FIXME tutaj, bo w skeypcie 2d-coulomb*py zmieniłem potentialMaximum a potem się dziwiłem, że nadal się liczy
	// dobrze - nie wieadomo dlaczego. A problem się wziął stąd że DUBLUJĘ TUTAJ DANE !!! FIXME FIXME !!!
	FIXME_param.potentialMaximum = harmonic->potentialMaximum;
	St1_QMStPotentialCoulomb FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
// FIXME(3) !!!!!!!! on nie może tutaj robić QMIGeom !!!!!!! musi robić QMIPhys !!!!! (i do tego musi to robić w Ip2_*)
HERE;
//	NDimTable<Complexr>& val_I_GeomMarginal(qmigeom->potentialMarginalDistribution);


/*FIXME*/	if(not harmonic->potentialInteractionGlobal) {
/*FIXME*/		harmonic->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
/*FIXME*/		harmonic->potentialInteractionGlobal->wasGenerated=false;
/*FIXME*/	}
/*FIXME*/		// FIXME ! - tensorProduct !!
HERE;
/*FIXME*/	NDimTable<Complexr>& val(harmonic->potentialInteractionGlobal->psiGlobalTable);

	if(not harmonic->potentialInteractionGlobal->wasGenerated) {
	if(psi->gridSize.size() <= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !
std::cerr << "####### Law2_QMIGeom_QMIPhysCoulomb::go  START!  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳p̳o̳t̳e̳n̳t̳i̳a̳l̳!̳!̳!̳\n";
		val.resize(psi->getPsiGlobalExisting()->psiGlobalTable);
		val.fill1WithFunction( psi->gridSize.size()
			, [&](Real i, int d)->Real    { return psi->iToX(i,d) - qmigeom->relPos21[d];}           // xyz position function
			, [&](Vector3r& xyz)->Complexr{ return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);} // function value at xyz
			);
		harmonic->potentialInteractionGlobal->wasGenerated = true;
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysCoulomb::go, dim>3\n"; exit(1); };
	}
// FIXME - this should go to Ip2_::go (parent, toplevel)
/*FIXME*/	//val_I_GeomMarginal = val;
// FIXME 2 - this is duplicate (a little) with Law2_QMIGeom_QMIPhysCoulombParticles, but here it knows that there is static potential, not a particle.

	return true;
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulombParticles  :  H A R M O N I C   I N T E R A C T I O N
*
*********************************************************************************/

CREATE_LOGGER(Law2_QMIGeom_QMIPhysCoulombParticles);

bool Law2_QMIGeom_QMIPhysCoulombParticles::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
{
/*
 *
	qmigeom->extents1  = qg1->extents;        // type: Vector3r
	qmigeom->extents2  = qg2->extents;        // type: Vector3r
	qmigeom->relPos21  = qs2->pos - qs1->pos; // type: Vector3r
	qmigeom->size1     = qs1->size;           // type: vector<Real>
	qmigeom->size2     = qs2->size;           // type: vector<Real>
	qmigeom->gridSize1 = qs1->gridSize;       // type: vector<size_t>
	qmigeom->gridSize2 = qs2->gridSize;       // type: vector<size_t>
 */



	QMIGeom*                  qmigeom = dynamic_cast<QMIGeom*                 >(ig.get());
	QMIPhysCoulombParticles* harmonic = dynamic_cast<QMIPhysCoulombParticles*>(ip.get());
	if(not qmigeom or not harmonic) {
		HERE_ERROR("missing qmigeom or QMIPhysCoulombParticles");
		exit(1);
	}

	// FIXME, but how?? I need this equation somehow.
	QMParametersCoulomb FIXME_param;
	FIXME_param.dim=harmonic->dim; FIXME_param.hbar = harmonic->hbar; FIXME_param.coefficient = harmonic->coefficient1.cwiseProduct(harmonic->coefficient2);
	FIXME_param.potentialMaximum = -10000; ///FIXME !!!!!//harmonic->potentialMaximum;
	St1_QMStPotentialCoulomb FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi1=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	QMStateDiscrete* psi2=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id2]->state.get());
// FIXME(3) !!!!!!!! on nie może tutaj robić QMIGeom !!!!!!! musi robić QMIPhys !!!!! (i do tego musi to robić w Ip2_*)
//	NDimTable<Complexr>& val_I_GeomMarginal(qmigeom->potentialMarginalDistribution);

	if(psi1->getPsiGlobalExisting()->members.size() == 0) {

/*FIXME*/	if(not harmonic->potentialInteractionGlobal) {
/*FIXME*/		harmonic->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
/*FIXME*/		harmonic->potentialInteractionGlobal->wasGenerated=false;
/*FIXME*/	}
/*FIXME*/		// FIXME ! - tensorProduct !!
HERE;
/*FIXME*/	NDimTable<Complexr>& val(harmonic->potentialInteractionGlobal->psiGlobalTable);


	if(not harmonic->potentialInteractionGlobal->wasGenerated) {
	if(psi1->dim() == psi2->dim() and psi1->dim()<= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !

HERE;
		NDimTable<Complexr>::DimN newDim=psi1->getPsiGlobalExisting()->psiGlobalTable.dim();
		newDim.insert(newDim.end(),psi2->getPsiGlobalExisting()->psiGlobalTable.dim().begin(),psi2->getPsiGlobalExisting()->psiGlobalTable.dim().end());
// generate multi dimensional potential here
std::cerr << "####### Law2_QMIGeom_QMIPhysCoulombParticles::go  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳̲P̲A̲R̲T̲I̲C̲L̲E̲ ̲I̲N̲T̲E̲R̲A̲C̲T̲I̲O̲N̲ ̲p̳o̳t̳e̳n̳t̳i̳a̳l̳\n";
		val.resize(newDim);             /// FIXME !!! ↓ - whichPartOfpsiGlobal
		val.fill2WithFunction( psi1->dim() , 0 , psi1->dim()   // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ sprawdzić to jeszcze!!!!!
			, [&](Real i_1, Real i_2, int d)->Real{return psi1->iToX(i_1,d) - psi2->iToX(i_2,d) - qmigeom->relPos21[d];} // xyz position function
				//FIXME: czy iToX() daje dobry wynik, gdy środek State::pos jest przesunięty?? Chyba raczej nie?
				//       a może musze to przesunięcie załatwiac osobno? Przy każdym wywołaniu tej methody, indywidaulnie?
			, [&](Vector3r& xyz)->Complexr{ return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);} // function value at xyz
			);
		harmonic->potentialInteractionGlobal->wasGenerated = true;
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysCoulomb::go, dim>3 or psi1->dim() != psi2->dim()\n"; exit(1); };
	}
// FIXME - calcMarginalDistribution somewhere else !! in Gl1_QMIGeom

//	if(not psi1->psiGlobal)	psi1->psiGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
//	if(not psi2->psiGlobal)	psi2->psiGlobal = psi1->psiGlobal;
//		assert( psi1->psiGlobal == psi2->psiGlobal );

/////	if(psi1->psiGlobal->members.size() == 0) {
		boost::shared_ptr<QMStateDiscreteGlobal> newGlobal(new QMStateDiscreteGlobal);
		newGlobal->members.push_back(I->id1);
		newGlobal->members.push_back(I->id2);
//                                                                        FIXME    ↓ - nie powienienem używać geometrii tylko faktyczną funkcję
HERE;
		std::vector<const NDimTable<Complexr>*> partsNormalized({&(psi1->getPsiGlobalExisting()->psiGlobalTable),&(psi2->getPsiGlobalExisting()->psiGlobalTable)});
HERE;
		newGlobal->psiGlobalTable = NDimTable<Complexr>(partsNormalized); // calcTensorProduct (duplikat w *Harmonic* i *Barrier*)

		newGlobal->wasGenerated = true;
	
		// FIXME - bez sensu, gridSize się dubluje z NDimTable::dim_n i ja tego wcześniej nie zauważyłem?
		std::vector<std::size_t> newGridSize = psi1->gridSize;
		newGridSize.insert(newGridSize.end(),psi2->gridSize.begin(),psi2->gridSize.end());
		assert( newGridSize == newGlobal->psiGlobalTable.dim());
		newGlobal->gridSize = newGlobal->psiGlobalTable.dim(); //psi1->gridSize + psi2->gridSize;
		newGlobal->setSpatialSizeGlobal(psi1->getSpatialSize());
		newGlobal->addSpatialSizeGlobal(psi2->getSpatialSize());

		psi1->wasGenerated = true;
		psi2->wasGenerated = true;
		psi1->setEntangled(0); // FIXME - allow more entangle particles, so not always ZERO!
		psi2->setEntangled(psi1->dim());
//std::cerr << ".......... " << psi1->whichPartOfpsiGlobal << " ......... " << psi2->whichPartOfpsiGlobal << " ......  " ;
std::cerr << "...QMPotentialCoulomb....... "; for(Real s :newGlobal->getSpatialSizeGlobal()){ std::cerr <<s << ",";} std::cerr <<"\n";
		psi1->setPsiGlobal( psi2->setPsiGlobal( newGlobal ) );

	} else { 
		if(timeLimitC.messageAllowed(12)) std::cerr << "\nLaw2_QMIGeom_QMIPhysCoulomb::go skip tensor product\n"; //exit(1); 
	};

// ? 
// ? 		std::size_t rank = psi1->psiGlobal->dim();
// ? 		std::vector<short int> remainDims(rank, 0);
// ? 		for(std::size_t i = psi1->whichPartOfpsiGlobal ; i < (rank+psi1->dim()) ;  i++) remainDims[i]=1;
// ? /*FIXME - better use QMGeometryDisplayConfig */
// ? /*FIXME*/	val_I_GeomMarginal = val.calcMarginalDistribution(remainDims,psi1->psiGlobal->size);
// ? 

	return true;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////                                                      ////////////////////////////////////////
////////////////////////////////////                                                      ////////////////////////////////////////
////////////////////////////////////                       hydrogen                       ////////////////////////////////////////
////////////////////////////////////                                                      ////////////////////////////////////////
////////////////////////////////////                                                      ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*********************************************************************************
*
* Ip2   QMParticleCoulomb   QMParametersCoulomb  →  QMIPhysCoulombParticles
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles);

void Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
//	std::cerr << "\n\n## 1 ##### Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles::go  START!\n\n\n";
	if(timeLimitC.messageAllowed(12)) std::cerr << "####### Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles::go  START!\n";

	shared_ptr<QMIPhysCoulombParticles> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysCoulombParticles>(I->phys);
	else { pot=shared_ptr<QMIPhysCoulombParticles>(new QMIPhysCoulombParticles()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	const QMParticleCoulomb  * qm1 = dynamic_cast<QMParticleCoulomb*  >(m1.get()); // FIXME: static
	const QMParametersCoulomb* qm2 = dynamic_cast<QMParametersCoulomb*>(m2.get()); // FIXME: static
	if(not qm1 or not qm2) {
		std::cerr << "\n\n ERROR: Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles \n\n";
		exit(1);
	};

	pot->coefficient1 = qm1->coefficient;
	pot->coefficient2 = qm2->coefficient;
	pot->potentialMaximum = qm2->potentialMaximum;

// FIXME: create here QMIPhys::potentialInteractionGlobal
//        then call 
//		Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);
//        to assign (or calcMarginalDistribution??? ← no, better leave that for Gl1_drawer), so I don't actually need to assign!
//		harmonic->potentialInteractionGlobal->psiGlobalTable = val;

}

void Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	std::cerr << "\n\n####### Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::goReverse <-------------------- START!\n\n";
	I->swapOrder();
	go(m2,m1,I);
};







/*********************************************************************************
*
* Ip2   QMParticleCoulomb   QMParametersCoulomb  →  QMIPhysCoulombParticleInPotential
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential);

void Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential::go( // wodór
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
//	std::cerr << "\n\n## 1 ##### Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential::go  START!\n\n\n";
	if(timeLimitC.messageAllowed(12)) std::cerr << "####### Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential::go  START!\n";

	shared_ptr<QMIPhysCoulombParticleInPotential> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysCoulombParticleInPotential>(I->phys);
	else { pot=shared_ptr<QMIPhysCoulombParticleInPotential>(new QMIPhysCoulombParticleInPotential()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	const QMParticleCoulomb  * qm1 = dynamic_cast<QMParticleCoulomb*  >(m1.get()); // FIXME: static
	const QMParametersCoulomb* qm2 = dynamic_cast<QMParametersCoulomb*>(m2.get()); // FIXME: static
	if(not qm1 or not qm2) {
		std::cerr << "\n\n ERROR: Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential \n\n";
		exit(1);
	};

	pot->coefficient1 = qm1->coefficient;
	pot->coefficient2 = qm2->coefficient;
	pot->potentialMaximum = qm2->potentialMaximum;

// FIXME: create here QMIPhys::potentialInteractionGlobal
//        then call 
//		Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);
//        to assign (or calcMarginalDistribution??? ← no, better leave that for Gl1_drawer), so I don't actually need to assign!
//		harmonic->potentialInteractionGlobal->psiGlobalTable = val;

}

void Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticleInPotential::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	std::cerr << "\n\n####### Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb::goReverse <-------------------- START!\n\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulombParticles  :  freeely propagating hydrogen
*
*********************************************************************************/

#include "QMPacketHydrogenEigenFunc.hpp"
#include "QMPacketGaussianWave.hpp"

CREATE_LOGGER(Law2_QMIGeom_QMIPhysCoulombParticlesFree);

bool Law2_QMIGeom_QMIPhysCoulombParticlesFree::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
{
//	std::cerr << "\n\n## 2 ##### Law2_QMIGeom_QMIPhysCoulombParticlesFree::go  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳̲P̲A̲R̲T̲I̲C̲L̲E̲ ̲I̲N̲T̲E̲R̲A̲C̲T̲I̲O̲N̲ ̲p̳o̳t̳e̳n̳t̳i̳a̳l̳\n\n\n";
/*
 *
	qmigeom->extents1  = qg1->extents;        // type: Vector3r
	qmigeom->extents2  = qg2->extents;        // type: Vector3r
	qmigeom->relPos21  = qs2->pos - qs1->pos; // type: Vector3r
	qmigeom->size1     = qs1->size;           // type: vector<Real>
	qmigeom->size2     = qs2->size;           // type: vector<Real>
	qmigeom->gridSize1 = qs1->gridSize;       // type: vector<size_t>
	qmigeom->gridSize2 = qs2->gridSize;       // type: vector<size_t>
 */


//std::cerr << " id1 = " << I->id1 << "\n";
//std::cerr << " id2 = " << I->id2 << "\n";

	QMIGeom*                  qmigeom  = dynamic_cast<QMIGeom*                 >(ig.get());
	QMIPhysCoulombParticles* harmonic = dynamic_cast<QMIPhysCoulombParticles*>(ip.get());
	if(not qmigeom or not harmonic) {
		HERE_ERROR("not qmigeom or not harmonic");
		exit(1);
	};

	// FIXME, but how?? I need this equation somehow.
	QMParametersCoulomb FIXME_param;
	FIXME_param.dim=harmonic->dim; FIXME_param.hbar = harmonic->hbar; FIXME_param.coefficient = harmonic->coefficient1.cwiseProduct(harmonic->coefficient2);
	FIXME_param.potentialMaximum = -10000; ///FIXME !!!!!//harmonic->potentialMaximum;
	St1_QMStPotentialCoulomb FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi1=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	QMStateDiscrete* psi2=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id2]->state.get());
// FIXME(3) !!!!!!!! on nie może tutaj robić QMIGeom !!!!!!! musi robić QMIPhys !!!!! (i do tego musi to robić w Ip2_*)
//	NDimTable<Complexr>& val_I_GeomMarginal(qmigeom->potentialMarginalDistribution);

	if(psi1->getPsiGlobalExisting()->members.size() == 0 or (psi1->isAnalytic() and psi2->isAnalytic()) ) {

/*FIXME*/	if(not harmonic->potentialInteractionGlobal) {
/*FIXME*/		harmonic->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
/*FIXME*/		harmonic->potentialInteractionGlobal->wasGenerated=false;
/*FIXME*/	}
/*FIXME*/		// FIXME ! - tensorProduct !!
HERE;
/*FIXME*/	NDimTable<Complexr>& val(harmonic->potentialInteractionGlobal->psiGlobalTable);


	if(not harmonic->potentialInteractionGlobal->wasGenerated) {
if(psi1->getPsiGlobalExisting()->members.size() == 0){
	if(psi1->dim() == psi2->dim() and psi1->dim()<= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !

HERE;
		NDimTable<Complexr>::DimN newDim=psi1->getPsiGlobalExisting()->psiGlobalTable.dim();
		newDim.insert(newDim.end(),psi2->getPsiGlobalExisting()->psiGlobalTable.dim().begin(),psi2->getPsiGlobalExisting()->psiGlobalTable.dim().end());
// generate multi dimensional potential here
std::cerr << "####### Law2_QMIGeom_QMIPhysCoulombParticlesFree::go  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳̲P̲A̲R̲T̲I̲C̲L̲E̲ ̲I̲N̲T̲E̲R̲A̲C̲T̲I̲O̲N̲ ̲p̳o̳t̳e̳n̳t̳i̳a̳l̳\n";
		val.resize(newDim);             /// FIXME !!! ↓ - whichPartOfpsiGlobal
		val.fill2WithFunction( psi1->dim() , 0 , psi1->dim()   // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ sprawdzić to jeszcze!!!!!
			, [&](Real i_1, Real i_2, int d)->Real{return psi1->iToX(i_1,d) - psi2->iToX(i_2,d) - qmigeom->relPos21[d];} // xyz position function
				//FIXME: czy iToX() daje dobry wynik, gdy środek State::pos jest przesunięty?? Chyba raczej nie?
				//       a może musze to przesunięcie załatwiac osobno? Przy każdym wywołaniu tej methody, indywidaulnie?
			, [&](Vector3r& xyz)->Complexr{ return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);} // function value at xyz
			);
		harmonic->potentialInteractionGlobal->wasGenerated = true;
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysCoulomb::go, dim>3 or psi1->dim() != psi2->dim()\n"; exit(1); };
}
	}
// FIXME - calcMarginalDistribution somewhere else !! in Gl1_QMIGeom

//	if(not psi1->psiGlobal)	psi1->psiGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
//	if(not psi2->psiGlobal)	psi2->psiGlobal = psi1->psiGlobal;
//		assert( psi1->psiGlobal == psi2->psiGlobal );

/////	if(psi1->psiGlobal->members.size() == 0) {
		boost::shared_ptr<QMStateDiscreteGlobal> newGlobal(new QMStateDiscreteGlobal);
if( psi1->getPsiGlobalExisting()->psiGlobalTable.dim().size() + psi2->getPsiGlobalExisting()->psiGlobalTable.dim().size() 
	== psi1->dim()*2
){
		newGlobal->members.push_back(I->id1);
		newGlobal->members.push_back(I->id2);
//                                                                        FIXME    ↓ - nie powienienem używać geometrii tylko faktyczną funkcję
HERE;
		std::vector<const NDimTable<Complexr>*> partsNormalized({&(psi1->getPsiGlobalExisting()->psiGlobalTable),&(psi2->getPsiGlobalExisting()->psiGlobalTable)});
//std::cerr << " 1  psi1->getPsiGlobalExisting()->psiGlobalTable.dim() = " << psi1->getPsiGlobalExisting()->psiGlobalTable.dim() << "\n";
//std::cerr << " 1  psi2->getPsiGlobalExisting()->psiGlobalTable.dim() = " << psi2->getPsiGlobalExisting()->psiGlobalTable.dim() << "\n";
//std::cerr << " 1  newGlobal->psiGlobalTable                   .dim() = " << newGlobal->psiGlobalTable                   .dim() << "\n";
HERE;
		newGlobal->psiGlobalTable = NDimTable<Complexr>(partsNormalized); // calcTensorProduct (duplikat w *Harmonic* i *Barrier*)
//std::cerr << " 2  newGlobal->psiGlobalTable                   .dim() = " << newGlobal->psiGlobalTable                   .dim() << "\n";
} else 	newGlobal = psi1->getPsiGlobalExisting();
//// FIXME MEGA_FIXME !!!!!!!!!!!!! jak to uporządkować??
		St1_QMPacketHydrogenEigenGaussianWave FIXME_2particle;
		QMPacketHydrogenEigenGaussianWave     FIXME_qmstate;
		QMParameters* par1;
		QMParameters* par2;

			FIXME_qmstate.hydrogenEigenFunc = boost::dynamic_pointer_cast<QMPacketHydrogenEigenFunc>((*(scene->bodies))[I->id1]->state);
			par1                            = dynamic_cast<QMParameters*>(dynamic_cast<QMParametersCoulomb*>((*(scene->bodies))[I->id1]->material.get()));
		if(not FIXME_qmstate.hydrogenEigenFunc) {
			FIXME_qmstate.hydrogenEigenFunc = boost::dynamic_pointer_cast<QMPacketHydrogenEigenFunc>((*(scene->bodies))[I->id2]->state);
			par1                            = dynamic_cast<QMParameters*>(dynamic_cast<QMParametersCoulomb*>((*(scene->bodies))[I->id2]->material.get()));
		}
			
			FIXME_qmstate.gaussianWave      = boost::dynamic_pointer_cast<QMPacketGaussianWave     >((*(scene->bodies))[I->id1]->state);
			par2                            = dynamic_cast<QMParameters*>(dynamic_cast<QMParticleCoulomb*>((*(scene->bodies))[I->id1]->material.get()));
		if(not FIXME_qmstate.gaussianWave     ) {
			FIXME_qmstate.gaussianWave      = boost::dynamic_pointer_cast<QMPacketGaussianWave     >((*(scene->bodies))[I->id2]->state);
			par2                            = dynamic_cast<QMParameters*>(dynamic_cast<QMParticleCoulomb*>((*(scene->bodies))[I->id2]->material.get()));
		}

		if(not FIXME_qmstate.gaussianWave or not FIXME_qmstate.hydrogenEigenFunc or not par1 or not par2) {
			std::cerr << "\n\nQMPacketHydrogenEigenFunc & QMPacketGaussianWave expected.\n\n";
			exit(1);
		}

// Fx3	std::function<void()> generate_Global_Table = [&]()->void{
HERE2;
		newGlobal->psiGlobalTable.fill2part_WithFunction( psi1->dim() , 0 , psi1->dim()   // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ sprawdzić to jeszcze!!!!!
			, [&](Real i_1,int d)->Real{return psi1->iToX(i_1,d) + psi1->pos[d];}// - qmigeom->relPos21[d];} // xyz position function
			, [&](Real i_2,int d)->Real{return psi2->iToX(i_2,d) + psi2->pos[d];}// - qmigeom->relPos21[d];} // xyz position function
			, [&](Vector3r& xyz1,Vector3r& xyz2)->Complexr{ return FIXME_2particle.getValPos_2particles(xyz1,xyz2,par1,par2,&FIXME_qmstate);} // function value at xyz
			);


		try {
			boost::shared_ptr<Law2_QMIGeom_QMIPhys_GlobalWavefunction> me = boost::dynamic_pointer_cast<Law2_QMIGeom_QMIPhys_GlobalWavefunction>(shared_from_this());
			psi1->setLaw2Generator(/*shared_ptr<IGeom>&*/ ig, /*shared_ptr<IPhys>&*/ ip, /*Interaction* */ I->shared_from_this(), me );
			psi2->setLaw2Generator(/*shared_ptr<IGeom>&*/ ig, /*shared_ptr<IPhys>&*/ ip, /*Interaction* */ I->shared_from_this(), me );
		} catch(const boost::bad_weak_ptr& e) {
			// FIXME - serialization of StateDispatcher gone wrong?
			std::cerr << "Law2_QMIGeom_QMIPhysCoulombParticlesFree::go() exception boost::bad_weak_ptr, there's no shared_ptr to this="<<this<<" class. Serialization gone wrong?\n";
		}



// Fx3	};
// Fx3	generate_Global_Table();
// Fx3	psi1->set_FIXME_EXTRA_Generator(generate_Global_Table);
// Fx3	psi2->set_FIXME_EXTRA_Generator(generate_Global_Table);

//// FIXME MEGA_FIXME - end !!!!!!!
/* 2 */// static bool printed=false;
/* 2 */// if(not printed) {
/* 2 */// 	std::cout << "\n--------------------------------\n";
/* 2 */// 	newGlobal->psiGlobalTable.print();
/* 2 */// 	std::cout << "\n--------------------------------\n";
/* 2 */// 	printed=true;
/* 2 */// }

		newGlobal->wasGenerated = true;
//if(timeLimitC.messageAllowed(14)) 
//std::cerr << "# →→→→→ Law2_QMIGeom_QMIPhysCoulombParticlesFree::go  GENERATED !\n";
//std::cerr << " id1 = " << I->id1 << "\n";
//std::cerr << " id2 = " << I->id2 << "\n";
		// FIXME - bez sensu, gridSize się dubluje z NDimTable::dim_n i ja tego wcześniej nie zauważyłem?
		std::vector<std::size_t> newGridSize = psi1->gridSize;
		newGridSize.insert(newGridSize.end(),psi2->gridSize.begin(),psi2->gridSize.end());
//std::cerr << " 3 newGridSize = " << newGridSize << "   newGlobal->psiGlobalTable.dim() = " << newGlobal->psiGlobalTable.dim() << "\n";
		assert( newGridSize == newGlobal->psiGlobalTable.dim());
		newGlobal->gridSize = newGlobal->psiGlobalTable.dim(); //psi1->gridSize + psi2->gridSize;
		newGlobal->setSpatialSizeGlobal(psi1->getSpatialSize());
		newGlobal->addSpatialSizeGlobal(psi2->getSpatialSize());

		psi1->wasGenerated = true;
		psi2->wasGenerated = true;
		psi1->setEntangled(0); // FIXME - allow more entangle particles, so not always ZERO!
		psi2->setEntangled(psi1->dim());
//std::cerr << ".......... " << psi1->whichPartOfpsiGlobal << " ......... " << psi2->whichPartOfpsiGlobal << " ......  " ;
if(timeLimitC.messageAllowed(13)) {
std::cerr << "...QMPotentialCoulomb....... "; for(Real s :newGlobal->getSpatialSizeGlobal()){ std::cerr <<s << ",";} std::cerr <<"\n";
}
		psi1->setPsiGlobal( psi2->setPsiGlobal( newGlobal ) );

	} else { 
		if(timeLimitC.messageAllowed(12)) std::cerr << "\nLaw2_QMIGeom_QMIPhysCoulombParticlesFree::go skip tensor product\n"; //exit(1); 
	};

// ? 
// ? 		std::size_t rank = psi1->psiGlobal->dim();
// ? 		std::vector<short int> remainDims(rank, 0);
// ? 		for(std::size_t i = psi1->whichPartOfpsiGlobal ; i < (rank+psi1->dim()) ;  i++) remainDims[i]=1;
// ? /*FIXME - better use QMGeometryDisplayConfig */
// ? /*FIXME*/	val_I_GeomMarginal = val.calcMarginalDistribution(remainDims,psi1->psiGlobal->size);
// ? 

	return true;
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulombParticleInPotential  :  H A R M O N I C   P O T E N T I A L
*
*********************************************************************************/

CREATE_LOGGER(Law2_QMIGeom_QMIPhysCoulombParticleInPotential);

bool Law2_QMIGeom_QMIPhysCoulombParticleInPotential::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
{

	QMIGeom*                            qmigeom  = dynamic_cast<QMIGeom*                          >(ig.get());
	QMIPhysCoulombParticleInPotential*  pott     = dynamic_cast<QMIPhysCoulombParticleInPotential*>(ip.get());
	if(not qmigeom or not pott) {
		HERE_ERROR("missing pott, qmigeom");
		exit(1);
	};

	// FIXME, but how?? I need this equation somehow.
	QMParametersCoulomb FIXME_param;
	FIXME_param.dim              = pott->dim;
	FIXME_param.hbar             = pott->hbar;
	FIXME_param.coefficient      = Vector3r((pott->coefficient1).cwiseProduct(pott->coefficient2));
	FIXME_param.potentialMaximum = pott->potentialMaximum;
	// było grubsze FIXME tutaj, bo w skrypcie 2d-coulomb*py zmieniłem potentialMaximum a potem się dziwiłem, że nadal się liczy
	// dobrze - nie wieadomo dlaczego. A problem się wziął stąd że DUBLUJĘ TUTAJ DANE !!! FIXME FIXME !!!
	St1_QMStPotentialCoulomb FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
	if(psi->getClassName() == "QMStPotentialCoulomb" ) {
		HERE_ERROR("class QMStPotentialCoulomb encountered! ");
		exit(1);
	};
	std::cerr << "Law2_QMIGeom_QMIPhysCoulombParticleInPotential::go uses class "<<psi->getClassName()<<"\n";
// FIXME(3) !!!!!!!! on nie może tutaj robić QMIGeom !!!!!!! musi robić QMIPhys !!!!! (i do tego musi to robić w Ip2_*)
HERE2;
/*FIXME*/	if(not  pott->potentialInteractionGlobal) {
/*FIXME*/		pott->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
/*FIXME*/		pott->potentialInteractionGlobal->wasGenerated=false;
/*FIXME*/	}
/*FIXME*/		// FIXME ! - tensorProduct !!
HERE2;
/*FIXME*/	NDimTable<Complexr>& val(pott->potentialInteractionGlobal->psiGlobalTable);

	size_t members_size = psi->getPsiGlobalExisting()->members.size();
	size_t this_member  = 0;
	if(members_size != 1) {
		std::cerr << "members_size="<<members_size<<"\n";
		for(size_t i=0; i<members_size ; i++) {
			if(psi->getPsiGlobalExisting()->members[i] != I->id1) { // FIXME - tę pętlę można uprościć, zamiast `i` używać `this_member`
				this_member++; // find which one it is in this NDimTable
			} else {
				break;
			}
		}
		std::cerr << "this_member="<<this_member<<"\n";
	}

	if(not pott->potentialInteractionGlobal->wasGenerated) {
	if(psi->gridSize.size() <= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !
std::cerr << "####### Law2_QMIGeom_QMIPhysCoulombParticleInPotential::go  START!  r̳e̳c̳a̳l̳c̳u̳l̳a̳t̳i̳n̳g̳ ̳w̳h̳o̳l̳e̳ ̳p̳o̳t̳e̳n̳t̳i̳a̳l̳!̳!̳!̳\n";
		val.resize(psi->getPsiGlobalExisting()->psiGlobalTable);
		val.fill1WithFunction( psi->gridSize.size()
			, [&](Real i, int d)->Real    { return psi->iToX(i,d) - qmigeom->relPos21[d];}           // xyz position function
			, [&](Vector3r& xyz)->Complexr{ return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);} // function value at xyz
			, this_member);
		pott->potentialInteractionGlobal->wasGenerated = true;
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysCoulomb::go, dim>3\n"; exit(1); };
	}
// FIXME - this should go to Ip2_::go (parent, toplevel)
/*FIXME*///	val_I_GeomMarginal = val;
// FIXME 2 - this is duplicate (a little) with Law2_QMIGeom_QMIPhysCoulombParticles, but here it knows that there is static potential, not a particle.

	return true;
};

