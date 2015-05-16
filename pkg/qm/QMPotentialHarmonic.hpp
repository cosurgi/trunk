// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMParameters.hpp"
#include "QMPotential.hpp"
#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* Q M   P O T E N T I A L   H A R M O N I C   parameters      QMParametersHarmonic
*
*********************************************************************************/

/*! @brief QMParametersHarmonic stores parameters for a harmonic potential with given coefficient.
 */

class QMParametersHarmonic: public QMParameters
{
	public:
		virtual ~QMParametersHarmonic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMParametersHarmonic /* class name*/, QMParameters /* base class */
			, "It's a potential barrier with given height" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),,"Harmonic potential: V(r)=coefficient*r^2"))
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMParametersHarmonic,QMParameters);
};
REGISTER_SERIALIZABLE(QMParametersHarmonic);

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C                               QMStateHarmonic
*
*********************************************************************************/

/*! @brief QMStateHarmonic is an analytical representation for harmonic potential.
 * 
 *  This class is empty in fact, and is present only for StateDispatcher to dispach properly and
 *  call St1_QMStateHarmonic to fill (FIXME) QMStateDiscrete::tableValuesPosition with its representation
 */

class QMStateHarmonic: public QMStatePotential
{
	public:
		virtual ~QMStateHarmonic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMStateHarmonic /* class name*/, QMStatePotential /* base class */
			, "It's an unmovable potential barrier: a source of potential (interaction)" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMStateHarmonic,QMStatePotential);
};
REGISTER_SERIALIZABLE(QMStateHarmonic);

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C   F U N C T O R               QMStateHarmonic
*
*********************************************************************************/

class St1_QMStateHarmonic: public St1_QMStateAnalytic
{
	public:
		FUNCTOR1D(QMStateHarmonic);
		YADE_CLASS_BASE_DOC(St1_QMStateHarmonic/* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMStateHarmonic` from :yref:`QMParametersHarmonic`." // class description
		);
	private:
		//! return complex quantum aplitude at given positional representation coordinates
// FIXME: it's only for display, so this should go to Gl1_QMIGeom or Gl1_QMIGeomHarmonic (?) or Gl1_QMIPhys or Gl1_QMIPhysHarmonic
/* FIXME: duplicate with Law2_* */		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms);
};
REGISTER_SERIALIZABLE(St1_QMStateHarmonic);

/*********************************************************************************
*
* Q M   interaction   H A R M O N I C   P O T E N T I A L          QMIPhysHarmonic
*
*********************************************************************************/

/*! @brief QMIPhysHarmonic is the physical parameters concerning interaction happening between a particle and a harmonic potential.
 */

class QMIPhysHarmonic: public QMIPhys
{
	public:
		virtual ~QMIPhysHarmonic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhysHarmonic /* class name */, QMIPhys /* base class */
			, "QMIPhysHarmonic is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),Attr::readonly,"Harmonic potential: V(r)=coefficient*r^2, initialised from QMParametersHarmonic"))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhysHarmonic,QMIPhys);
};
REGISTER_SERIALIZABLE(QMIPhysHarmonic);

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersHarmonic  →  QMIPhysHarmonic
*
*********************************************************************************/

/*! @brief When QMParameters collides with QMParametersHarmonic, the info about potential coefficient is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic,Ip2_2xQMParameters_QMIPhys,"Create (but can't update) physical parameters of the interaction between :yref:`QMParameters` and :yref:`QMParametersHarmonic`, hbar, dimension (parent) + harmonic potential coefficient.");
	FUNCTOR2D(QMParameters,QMParametersHarmonic);
	DEFINE_FUNCTOR_ORDER_2D(QMParameters,QMParametersHarmonic);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysHarmonic  :  H A R M O N I C   P O T E N T I A L
*
*********************************************************************************/

/*! @brief Handles interaction between particle and a harmonic potential
 *
 *  Mainly it just puts potential shape into the potential NDimTable.
 *
 *  Important note: every IPhysFunctor must call its parent's go() method so that
 *  the higher level stuff gets taken care of        FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysHarmonic: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysHarmonic,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhys`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysHarmonic);

