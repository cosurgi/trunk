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
// FIXME - does it really have to be centered around zero?
	if(this->dim == 1) {
		if (gridSize.size() != 1) throw std::out_of_range("QMStateDiscrete::postLoad(), should be dimension 1\n");
		tableValuesPosition.resize(gridSize);
		for(size_t i=0 ; i<gridSize[0] ; i++)
			tableValuesPosition.at(i) = creator->getValPos(Vector3r(iToX(i,0),0,0));
	} else if(this->dim == 2) {
		if (gridSize.size() != 2) throw std::out_of_range("QMStateDiscrete::postLoad(), should be dimension 2\n");
		tableValuesPosition.resize(gridSize,-0.5);                                                                   //   ↓
		for(size_t i=0 ; i<gridSize[0] ; i++)
		for(size_t j=0 ; j<gridSize[1] ; j++)
			tableValuesPosition.at    ( i,j ) = creator->getValPos(Vector3r(iToX(i,0),iToX(j,1),0));
// OK			tableValuesPosition.atSafe({i,j}) = creator->getValPos(Vector3r(iToX(i,0),iToX(j,1),0));
//			///////////////////////////////////   o dziwo zamiana x oraz y "pomogła"                           //   ↑
//			//////////////////// FIXME - może przypisać wszzędzie -0.5 żeby widzieć które wartości są przypisywane a które nie
	} else if(this->dim == 3) {
		if(not( (size[0]==size[1]) and (size[0]==size[2]))) {
			std::cerr << "WARNIG - wavepacket dimensions are different in each positional representation \
			direction. This is not tested yet.\n";
		}; // FIXME - think if it's really necessary. Test it.
		//3 tableValuesPosition      .resize(gridSize);  // x position coordinate
		//3 FOREACH(std::vector<std::vector<Complexr> >& xx, tableValuesPosition   ) {
		//3 	xx.resize(gridSize);           // y position coordinate
		//3 	FOREACH(std::vector<Complexr>& yy, xx) {
		//3 		yy.resize(gridSize,0); // z position coordinate
		//3 	};
		//3 };
		tableValuesPosition.resize(gridSize);

		// Fill the table by copying from creator
// FIXME	tableWavenumber  =  tableValueWavenumber=  tablePosition=  tableValuesPosition;
//		/*tableWavenumber=*/tableValueWavenumber=/*tablePosition=*/tableValuesPosition;
		int i       =  0;
		for(Real x=start(0) ; i<gridSize[0] ; x+=stepInPositionalRepresentation(0),i++ ) {
			int j=0;
			for(Real y=start(1) ; j<gridSize[1] ; y+=stepInPositionalRepresentation(1),j++ )
			{
				int k=0;
				for(Real z=start(2) ; k<gridSize[2] ; z+=stepInPositionalRepresentation(2),k++ )
				{
					tableValuesPosition.at(i,j,k) = creator->getValPos(Vector3r(x,y,z));
				}
			}
		}
	} else {
		throw std::runtime_error("QMStateDiscrete() supports in 1,2 or 3 dimensions.");
	}
	} else { // not a firstRun, we have been just loaded from file
	};
	//////////////FIXME qtHide="nowyKontener"; albo może nawet poprawić qtHide w klasie macierzystej, co za różnica(?) E, chyba lepiej tu, żeby było widać, że tu.
};

/// return complex quantum aplitude at given positional representation coordinates
Complexr QMStateDiscrete::getValPos(Vector3r xyz)
{
	double errorTime(2);
	int i( xToI(xyz[0],0) ),
	    j( xToI(xyz[1],1) ),
	    k( xToI(xyz[2],2) );
	switch(this->dim) { // FIXME(2) - should instead give just `const ref&` to this table, but GLDraw has problem - draws one too much!!
	
	//FIXME - must work for all dimensions, with contractions !!!!!!
		case 1 :
			if    ( (i>=0) and (i<gridSize[0]))
				return tableValuesPosition.at(i);
			//else throw std::runtime_error("QMStateDiscrete::getValPos "+boost::lexical_cast<string>(i)+" outside bounds.");
			else if(i==gridSize[0] or !timeLimit.messageAllowed(errorTime)) return 0; // skip silently
			else {std::cerr << "QMStateDiscrete::getValPos "<<i<<" outside bounds.";return 0;};
		break;

		case 2 :
			if(   ( (i>=0) and (i<gridSize[0]))
			  and ( (j>=0) and (j<gridSize[1])))
				return tableValuesPosition.at(i,j);
			else if(i==gridSize[0] or j==gridSize[1] or !timeLimit.messageAllowed(errorTime)) return 0; // skip silently
			else {std::cerr << "QMStateDiscrete::getValPos "<<i<<" or "<<j<<" outside bounds.";return 0;};
			//else throw std::runtime_error("QMStateDiscrete::getValPos outside bounds.");
		break;

		case 3 :
			if(   ( (i>=0) and (i<gridSize[0]))
			  and ( (j>=0) and (j<gridSize[1]))
			  and ( (k>=0) and (k<gridSize[2])))
				return tableValuesPosition.at(i,j,k);
			else if(i==gridSize[0] or j==gridSize[1] or k==gridSize[2] or !timeLimit.messageAllowed(errorTime)) return 0; // skip silently
			else throw std::runtime_error("QMStateDiscrete::getValPos outside bounds.");
		break;

		default: throw std::runtime_error("Wrong dimension! This code in QMStateDiscrete::getValPos should be unreachable");
	}
};

