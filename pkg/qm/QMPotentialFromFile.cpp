// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include <iostream>
#include <sstream>
#include <fstream>

#include "QMPotentialFromFile.hpp"

#include <lib/time/TimeLimit.hpp>
#include <lib/smoothing/Spline6Interpolate.hpp>
//#include <lib/smoothing/Sinc16Interpolate.hpp>

TimeLimit timeLimitF; // FIXME - remove when finshed fixing

YADE_PLUGIN(
	(QMParametersFromFile)
	(QMStPotentialFromFile)
	(St1_QMStPotentialFromFile)
	(QMIPhysFromFile)
	(Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile)
	(Law2_QMIGeom_QMIPhysFromFile)
	);

/*********************************************************************************
*
* Q M   P O T E N T I A L   F R O M   F I L E   parameters    QMParametersFromFile
*
*********************************************************************************/
CREATE_LOGGER(QMParametersFromFile);
// !! at least one virtual function in the .cpp file
QMParametersFromFile::~QMParametersFromFile(){};

void QMParametersFromFile::readFileIfDidnt() {
//	std::cerr << "\nQMParametersFromFile::readFileIfDidnt\n";
//	std::cerr << "  fileLoaded = " << fileLoaded << "\n";
//	std::cerr << "  filename   = " << filename   << "\n";
//	std::cerr << "  columnX    = " << columnX    << "\n";
//	std::cerr << "  columnVal  = " << columnVal  << "\n";
	if(fileLoaded) return;
	if(filename == "" ) {
		std::cerr << "WARNING: can't read file with this EMPTY name: \"" << filename << "\" doing nothing.\n";
		return;
	};
	std::ifstream file;
	std::string line;
	std::vector<Real> row;
	std::cerr << "Reading potential data from file: \""<< filename << "\"\n";
	std::vector<std::vector<Real> > fileData;
	file.open(filename);
	if(file.is_open()) {
		while (getline (file, line)) {
			if(line.size() == 0) continue;
			size_t pos=0;
			while( (pos+1) < line.size() and line[pos]==' ') { pos++; }; // skip ' ' at start of the line
			if(line[pos] == '#') continue;                               // skip commented lines
			std::stringstream ss(line);
			while(not ss.eof()) {
				Real val=0;
				ss >> val;
				// if(not ss.eof())   // didn't work: it skipped adding numbers if there wasn't en extra empty character after last number
				//                       so instead we have an extra column with '0'
				row.push_back(val);
			}
			fileData.push_back(row);
			row.clear();
			//cout << line << endl;
		}
		file.close();
	} else {
		std::cerr << "ERROR: can't read file with this name: \"" << filename << "\"\n";
		exit(1);
	}
// // print fileData for debugging ...
//	for(size_t i=0 ; i < fileData.size() ; i++) {
//		for(size_t j=0 ; j < fileData[i].size() ; j++) {
//			std::cout << fileData[i][j] << " ";
//		}
//		std::cout << "\n";
//	}

	// now extract the interesting X,val information
	fileDataX.clear();
	fileDataVal.clear();
	int  _col_x  = columnX   -1;
	int  _val_y  = columnVal -1;
	for(size_t i=0 ; i < fileData.size() ; i++) {
		if(_col_x >= 0 and _col_x < (int)fileData[i].size()  and  _val_y >=0 and _val_y < (int)fileData[i].size() ) {
			fileDataX  .push_back( fileData[i][_col_x] );
			fileDataVal.push_back( fileData[i][_val_y] );
		} else {
			std::cerr << "ERROR 2 in  QMParametersFromFile::readFileIfDidnt    columnX   = " << _col_x+1 << " is wrong\n";
			std::cerr << "        or  QMParametersFromFile::readFileIfDidnt    columnVal = " << _val_y+1 << " is wrong.\n";
		}
	}
	fileLoaded=true;
};

/*********************************************************************************
*
* Q M   S T A T E    F R O M   F I L E                       QMStPotentialFromFile
*
*********************************************************************************/
CREATE_LOGGER(QMStPotentialFromFile);
// !! at least one virtual function in the .cpp file
QMStPotentialFromFile::~QMStPotentialFromFile(){};

/*********************************************************************************
*
* Q M   S T A T E    F R O M   F I L E   F U N C T O R       QMStPotentialFromFile
*
*********************************************************************************/

Complexr St1_QMStPotentialFromFile::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	//const QMStPotentialFromFile*        state = static_cast <const QMStPotentialFromFile*>(qms);
	QMParametersFromFile* barrier = const_cast<QMParametersFromFile*>(dynamic_cast<const QMParametersFromFile*>(pm));
	if(not barrier) { throw std::runtime_error("\n\nERROR: St1_QMStPotentialFromFile::getValPos() nas no QMParametersFromFile, but rather `"
		+std::string(pm?pm->getClassName():"")+"`.\n\n");};

