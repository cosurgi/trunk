// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <lib/base/NDimTable.hpp>
#include <core/IGeom.hpp>
#include <core/IPhys.hpp>
#include <pkg/common/Box.hpp>
#include "QMGeometry.hpp"
#include "QMParameters.hpp"
#include "QMStateAnalytic.hpp"

/*********************************************************************************
*
* Q M   S T A T E    P O T E N T I A L                            QMStatePotential
*
*********************************************************************************/

/*! @brief QMStatePotential stores parameters for a potential. It can evolve with time using `QMStateAnalytic::t`
 */

class QMStatePotential: public QMStateAnalytic
{
	public:
		virtual ~QMStatePotential();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(	QMStatePotential /* class name*/, QMStateAnalytic /* base class */
			, "It's an unmovable potential: a source of potential (interaction)" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
			  blockedDOFs=State::DOF_ALL; // unmovable, has DOFs blocked
	);
	REGISTER_CLASS_INDEX(QMStatePotential,QMStateAnalytic);
};
REGISTER_SERIALIZABLE(QMStatePotential);


/*********************************************************************************
*
* Q M   I N T E R A C T I O N   potential   G E O M E T R Y                QMIGeom
*
*********************************************************************************/

/*! @brief QMIGeom is the geometrical data about interaction happening between two particles.
 *
 * Namely, since the interaction happens on an FFT grid, the `gridSize` and and `positional` size of particle1 and particle2
 * must be added to the interaction data, so that tensor product will be performed correctly.
 *
 * Also it must store the information whether one of the participants has no degrees of freedom `gridSize1.size() == 0`, and there's no
 * need to actually make the tensor product, just use the potential.
 *
 * The particles 'size' nor 'gridSize' do not change. The tensor product is performed respecting this data.
 * Just like single particle can have different step(0) and step(1), the interactions between them can accomodate, and
 * their spatial ranges and `gridSize`s respect the particle's size and gridSize. They can be different.
 *
 *  Member variables:
 *
 *    Vector3r        extents1    → Half-size of the 1st particle
 *    Vector3r        extents2    → Half-size of the 2nd particle
 *    Vector3r        relPos21    → Relative position of two boxes with wavefunctions or potentials
 *    FIXME:          relOri21    → Relative orientation of two boxes with wavefunctions or potentials.
 *    vector<Real>    size1       → 1st wavepacket size in position representation space
 *    vector<Real>    size2       → 1st wavepacket size in position representation space
 *    vector<size_t>  gridSize1   → Lattice grid size used to describe the 1st wavefunction
 *    vector<size_t>  gridSize1   → Lattice grid size used to describe the 2nd wavefunction
 *
 */

class QMIGeom: public IGeom
{
	public:
		virtual ~QMIGeom();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIGeom /* class name */, IGeom /* base class */
			, "QMIGeom is the geometrical data about interaction happening between two particles" // class description
			, // attributes, public variables
			  ((Vector3r,extents1,,,"Half-size of the 1st particle"))
			  ((Vector3r,extents2,,,"Half-size of the 2nd particle"))
			  ((Vector3r,relPos21,,,"Relative position of two boxes with wavefunctions or potentials."))
// FIXME:		  ((        ,relOri21,,,"Relative orientation of two boxes with wavefunctions or potentials."))
			  ((vector<Real>,size1,vector<Real>({}),,"1st wavepacket size in position representation space"))
			  ((vector<Real>,size2,vector<Real>({}),,"1st wavepacket size in position representation space"))
			  ((vector<size_t>,gridSize1,vector<size_t>({}),,"Lattice grid size used to describe the 1st wavefunction."))
			  ((vector<size_t>,gridSize2,vector<size_t>({}),,"Lattice grid size used to describe the 2nd wavefunction."))

// Compare in history, very long time ago:  InteractingBox2InteractingBox4ClosestFeatures.cpp (git lof, git lol -p)
//			((Vector3r , relativePosition21    , ,, "Relative position    of two boxes with wavefunctions or potentials."))
//			((         , relativeOrientation21 , ,, "Relative orientation of two boxes with wavefunctions or potentials."))
//			((Vector3r , halfSize1             , ,, "Size of 1st box."))
//			((Vector3r , halfSize2             , ,, "Size of 2nd box."))
//			((vector<size_t>, gridSize1        , ,, "gridSize of 1st box."))
//			((vector<size_t>, gridSize2        , ,, "gridSize of 2nd box."))
			, // constructor
			  createIndex();
			, // python bindings
// FIXME (!1) - add ability to read potential values at given i,j,k,l,... coord
// FIXME (!1)	.def("valAtIdx"     ,&QMStateDiscrete::valAtIdx     ,"Get potential value at coord idx, invoke for example: valAtIdx([10,20]) # for 2D")
// FIXME (!1)	.def("valAtPos"     ,&QMStateDiscrete::valAtPos     ,"Get potential value at coord idx, invoke for example: valAtPos([1.0,2.2]) # for 2D")
		);
		DECLARE_LOGGER;
		REGISTER_CLASS_INDEX(QMIGeom,IGeom);

