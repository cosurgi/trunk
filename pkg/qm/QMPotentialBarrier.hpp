// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMParameters.hpp"
#include "QMPotential.hpp"
#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* Q M   P O T E N T I A L   B A R R I E R   parameters         QMParametersBarrier
*
*********************************************************************************/

/*! @brief QMParametersBarrier stores parameters for a potential barrier with given height.
 */

class QMParametersBarrier: public QMParameters
{
	public:
		virtual ~QMParametersBarrier();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMParametersBarrier /* class name*/, QMParameters /* base class */
			, "It's a potential barrier with given height" // class description
			, // attributes, public variables
			  ((Real    ,height,1               ,,"Potential height: V(r)=height"))
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMParametersBarrier,QMParameters);
};
REGISTER_SERIALIZABLE(QMParametersBarrier);

/*********************************************************************************
*
* Q M   S T A T E    B A R R I E R                            QMStPotentialBarrier
*
*********************************************************************************/

/*! @brief QMStPotentialBarrier is an analytical representation for potential barrier.
 *
 *  This class is empty in fact, and is present only for StateDispatcher to dispach properly and
 *  call St1_QMStPotentialBarrier to fill (FIXME) QMStateDiscrete::tableValuesPosition with its representation
 */

class QMStPotentialBarrier: public QMStatePotential
{
	public:
		virtual ~QMStPotentialBarrier();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMStPotentialBarrier /* class name*/, QMStatePotential /* base class */
			, "It's an unmovable potential barrier: a source of potential (interaction)" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMStPotentialBarrier,QMStatePotential);
};
REGISTER_SERIALIZABLE(QMStPotentialBarrier);

/*********************************************************************************
*
* Q M   S T A T E    B A R R I E R   F U N C T O R            QMStPotentialBarrier
*
*********************************************************************************/

class St1_QMStPotentialBarrier: public St1_QMStateAnalytic
{
	public:
		FUNCTOR1D(QMStPotentialBarrier);
		YADE_CLASS_BASE_DOC(St1_QMStPotentialBarrier/* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMStPotentialBarrier` from :yref:`QMParametersBarrier`." // class description
		);
	private:
		//! return complex quantum aplitude at given positional representation coordinates
// FIXME: it's only for display, so this should go to Gl1_QMIGeom or Gl1_QMIGeomHarmonic (?) or Gl1_QMIPhys or Gl1_QMIPhysHarmonic
/* FIXME: duplicate with Law2_* */		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms);
};
REGISTER_SERIALIZABLE(St1_QMStPotentialBarrier);

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   B A R R I E R             QMIPhysBarrier
*
*********************************************************************************/

/*! @brief QMIPhysBarrier is the physical parameters concerning interaction happening between a particle and a barrier.
 */

class QMIPhysBarrier: public QMIPhys
{
	public:
		virtual ~QMIPhysBarrier();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhysBarrier /* class name */, QMIPhys /* base class */
			, "QMIPhysBarrier is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			  ((Real    ,height,1,Attr::readonly,"Potential height: V(r)=height, initialised from QMParametersBarrier"))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhysBarrier,QMIPhys);
};
REGISTER_SERIALIZABLE(QMIPhysBarrier);

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersBarrier  →  QMIPhysBarrier
*
*********************************************************************************/

/*! @brief When QMParameters collides with QMParametersBarrier, the info about barrier height is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier,Ip2_2xQMParameters_QMIPhys,"Create (but can't update) physical parameters of the interaction between :yref:`QMParameters` and :yref:`QMParametersBarrier`, hbar, dimension (parent) + barrier potential height.");
	FUNCTOR2D(QMParameters,QMParametersBarrier);
	DEFINE_FUNCTOR_ORDER_2D(QMParameters,QMParametersBarrier);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParameters_QMParametersBarrier_QMIPhysBarrier);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysBarrier  :  B A R R I E R
*
*********************************************************************************/

/*! @brief Handles interaction between particle and a barrier
 *
 *  Mainly it just puts potential height into the potential NDimTable.
 *
 *  Important note: every IPhysFunctor must call its parent's go() method so that
 *  the higher level stuff gets taken care of        FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysBarrier: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysBarrier,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhys`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysBarrier);

