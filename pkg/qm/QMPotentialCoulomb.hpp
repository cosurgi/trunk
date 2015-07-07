// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include "QMParameters.hpp"
#include "QMPotential.hpp"
#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* Q M   P O T E N T I A L   H A R M O N I C   parameters      QMParametersCoulomb
*
*********************************************************************************/

/*! @brief QMParametersCoulomb stores parameters for a harmonic potential with given coefficient.
 */

class QMParametersCoulomb: public QMParameters
{
	public:
		virtual ~QMParametersCoulomb();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMParametersCoulomb /* class name*/, QMParameters /* base class */
			, "It's a potential barrier with given height" // class description
			, // attributes, public variables
			//((Real     QMParameters::hbar
			//((size_t   QMParameters::dim 
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),,"Coulomb potential: V(r)=coefficient/r"))
			  ((Real    ,potentialMaximum,100,,"Coulomb potential limit, to prevent division by zero. This value is a value of maximum allowed potential. When it is positive, then this value is used (times coefficient), when it is negative then ZERO is used."))
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMParametersCoulomb,QMParameters);
};
REGISTER_SERIALIZABLE(QMParametersCoulomb);

/*********************************************************************************
*
* Q M   P A R T I C L E   with   H A R M O N I C   ,,charge''   QMParticleCoulomb
*
*********************************************************************************/

/*! @brief QMParticleCoulomb is a dynamic particle that is able to connect with other QMParticleCoulomb via harmonic interaction.
 *
 *  This class is only added for dispatcher to work correctly, and distinguish them.
 *  FIXME  maybe I can distinguish them later somehow in Ip2_* and Ig2_*, without introducing an empty class for that.
 */

class QMParticleCoulomb: public QMParticle
{
	public:
		virtual ~QMParticleCoulomb();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMParticleCoulomb /* class name*/, QMParticle /* base class */
			, "It's a dynamic particle that is able to connect with other QMParticleCoulomb via harmonic interaction" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),,"Coulomb potential: V(r)=coefficient/r"))
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMParticleCoulomb,QMParticle);
};
REGISTER_SERIALIZABLE(QMParticleCoulomb);

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C                         QMStPotentialCoulomb
*
*********************************************************************************/

/*! @brief QMStPotentialCoulomb is an analytical representation for harmonic potential.
 * 
 *  This class is empty in fact, and is present only for StateDispatcher to dispach properly and
 *  call St1_QMStPotentialCoulomb to fill (FIXME) QMStateDiscreteGlobal::psiGlobalTable(Potential) with its representation
 */

class QMStPotentialCoulomb: public QMStatePotential
{
	public:
		virtual ~QMStPotentialCoulomb();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMStPotentialCoulomb /* class name*/, QMStatePotential /* base class */
			, "It's an unmovable potential barrier: a source of potential (interaction)" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
	);
	REGISTER_CLASS_INDEX(QMStPotentialCoulomb,QMStatePotential);
};
REGISTER_SERIALIZABLE(QMStPotentialCoulomb);

/*********************************************************************************
*
* Q M   S T A T E    H A R M O N I C   F U N C T O R         QMStPotentialCoulomb
*
*********************************************************************************/

class St1_QMStPotentialCoulomb: public St1_QMStateAnalytic
{
	public:
		FUNCTOR1D(QMStPotentialCoulomb);
		YADE_CLASS_BASE_DOC(St1_QMStPotentialCoulomb/* class name */, St1_QMStateAnalytic /* base class */
			, "Functor creating :yref:`QMStPotentialCoulomb` from :yref:`QMParametersCoulomb`." // class description
		);
		friend class Law2_QMIGeom_QMIPhysCoulomb;
		friend class Law2_QMIGeom_QMIPhysCoulombParticles;
		friend class Law2_QMIGeom_QMIPhysCoulombParticlesFree;
	private:
		//! return complex quantum aplitude at given positional representation coordinates
		virtual Complexr getValPos(Vector3r xyz , const QMParameters* par, const QMState* qms);
};
REGISTER_SERIALIZABLE(St1_QMStPotentialCoulomb);

/*********************************************************************************
*
* Q M   interaction   H A R M O N I C   P O T E N T I A L          QMIPhysCoulomb
*
*********************************************************************************/

/*! @brief QMIPhysCoulomb is the physical parameters concerning interaction happening between a particle and a harmonic potential.
 */

class QMIPhysCoulomb: public QMIPhys
{
	public:
		virtual ~QMIPhysCoulomb();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhysCoulomb /* class name */, QMIPhys /* base class */
			, "QMIPhysCoulomb is the physical parameters concerning interaction happening between particle and a harmonic potential" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient,Vector3r(0.5,0.5,0.5),Attr::readonly,"Coulomb potential: V(r)=coefficient/r, initialised from QMParametersCoulomb"))
			  ((Real    ,potentialMaximum,100,Attr::readonly,"Coulomb potential limit, to prevent division by zero"))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhysCoulomb,QMIPhys);
};
REGISTER_SERIALIZABLE(QMIPhysCoulomb);