// FIXME (!1)	/*FIXME? Complexr*/ Real valAtIdx(NDimTable<Real>::DimN    idx){};
// FIXME (!1)	/*FIXME? Complexr*/ Real valAtPos(NDimTable<Real>::DimReal pos){};


// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, wywalić to, a potem zrobić generację tego w Law2 używając
//        gridSize1, gridSize2, size1, size2. A potem nawet używając względne obroty! przecież współrzędne położeniowe są niezależne dla każdej cząstki
//        więc FFT nie przejmuje się tym jak one są obrócone w przestrzeni. Tylko żeby centrum było w zerze, bo z tym nie jestem pewien co mogę zrobić


// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, wywalić to, a potem zrobić generację tego w Law2 używając
		NDimTable<Complexr> potentialValues;     // Discrete values of potential
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME
// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME

// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME
//
//        kolejna sprawa - gdzie jest addForce i addTorque ? Bo tam powienien być addPotential !
//
//           odpowiedź:   macierzyste Law2 się tym zajmie.
//
//
//
};
REGISTER_SERIALIZABLE(QMIGeom);

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   P H Y S I C S               QMIPhys
*
*********************************************************************************/

/*! @brief QMIPhys is the physical parameters concerning interaction happening between two particles.
 *
 * Examples include: electric charge for Coulomb interaction or height of potential barrier
 *
 * Evolution of interaction is governed by children of Law2_QMIGeom_QMIPhys_GlobalWavefunction:
 * Those derived classes include hamiltonian elements for each interaction.
 */

class QMIPhys: public IPhys
{
	public:
		virtual ~QMIPhys();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMIPhys /* class name */, IPhys /* base class */
			, "QMIPhys is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			((Real  ,hbar  ,1               ,Attr::readonly,"Planck's constant $h$ divided by $2\\pi$: ħ=h/(2π)"))
			((size_t,dim   ,                ,Attr::readonly,"Describes in how many dimensions this quantum particle resides. First Vector3r[0] is used, then [1], then [2]."))
			, // constructor
			createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMIPhys,IPhys);
};
REGISTER_SERIALIZABLE(QMIPhys);

/////// FIXME: delete     /*********************************************************************************
/////// FIXME: delete     *
/////// FIXME: delete     * Ig2   Box   QMGeometry  →  QMIGeom
/////// FIXME: delete     *
/////// FIXME: delete     *********************************************************************************/
/////// FIXME: delete     
/////// FIXME: delete     /*! @brief When QMGeometry collides with a Box which is a potential, the geometry of their contact is calculated and stored in QMIGeom
/////// FIXME: delete      *
/////// FIXME: delete      *  Calculates geometrical contact of external potential (bounded by a BOX) on a wavefunction (bounded by QMGeometry).
/////// FIXME: delete      */
/////// FIXME: delete     
/////// FIXME: delete     class Ig2_Box_QMGeometry_QMIGeom: public IGeomFunctor
/////// FIXME: delete     {
/////// FIXME: delete     	public :
/////// FIXME: delete     		virtual bool go(const shared_ptr<Shape>& qm1, const shared_ptr<Shape>& qm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
/////// FIXME: delete     		virtual bool goReverse(	const shared_ptr<Shape>& qm1, const shared_ptr<Shape>& qm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
/////// FIXME: delete     	YADE_CLASS_BASE_DOC(Ig2_Box_QMGeometry_QMIGeom,IGeomFunctor,"Create an interaction geometry :yref:`QMIGeom` from :yref:`Box` and :yref:`QMGeometry`, representing the box overlapped onto wavefunction in positional representation.")
/////// FIXME: delete     	FUNCTOR2D(Box,QMGeometry);
/////// FIXME: delete     	DEFINE_FUNCTOR_ORDER_2D(Box,QMGeometry);
/////// FIXME: delete     	DECLARE_LOGGER;
/////// FIXME: delete     };
/////// FIXME: delete     
/////// FIXME: delete     REGISTER_SERIALIZABLE(Ig2_Box_QMGeometry_QMIGeom);

