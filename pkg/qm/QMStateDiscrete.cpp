// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateDiscrete.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMStateDiscrete)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMStateDiscrete);
// !! at least one virtual function in the .cpp file
QMStateDiscrete::~QMStateDiscrete(){};

void QMStateDiscrete::postLoad(QMStateDiscrete&)
{
	std::cerr<<"\nQMStateDiscrete postLoad\n";
	std::cerr<<"firstRun="<<firstRun<<"\n";
	if(firstRun) { // initialize from creator upon firstRun
	firstRun = false;
	if(this->dim == 1) {
		if (gridSize.size() != 1) throw std::out_of_range("QMStateDiscrete::postLoad(), should be dimension 1\n");
		tableValuesPosition.resize(gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<gridSize[0] ; i++)
			tableValuesPosition.at(i) = creator->getValPos(Vector3r(iToX(i,0),0,0));
	} else if(this->dim == 2) {
		if (gridSize.size() != 2) throw std::out_of_range("QMStateDiscrete::postLoad(), should be dimension 2\n");
		tableValuesPosition.resize(gridSize,5);  // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<gridSize[0] ; i++)
		for(size_t j=0 ; j<gridSize[1] ; j++)
			tableValuesPosition.at    ( i,j ) = creator->getValPos(Vector3r(iToX(i,0),iToX(j,1),0));
		//OK - that was just to be safe
		//	tableValuesPosition.atSafe({i,j}) = creator->getValPos(Vector3r(iToX(i,0),iToX(j,1),0));
	} else if(this->dim == 3) {
		if (gridSize.size() != 3) throw std::out_of_range("QMStateDiscrete::postLoad(), should be dimension 3\n");
		tableValuesPosition.resize(gridSize,5); // initialize with obviously wrong value, eg. 5, so that mistakes are easy to spot
		for(size_t i=0 ; i<gridSize[0] ; i++)
		for(size_t j=0 ; j<gridSize[1] ; j++)
		for(size_t k=0 ; k<gridSize[2] ; k++)
			tableValuesPosition.at    ( i,j,k ) = creator->getValPos(Vector3r(iToX(i,0),iToX(j,1),iToX(k,2)));
	} else {
		throw std::runtime_error("QMStateDiscrete() supports in 1,2 or 3 dimensions.");
	}
	} else { // not a firstRun, we have been just loaded from file
	};
	//////////////FIXME qtHide="nowyKontener"; albo może nawet poprawić qtHide w klasie macierzystej, co za różnica(?) E, chyba lepiej tu, żeby było widać, że tu.
};

/// return complex quantum aplitude at given positional representation coordinates
Complexr QMStateDiscrete::getValPos(Vector3r xyz) // FIXME - should take   std::vector<Real> (probably? - maybe useful for 4D?)
{
	double errorTime(2);
	switch(this->dim) {
	// FIXME(2) - should instead give just `const ref&` to this table, but GLDraw has problem - draws one too much!!
	// FIXME - must work for all dimensions, with contractions !!!!!!
		case 1 : return tableValuesPosition.atSafe({xToI(xyz[0],0)                              }); break;
		case 2 : return tableValuesPosition.atSafe({xToI(xyz[0],0),xToI(xyz[1],1)               }); break;
		case 3 : return tableValuesPosition.atSafe({xToI(xyz[0],0),xToI(xyz[1],1),xToI(xyz[2],2)}); break;
		default: throw std::runtime_error("Wrong dimension! This code in QMStateDiscrete::getValPos should be unreachable");
	}
};