/*********************************************************************************
*
* Q M particle interaction via harmonic potential         QMIPhysCoulombParticles
*
*********************************************************************************/

/*! @brief QMIPhysCoulombParticles is the physical parameters concerning interaction happening between a particle and a harmonic potential.
 */

class QMIPhysCoulombParticles: public QMIPhys
{
	public:
		virtual ~QMIPhysCoulombParticles();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhysCoulombParticles /* class name */, QMIPhys /* base class */
			, "QMIPhysCoulombParticles is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			  ((Vector3r,coefficient1,Vector3r(0.5,0.5,0.5),Attr::readonly,"Coulomb potential: V(r)=coefficient1/r, initialised from QMParticleCoulomb"))
			  ((Vector3r,coefficient2,Vector3r(0.5,0.5,0.5),Attr::readonly,"Coulomb potential: V(r)=coefficient2/r, initialised from QMParticleCoulomb"))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhysCoulombParticles,QMIPhys);
};
REGISTER_SERIALIZABLE(QMIPhysCoulombParticles);

/*********************************************************************************
*
* Ip2   QMParameters   QMParametersCoulomb  →  QMIPhysCoulomb
*
*********************************************************************************/

/*! @brief When QMParameters collides with QMParametersCoulomb, the info about potential coefficient is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb,Ip2_2xQMParameters_QMIPhys
	  ,"Create (or update) physical parameters of the interaction between :yref:`QMParameters` and :yref:`QMParametersCoulomb`, hbar, dimension (parent) + harmonic potential coefficient.");
	FUNCTOR2D(QMParameters,QMParametersCoulomb);
	DEFINE_FUNCTOR_ORDER_2D(QMParameters,QMParametersCoulomb);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParameters_QMParametersCoulomb_QMIPhysCoulomb);

/*********************************************************************************
*
* Ip2   QMParticleCoulomb   QMParticleCoulomb  →  QMIPhysCoulombParticles
*
*********************************************************************************/

/*! @brief When two QMParticleCoulomb collide, the info about potential coefficients is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles,Ip2_2xQMParameters_QMIPhys
	  ,"Create (or update) physical parameters of the interaction between two :yref:`QMParticleCoulomb`, hbar, dimension (parent) + harmonic potentials coefficients.");
	FUNCTOR2D(QMParticleCoulomb,QMParticleCoulomb);
	DEFINE_FUNCTOR_ORDER_2D(QMParticleCoulomb,QMParticleCoulomb);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParticleCoulomb_QMParticleCoulomb_QMIPhysCoulombParticles);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulomb  :  H A R M O N I C   P O T E N T I A L
*
*********************************************************************************/

/*! @brief Handles interaction between particle and a harmonic potential
 *
 *  Mainly it just puts potential shape into the potential NDimTable.
 *
 *  Important note: every IPhysFunctor must call its parent's go() method so that
 *  the higher level stuff gets taken care of        FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysCoulomb: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysCoulomb,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhysCoulomb`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysCoulomb);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulombParticles  :  H A R M O N I C   I N T E R A C T I O N
*
*********************************************************************************/

/*! @brief Handles interaction between two particles via harmonic potential
 *
 *  Mainly it prepares the entangled wavefunction in QMStateDiscreteGlobal and calculates the interaction potential QMStateDiscreteGlobal::allPotentials
 *
 *  Important note: every IPhysFunctor must call its parent's go() method so that
 *  the higher level stuff gets taken care of        FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysCoulombParticles: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysCoulombParticles,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhysCoulombParticles`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysCoulombParticles);

/*********************************************************************************
*
* Ip2   QMParticleCoulomb   QMParametersCoulomb  →  QMIPhysCoulombParticles
*
*********************************************************************************/

/*! @brief When two QMParticleCoulomb collide, the info about potential coefficients is needed
 *
 *  Important note: every Ip2_2xQMParameters_QMIPhys must call its parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles: public Ip2_2xQMParameters_QMIPhys
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
	YADE_CLASS_BASE_DOC(Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles,Ip2_2xQMParameters_QMIPhys
	  ,"Create (or update) physical parameters of the interaction between two :yref:`QMParticleCoulomb`, hbar, dimension (parent) + harmonic potentials coefficients.");
	FUNCTOR2D(QMParticleCoulomb,QMParametersCoulomb);
	DEFINE_FUNCTOR_ORDER_2D(QMParticleCoulomb,QMParametersCoulomb);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_QMParticleCoulomb_QMParametersCoulomb_QMIPhysCoulombParticles);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhysCoulombParticles  :  freeely propagating hydrogen
*
*********************************************************************************/

/*! @brief .....
 *
 *  FIXME !!!!!!!!.
 */

class Law2_QMIGeom_QMIPhysCoulombParticlesFree: public Law2_QMIGeom_QMIPhys_GlobalWavefunction
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhysCoulombParticlesFree,Law2_QMIGeom_QMIPhys_GlobalWavefunction,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhysCoulombParticles`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhysCoulombParticlesFree);

