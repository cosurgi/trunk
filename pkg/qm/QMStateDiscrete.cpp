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
	std::cerr<<"positionSize="<<positionSize<<"\n";
	std::cerr<<"gridSize="<<gridSize<<"\n";
	stepPos= getStepPos();
	Real step=stepPos;
	startX = -positionSize[0]/2.0;     // FIXME - does it really have to be centered around zero?
	endX   =  positionSize[0]/2.0;
	startY = -positionSize[1]/2.0;     // FIXME - does it really have to be centered around zero?
	endY   =  positionSize[1]/2.0;
	startZ = -positionSize[2]/2.0;     // FIXME - does it really have to be centered around zero?
	endZ   =  positionSize[2]/2.0;
	if(this->dim == 1) {
		tableValuesPosition      .resize(1);          // no         coordinate
		tableValuesPosition[0]   .resize(1);          // no         coordinate
		tableValuesPosition[0][0].resize(gridSize,0); // x position coordinate

		// Fill the table by copying from creator
// FIXME	tableWavenumber  =  tableValueWavenumber=  tablePosition=  tableValuesPosition;
//		/*tableWavenumber=*/tableValueWavenumber=/*tablePosition=*/tableValuesPosition;
		int i       =  0;
		for(Real x=startX ; i<gridSize ; x+=step,i++ ) {
			tableValuesPosition[0][0][i] = creator->getValPos(Vector3r(x,0,0));
// FIXME		tablePosition      [0][0][i] = x;
		}
	} else if(this->dim == 2) {
		assert( positionSize[0]==positionSize[1]); // FIXME - think if it's really necessary
		tableValuesPosition      .resize(1);          // no         coordinate
		tableValuesPosition[0]   .resize(gridSize);   // x position coordinate
		FOREACH(std::vector<Complexr>& xx, tableValuesPosition[0]) {
			xx.resize(gridSize,0);          // y position coordinate
		};

		// Fill the table by copying from creator
// FIXME	tableWavenumber  =  tableValueWavenumber=  tablePosition=  tableValuesPosition;
//		/*tableWavenumber=*/tableValueWavenumber=/*tablePosition=*/tableValuesPosition;
		int i       =  0;
		for(Real x=startX ; i<gridSize ; x+=step,i++ ) {
			int j=0;
			for(Real y=startY ; j<gridSize ; y+=step,j++ )
			{
				tableValuesPosition[0][i][j] = creator->getValPos(Vector3r(x,y,0));
			}
		}
	} else if(this->dim == 3) {
		assert( (positionSize[0]==positionSize[1]) and (positionSize[0]==positionSize[2])); // FIXME - think if it's really necessary
		tableValuesPosition      .resize(gridSize);  // x position coordinate
		FOREACH(std::vector<std::vector<Complexr> >& xx, tableValuesPosition   ) {
			xx.resize(gridSize);           // y position coordinate
			FOREACH(std::vector<Complexr>& yy, xx) {
				yy.resize(gridSize,0); // z position coordinate
			};
		};

		// Fill the table by copying from creator
// FIXME	tableWavenumber  =  tableValueWavenumber=  tablePosition=  tableValuesPosition;
//		/*tableWavenumber=*/tableValueWavenumber=/*tablePosition=*/tableValuesPosition;
		int i       =  0;
		for(Real x=startX ; i<gridSize ; x+=step,i++ ) {
			int j=0;
			for(Real y=startY ; j<gridSize ; y+=step,j++ )
			{
				int k=0;
				for(Real z=startZ ; k<gridSize ; z+=step,k++ )
				{
					tableValuesPosition[i][j][k] = creator->getValPos(Vector3r(x,y,z));
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
	int i( (xyz[0]-startX)/stepPos ), // FIXME - maybe add interpolation?
	    j( (xyz[1]-startY)/stepPos ),
	    k( (xyz[2]-startZ)/stepPos );
	switch(this->dim) {
		case 1 :
			if    ( (i>=0) and (i<gridSize))
				return tableValuesPosition[0][0][i];
			//else throw std::runtime_error("QMStateDiscrete::getValPos "+boost::lexical_cast<string>(i)+" outside bounds.");
			else if(i==gridSize or !timeLimit.messageAllowed(errorTime)) return 0; // skip silently
			else {std::cerr << "QMStateDiscrete::getValPos "<<i<<" outside bounds.";return 0;};
		break;

		case 2 :
			if(   ( (i>=0) and (i<gridSize))
			  and ( (j>=0) and (j<gridSize)))
				return tableValuesPosition[0][i][j];
			else if(i==gridSize or j==gridSize or !timeLimit.messageAllowed(errorTime)) return 0; // skip silently
			else {std::cerr << "QMStateDiscrete::getValPos "<<i<<" or "<<j<<" outside bounds.";return 0;};
			//else throw std::runtime_error("QMStateDiscrete::getValPos outside bounds.");
		break;

		case 3 :
			if(   ( (i>=0) and (i<gridSize))
			  and ( (j>=0) and (j<gridSize))
			  and ( (k>=0) and (k<gridSize)))
				return tableValuesPosition[i][j][k];
			else if(i==gridSize or j==gridSize or k==gridSize or !timeLimit.messageAllowed(errorTime)) return 0; // skip silently
			else throw std::runtime_error("QMStateDiscrete::getValPos outside bounds.");
		break;

		default: throw std::runtime_error("Wrong dimension! This code in QMStateDiscrete::getValPos should be unreachable");
	}
};