//	std::cerr << barrier->fileData.size() << "," << barrier->filename << "," << barrier->columnX << "," << barrier->columnVal <<"\n";

	barrier->readFileIfDidnt();

	Real x   = pos[0]                  ,y =pos[1]                  ,z =pos[2];
	// to teraz interpolujemy
	int idx_row = 0;
	while(  (idx_row < (int)barrier->fileDataX.size())  and  (barrier->fileDataX[idx_row] < x) ) { idx_row++; };
	if(idx_row >=  (int)barrier->fileDataX.size() ) idx_row = barrier->fileDataX.size()-1;
	if((idx_row < 0) or (idx_row >= (int)barrier->fileDataX.size() ) ) {
		std::cerr << "ERROR 1 in St1_QMStPotentialFromFile::getValPos     ( idx_row < 0 ) or  ( idx_row >= barrier->fileData.size() )\n";
		exit(1);
	}

	if(idx_row == (int)(barrier->fileDataX.size()-1) ) return barrier->fileDataVal[idx_row]; // after end of potential
	if(idx_row == 0                                  ) return barrier->fileDataVal[idx_row]; // before start of potential

	Real x_before  = barrier->fileDataX[idx_row-1];
	Real x_after   = barrier->fileDataX[idx_row  ];
	if(not( x >= x_before and x <= x_after)) {
		std::cerr << "ERROR 2  nonsense error  not( x > x_before and x < x_after)\n";
		exit(1);
	}
	// row 'between lines' that will be used for interpolating the value
	Real x_idx_row = (x - x_before)/(x_after - x_before) + idx_row -1;

	if(x_idx_row >= 3 and x_idx_row <  barrier->fileDataX.size()-3) { // Spline6Interpolate is possible
		return spline6InterpolatePoint1D<Real>(barrier->fileDataVal,x_idx_row);
		//return sinc16InterpolatePoint1D <Real>(barrier->fileDataVal,x_idx_row);
	} else { // must use linear interpolation
		// return barrier->fileDataVal[idx_row];//barrier->height;                    // to jest linijka która daje wynik bez interpolowania.
		Real val_before = barrier->fileDataVal[idx_row-1];
		Real val_after  = barrier->fileDataVal[idx_row  ];
		Real t = (x - x_before)/(x_after - x_before);
		// return linearInterpolatePoint1D<Real>(barrier->fileDataVal,x_idx_row);	// FIXME wrzucić to do  linearInterpolatePoint1D in yade/lib/smoothing/
		return (1-t)*val_before + t*val_after;
	}
};

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   F R O M   F I L E        QMIPhysFromFile
*
*********************************************************************************/
CREATE_LOGGER(QMIPhysFromFile);
// !! at least one virtual function in the .cpp file
QMIPhysFromFile::~QMIPhysFromFile(){};

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersFromFile  →  QMIPhysFromFile
*
*********************************************************************************/

CREATE_LOGGER(Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile);

void Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile::go(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitF.messageAllowed(12)) std::cerr << "####### Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile::go  START!\n";

	shared_ptr<QMIPhysFromFile> pot;
	bool isNew = !I->phys;
	if(!isNew) pot=YADE_PTR_CAST<QMIPhysFromFile>(I->phys);
	else { pot=shared_ptr<QMIPhysFromFile>(new QMIPhysFromFile()); I->phys=pot; }

	Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);

	//const QMParameters*        qm1 = static_cast<QMParameters*       >(m1.get());
	const QMParametersFromFile* qm2 = static_cast<QMParametersFromFile*>(m2.get());

// FIXME: it's only for display, so this should go to Gl1_QMIGeom or Gl1_QMIGeomHarmonic (?) or Gl1_QMIPhys or Gl1_QMIPhysHarmonic
//        but then - the potential itself shall be drawn just like before: as a Box ??
	// pot->height = qm2->height;
	pot->filename    = qm2->filename;
	pot->columnX     = qm2->columnX;
	pot->columnVal   = qm2->columnVal;
	pot->fileLoaded  = qm2->fileLoaded;
	pot->fileDataX   = qm2->fileDataX;   // FIXME - ojej kopiuje!
	pot->fileDataVal = qm2->fileDataVal; // FIXME - ojej kopiuje!

