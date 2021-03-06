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
			, "It's a harmonic potential with given coefficient" // class description
			, // attributes, public variables
			//((Real     QMParameters::hbar
			//((size_t   QMParameters::dim 
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),,"Harmonic potential: V(r)=coefficient*r^2"))
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMParametersHarmonic,QMParameters);
};
REGISTER_SERIALIZABLE(QMParametersHarmonic);

/*********************************************************************************
*
* Q M   P A R T I C L E   with   H A R M O N I C   ,,charge''   QMParticleHarmonic
*
*********************************************************************************/

/*! @brief QMParticleHarmonic is a dynamic particle that is able to connect with other QMParticleHarmonic via harmonic interaction.
 *
 *  This class is only added for dispatcher to work correctly, and distinguish them.
 *  FIXME  maybe I can distinguish them later somehow in Ip2_* and Ig2_*, without introducing an empty class for that.
 */

class QMParticleHarmonic: public QMParticle
{
	public:
		virtual ~QMParticleHarmonic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMParticleHarmonic /* class name*/, QMParticle /* base class */
			, "It's a dynamic particle that is able to connect with other QMParticleHarmonic via harmonic interaction" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),,"Harmonic potential: V(r)=coefficient*r^2"))
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMParticleHarmonic,QMParticle);
};
REGISTER_SERIALIZABLE(QMParticleHarmonic);

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C                         QMStPotentialHarmonic
*
*********************************************************************************/

/*! @brief QMStPotentialHarmonic is an analytical representation for harmonic potential.
 * 
 *  This class is empty in fact, and is present only for StateDispatcher to dispach properly and
 *  call St1_QMStPotentialHarmonic to fill (FIXME) QMStateDiscreteGlobal::psiGlobalTable(Potential) with its representation
 */

class QMStPotentialHarmonic: public QMStatePotential
{
	public:
		virtual ~QMStPotentialHarmonic();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMStPotentialHarmonic /* class name*/, QMStatePotential /* base class */
			, "It's an unmovable potential barrier: a source of potential (interaction)" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMStPotentialHarmonic,QMStatePotential);
};
REGISTER_SERIALIZABLE(QMStPotentialHarmonic);

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C   F U N C T O R         QMStPotentialHarmonic
*
*********************************************************************************/

class St1_QMStPotentialHarmonic: public St1_QMStateAnalytic // FIXME - public St1_QMPotential ???????????
{
	public:
		FUNCTOR1D(QMStPotentialHarmonic);
		YADE_CLASS_BASE_DOC(St1_QMStPotentialHarmonic/* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMStPotentialHarmonic` from :yref:`QMParametersHarmonic`." // class description
		);
		friend class Law2_QMIGeom_QMIPhysHarmonic;
		friend class Law2_QMIGeom_QMIPhysHarmonicParticles;
	private:
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms);
	public: //protected??
		virtual bool changesWithTime() {return false;};
};
REGISTER_SERIALIZABLE(St1_QMStPotentialHarmonic);

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
			, "QMIPhysHarmonic is the physical parameters concerning interaction happening between particle and a harmonic potential" // class description
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
* Q M particle interaction via harmonic potential         QMIPhysHarmonicParticles
*
*********************************************************************************/

/*! @brief QMIPhysHarmonicParticles is the physical parameters concerning interaction happening between a particle and a harmonic potential.
 */

class QMIPhysHarmonicParticles: public QMIPhys
{
	public:
		virtual ~QMIPhysHarmonicParticles();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhysHarmonicParticles /* class name */, QMIPhys /* base class */
			, "QMIPhysHarmonicParticles is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient1,Vector3r(0.5,0.5,0.5),Attr::readonly,"Harmonic potential: V(r)=coefficient1*r^2, initialised from QMParticleHarmonic"))
			  ((Vector3r,coefficient2,Vector3r(0.5,0.5,0.5),Attr::readonly,"Harmonic potential: V(r)=coefficient2*r^2, initialised from QMParticleHarmonic"))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhysHarmonicParticles,QMIPhys);
};
REGISTER_SERIALIZABLE(QMIPhysHarmonicParticles);

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
	YADE_CLASS_BASE_DOC(Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic,Ip2_2xQMParameters_QMIPhys
	  ,"Create (or update) physical parameters of the interaction between :yref:`QMParameters` and :yref:`QMParametersHarmonic`, hbar, dimension (parent) + harmonic potential coefficient.");
	FUNCTOR2D(QMParameters,QMParametersHarmonic);
	DEFINE_FUNCTOR_ORDER_2D(QMParameters,QMParametersHarmonic);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParameters_QMParametersHarmonic_QMIPhysHarmonic);

/*********************************************************************************
*
* Ip2   QMParticleHarmonic   QMParticleHarmonic  →  QMIPhysHarmonicParticles
*
*********************************************************************************/

/*! @brief When two QMParticleHarmonic collide, the info about potential coefficients is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles,Ip2_2xQMParameters_QMIPhys
	  ,"Create (or update) physical parameters of the interaction between two :yref:`QMParticleHarmonic`, hbar, dimension (parent) + harmonic potentials coefficients.");
	FUNCTOR2D(QMParticleHarmonic,QMParticleHarmonic);
	DEFINE_FUNCTOR_ORDER_2D(QMParticleHarmonic,QMParticleHarmonic);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParticleHarmonic_QMParticleHarmonic_QMIPhysHarmonicParticles);

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
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhysHarmonic`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysHarmonic);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysHarmonicParticles  :  H A R M O N I C   I N T E R A C T I O N
*
*********************************************************************************/

/*! @brief Handles interaction between two particles via harmonic potential
 *
 *  Mainly it prepares the entangled wavefunction in QMStateDiscreteGlobal and calculates the interaction potential QMStateDiscreteGlobal::allPotentials
 *
 *  Important note: every IPhysFunctor must call its parent's go() method so that
 *  the higher level stuff gets taken care of        FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysHarmonicParticles: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysHarmonicParticles,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhysHarmonicParticles`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysHarmonicParticles);

