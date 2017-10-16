// 2017 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMStateAnalytic.hpp"

// HUGE FIXME - po co dublować operację ładowania z pliku ??????????????

/*********************************************************************************
*
* L O A D   P A C K E T   S H A P E   F R O M   F I L E
* 
*********************************************************************************/

/*! @brief QMParametersFromFile stores parameters for a potential fromm a given filename
 */

class QMPacketFromFile: public QMStateAnalytic
{
	public:
		void readFileIfDidnt();
		virtual ~QMPacketFromFile();
		YADE_CLASS_BASE_DOC_ATTRS_INIT_CTOR_PY(
			  // class name
			QMPacketFromFile
			, // base class
			QMStateAnalytic
			, // class description
			"QMPacketFromFile is for loading the 1d packet shape from file"
			, // attributes, public variables
			  ((std::string       ,filename   ,""   ,              ,"Potential source text file: V(columnX)=filename(columnVal)"))
			  ((int               ,columnX    ,-1   ,              ,"X coordinate for potential value, using atomic units (column count starts from 1)"))
			  ((Real              ,multX      ,1.0  ,              ,"X coordinate is multiplied by this value (before shifting it!)"))
			  ((Real              ,shiftX     ,0    ,              ,"X coordinate is shifted by this value"))
			  ((int               ,columnVal  ,-1   ,              ,"Value of potential at given X coordinate, using Hartree energy (column count starts from 1)"))
			  ((Real              ,shiftVal   ,0    ,              ,"Value is shifted by this shiftVal"))
			  ((bool              ,fileLoaded ,false,Attr::readonly,"Stores information whether the file has been already loaded"))
			  //((std::vector<std::vector<Real> > ,fileData  ,  ,(Attr::hidden|Attr::readonly),"The data loaded from file"))
			  ((std::vector<Real> ,fileDataX  ,     ,Attr::readonly,"The data loaded from file, the X coordinates"))
			  ((std::vector<Real> ,fileDataVal,     ,Attr::readonly,"The data loaded from file, the Values of potential coordinates"))
			, // additional initializers (for references)
			, // constructor
			createIndex();
			, // python bindings
		);
		REGISTER_CLASS_INDEX(QMPacketFromFile,QMStateAnalytic);
		virtual Real energy() {
			std::cerr << "Real QMPacketFromFile::energy() is Not implemented.\n";
			return 0;
		};
};
REGISTER_SERIALIZABLE(QMPacketFromFile);

/*********************************************************************************
*
* L O A D   P A C K E T   S H A P E   F R O M   F I L E   F U N C T O R 
*
*********************************************************************************/

class St1_QMPacketFromFile: public St1_QMStateAnalytic
{
	public:
		FUNCTOR1D(QMPacketFromFile);
		YADE_CLASS_BASE_DOC(St1_QMPacketFromFile /* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMPacketFromFile` from :yref:`QMParticle`." // class description
		);
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz, const QMParameters* par, const QMState* qms);
	private:
		Complexr waveFunctionValue_1D_positionRepresentation( Vector3r pos , const QMParameters* pm, const QMState* qms );
};
REGISTER_SERIALIZABLE(St1_QMPacketFromFile);