// FIXME: create here QMIPhys::potentialInteractionGlobal
//        then call 
//		Ip2_2xQMParameters_QMIPhys::go(m1,m2,I);
//        to assign (or calcMarginalDistribution??? ← no, better leave that for Gl1_drawer), so I don't actually need to assign!
//		barrier->potentialInteractionGlobal->psiGlobalTable = val;

}

void Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile::goReverse(
	  const shared_ptr<Material>& m1, const shared_ptr<Material>& m2
	, const shared_ptr<Interaction>& I)
{
	if(timeLimitF.messageAllowed(12)) std::cerr << "####### Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile::goReverse  START!\n";
	I->swapOrder();
	go(m2,m1,I);
};

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysFromFile  :  F R O M   F I L E
*
*********************************************************************************/

#include <Eigen/Dense>
#include <Eigen/src/Core/Functors.h>

CREATE_LOGGER(Law2_QMIGeom_QMIPhysFromFile);

bool Law2_QMIGeom_QMIPhysFromFile::go(shared_ptr<IGeom>& ig, shared_ptr<IPhys>& ip, Interaction* I)
{
	if(timeLimitF.messageAllowed(12)) std::cerr << "####### Law2_QMIGeom_QMIPhysFromFile::go  START!\n";
	
	QMIGeom*        qmigeom = static_cast<QMIGeom*       >(ig.get());
	QMIPhysFromFile* barrier = static_cast<QMIPhysFromFile*>(ip.get());

	// FIXME, but how?? I need this equation somehow.
	QMParametersFromFile FIXME_param;
	FIXME_param.dim=barrier->dim; FIXME_param.hbar = barrier->hbar; 
if(FIXME_param.dim != 1) {
	std::cerr << "\nERROR: only 1D potentials from filenames are working now\n\n";
	exit(1);
};
	FIXME_param.filename    = barrier->filename;
	FIXME_param.columnX     = barrier->columnX;
	FIXME_param.columnVal   = barrier->columnVal;
	FIXME_param.fileLoaded  = barrier->fileLoaded;
	FIXME_param.fileDataX   = barrier->fileDataX;   // FIXME - o rany, kopiuję całą tablicę !!
	FIXME_param.fileDataVal = barrier->fileDataVal; // FIXME - o rany, kopiuję całą tablicę !!
	St1_QMStPotentialFromFile FIXME_equation;

	//FIXME - how to avoid getting Body from scene?
	QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>((*(scene->bodies))[I->id1]->state.get());
HERE;
//	NDimTable<Complexr>& val(qmigeom->potentialMarginalDistribution);

	if(not  barrier->potentialInteractionGlobal) {
		barrier->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
		barrier->potentialInteractionGlobal->wasGenerated=false;
	}
		// FIXME ! - tensorProduct !!
HERE;
	NDimTable<Complexr>& val(barrier->potentialInteractionGlobal->psiGlobalTable);




	if(psi->gridSize.size() <= 3) {
// FIXME (1↓) problem zaczyna się tutaj, ponieważ robiąc resize tak żeby pasowały do siebie, zakładam jednocześnie że siatki się idealnie nakrywają.
//            hmm... ale nawet gdy mam iloczyn tensorowy to one muszą się idealnie nakrywać !
		val.resize(psi->getPsiGlobalExisting()->psiGlobalTable);
		val.fill1WithFunction( psi->gridSize.size()
			, [&](Real i, int d)->Real    { return psi->iToX(i,d) - qmigeom->relPos21[d];}           // xyz position function
			, [&](Vector3r& xyz)->Complexr{
				//Vector3r tmp = xyz.cwiseAbs() - qmigeom->extents2;
				//Vector3r zero(0,0,0);
				//if( tmp[0] <= 0 and tmp[1] <=0 and tmp[1] <= 0)
				// http://eigen.tuxfamily.org/dox/group__TutorialReductionsVisitorsBroadcasting.html
				if(  ((xyz.cwiseAbs() - qmigeom->extents2).array() <= Vector3r::Zero().array()).count() == 3  )
					return FIXME_equation.getValPos(xyz,&FIXME_param,NULL);                  // function value at xyz
				return 0;
			  }
			);
	} else { std::cerr << "\nLaw2_QMIGeom_QMIPhysFromFile::go, dim>3\n"; exit(1); };

// FIXME - this should go to Ip2_::go (parent, toplevel)
/*FIXME*///	if(not barrier->potentialInteractionGlobal)
/*FIXME*///		barrier->potentialInteractionGlobal = boost::shared_ptr<QMStateDiscreteGlobal>(new QMStateDiscreteGlobal);
/*FIXME*///		// FIXME ! - tensorProduct !!
/*FIXME*///	barrier->potentialInteractionGlobal->psiGlobalTable = val;

	return true;
};