/*********************************************************************************
*
* Ig2   QMGeometry   QMGeometry  →  QMIGeom
*
*********************************************************************************/

/*! @brief When QMGeometry collides with another QMGeometry things get interesting, because two wavefunction will get entangled.
 *
 *  The geometry of their contact is calculated and stored in QMIGeom.
 *  Their wavefunction will later be subjected to a tensor product.
 *
 *  Important note: they do not need to have same size (extents), or same gridSize, or same step()
 */

class Ig2_2xQMGeometry_QMIGeom: public IGeomFunctor
{
	public :
		virtual bool go       (const shared_ptr<Shape>&, const shared_ptr<Shape>&, const State&, const State&, const Vector3r&, const bool&, const shared_ptr<Interaction>&);
		virtual bool goReverse(const shared_ptr<Shape>&, const shared_ptr<Shape>&, const State&, const State&, const Vector3r&, const bool&, const shared_ptr<Interaction>&)
		{ throw std::logic_error("\n\nIg2_2xQMGeometry_QMIGeom::goReverse called, but the functor is symmetric.\n\n"); };
	YADE_CLASS_BASE_DOC(Ig2_2xQMGeometry_QMIGeom,IGeomFunctor
	,"Create an interaction geometry :yref:`QMIGeom` from two :yref:`QMGeometry`, with geometrical info about two quantum wavefunctions (or potentials) in positional representation.")
	FUNCTOR2D(QMGeometry,QMGeometry);
	DEFINE_FUNCTOR_ORDER_2D(QMGeometry,QMGeometry);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ig2_2xQMGeometry_QMIGeom);

/*********************************************************************************
*
* Ip2   QMParameters   QMParameters  →  QMIPhys
*
*********************************************************************************/

/*! @brief When two QMParameters collide, the must have the same Planck's constant.
 *
 *  Important note: every IPhysFunctor must call it's parent's go() method so that
 *  the higher level stuff gets taken care of.
 */

class Ip2_2xQMParameters_QMIPhys: public IPhysFunctor
{
	public:
		virtual void go       (const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&);
		virtual void goReverse(const shared_ptr<Material>&, const shared_ptr<Material>&, const shared_ptr<Interaction>&)
		{ throw std::runtime_error("\n\nIp2_2xQMParameters_QMIPhys::goReverse called, but it shouldn't be called, because it's symmetric.\n\n"); };
	YADE_CLASS_BASE_DOC(Ip2_2xQMParameters_QMIPhys,IPhysFunctor,"Create (but can't update) physical parameters of the interaction between two :yref:`QMParameters`, basically only Planck's constant and dimension.");
	FUNCTOR2D(QMParameters,QMParameters);
	DEFINE_FUNCTOR_ORDER_2D(QMParameters,QMParameters);
	DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Ip2_2xQMParameters_QMIPhys);

/*********************************************************************************
*
* Law2   QMIGeom + QMIPhys  :  G L O B A L   W A V E F U N C T I O N
*
*********************************************************************************/

/*! @brief Handles global wavefunction due to interaction between two particles
 *
 *  When two quantum objects interact, their interaction is described by QMIGeom and QMIPhys.
 *
 *  Important note: every IPhysFunctor must call it's parent's go() method so that
 *  the higher level stuff gets taken care of.        FIXME !!!!!!!!
 */

class Law2_QMIGeom_QMIPhys_GlobalWavefunction: public LawFunctor
{
	public:
		virtual bool go(shared_ptr<IGeom>&, shared_ptr<IPhys>&, Interaction*);
		FUNCTOR2D(QMIGeom,QMIPhys);
		YADE_CLASS_BASE_DOC(Law2_QMIGeom_QMIPhys_GlobalWavefunction,LawFunctor,
		"Handle quantum interaction described by :yref:`QMIGeom` and :yref:`QMIPhys`.");
		DECLARE_LOGGER;
};

REGISTER_SERIALIZABLE(Law2_QMIGeom_QMIPhys_GlobalWavefunction);

