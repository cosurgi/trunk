// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMParameters.hpp"
#include "QMPotential.hpp"
#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* Q M   P O T E N T I A L   F R O M   F I L E   parameters    QMParametersFromFile
*
*********************************************************************************/

/*! @brief QMParametersFromFile stores parameters for a potential fromm a given filename
 */

class QMParametersFromFile: public QMParameters
{
	public:
		void readFileIfDidnt();
		virtual ~QMParametersFromFile();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMParametersFromFile /* class name*/, QMParameters /* base class */
			, "It's a potential described by a text file (only 1d for now!)" // class description
			, // attributes, public variables
			  ((std::string       ,filename   ,""   ,              ,"Potential source text file: V(columnX)=filename(columnVal)"))
			  ((int               ,columnX    ,-1   ,              ,"X coordinate for potential value, using atomic units (column count starts from 1)"))
			  ((Real              ,shiftX     ,0    ,              ,"X coordinate is shifted by this value"))
			  ((int               ,columnVal  ,-1   ,              ,"Value of potential at given X coordinate, using Hartree energy (column count starts from 1)"))
			  ((bool              ,fileLoaded ,false,Attr::readonly,"Stores information whether the file has been already loaded"))
			  //((std::vector<std::vector<Real> > ,fileData  ,  ,(Attr::hidden|Attr::readonly),"The data loaded from file"))
			  ((std::vector<Real> ,fileDataX  ,     ,Attr::readonly,"The data loaded from file, the X coordinates"))
			  ((std::vector<Real> ,fileDataVal,     ,Attr::readonly,"The data loaded from file, the Values of potential coordinates"))
			, // constructor
			  createIndex();
		);
	REGISTER_CLASS_INDEX(QMParametersFromFile,QMParameters);
};
REGISTER_SERIALIZABLE(QMParametersFromFile);

/*********************************************************************************
*
* Q M   S T A T E    F R O M   F I L E                       QMStPotentialFromFile
*
*********************************************************************************/

/*! @brief QMStPotentialFromFile is an analytical representation for potential barrier.
 *
 *  This class is empty in fact, and is present only for StateDispatcher to dispach properly and
 *  call St1_QMStPotentialFromFile to fill (FIXME) QMStateDiscreteGlobal::psiGlobalTable with its representation
 */

class QMStPotentialFromFile: public QMStatePotential
{
	public:
		virtual ~QMStPotentialFromFile();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMStPotentialFromFile /* class name*/, QMStatePotential /* base class */
			, "It's a custom potential read from file: a source of potential (interaction)" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMStPotentialFromFile,QMStatePotential);
};
REGISTER_SERIALIZABLE(QMStPotentialFromFile);

/*********************************************************************************
*
* Q M   S T A T E    F R O M   F I L E   F U N C T O R       QMStPotentialFromFile
*
*********************************************************************************/

class St1_QMStPotentialFromFile: public St1_QMStateAnalytic // FIXME - public St1_QMPotential ???????????
{
	public:
		FUNCTOR1D(QMStPotentialFromFile);
		YADE_CLASS_BASE_DOC(St1_QMStPotentialFromFile/* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMStPotentialFromFile` from :yref:`QMParametersFromFile`." // class description
		);
	private:
		friend class Law2_QMIGeom_QMIPhysFromFile;
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms);
	public: //protected??
		virtual bool changesWithTime() {return false;};
};
REGISTER_SERIALIZABLE(St1_QMStPotentialFromFile);

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   F R O M   F I L E        QMIPhysFromFile
*
*********************************************************************************/

/*! @brief QMIPhysFromFile is the physical parameters concerning interaction happening between a particle and a barrier.
 */

class QMIPhysFromFile: public QMIPhys
{
	public:
		virtual ~QMIPhysFromFile();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhysFromFile /* class name */, QMIPhys /* base class */
			, "QMIPhysFromFile is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			  ((std::string ,filename ,""  ,Attr::readonly,"Potential source text file: V(columnX)=filename(columnVal)"))
			  ((int         ,columnX  ,-1  ,Attr::readonly,"X coordinate for potential value, using atomic units"))
			  ((Real        ,shiftX   ,0   ,              ,"X coordinate is shifted by this value"))
			  ((int         ,columnVal,-1  ,Attr::readonly,"Value of potential at given X coordinate, using Hartree energy"))
			  ((bool                            ,fileLoaded,false,Attr::readonly,"Stores information whether the file has been already loaded"))
			  //((std::vector<std::vector<Real> > ,fileData  ,  ,(Attr::hidden|Attr::readonly),"The data loaded from file"))
			  ((std::vector<Real> ,fileDataX  ,     ,Attr::readonly,"The data loaded from file, the X coordinates"))
			  ((std::vector<Real> ,fileDataVal,     ,Attr::readonly,"The data loaded from file, the Values of potential coordinates"))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhysFromFile,QMIPhys);
};
REGISTER_SERIALIZABLE(QMIPhysFromFile);

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersFromFile  →  QMIPhysFromFile
*
*********************************************************************************/

/*! @brief When QMParameters collides with QMParametersFromFile, the info about barrier shape is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile,Ip2_2xQMParameters_QMIPhys,"Create (but can't update) physical parameters of the interaction between :yref:`QMParameters` and :yref:`QMParametersFromFile`, hbar, dimension (parent) + potential shape.");
	FUNCTOR2D(QMParameters,QMParametersFromFile);
	DEFINE_FUNCTOR_ORDER_2D(QMParameters,QMParametersFromFile);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParameters_QMParametersFromFile_QMIPhysFromFile);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysFromFile  :  F R O M   F I L E
*
*********************************************************************************/

/*! @brief Handles interaction between particle and a barrier
 *
 *  Mainly it just puts potential shape into the potential NDimTable.
 *
 *  Important note: every IPhysFunctor must call its parent's go() method so that
 *  the higher level stuff gets taken care of        FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysFromFile: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysFromFile,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhys`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysFromFile);

