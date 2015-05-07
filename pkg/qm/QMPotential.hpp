// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <lib/base/NDimTable.hpp>
#include <core/IGeom.hpp>
#include <core/IPhys.hpp>

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   G E O M E T R Y
*
*********************************************************************************/

/*! @brief QMPotGeometry is the geometrical data about interaction happening between two particles.
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
 */

class QMPotGeometry: public IGeom
{
	public:
		virtual ~QMPotGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMPotGeometry /* class name */, IGeom /* base class */
			, "QMPotGeometry is the geometrical data about interaction happening between two particles" // class description
			, // attributes, public variables
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
		REGISTER_CLASS_INDEX(QMPotGeometry,IGeom);

// FIXME (!1)	/*FIXME? Complexr*/ Real valAtIdx(NDimTable<Real>::DimN    idx){};
// FIXME (!1)	/*FIXME? Complexr*/ Real valAtPos(NDimTable<Real>::DimReal pos){};


// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, wywalić to, a potem zrobić generację tego w Law2 używając
//        gridSize1, gridSize2, size1, size2. A potem nawet używając względne obroty! przecież współrzędne położeniowe są niezależne dla każdej cząstki
//        więc FFT nie przejmuje się tym jak one są obrócone w przestrzeni. Tylko żeby centrum było w zerze, bo z tym nie jestem pewien co mogę zrobić
		NDimTable<Complexr> potentialValues;     // Discrete values of potential

// FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, FIXME, wywalić to, a potem zrobić generację tego w Law2 używając
//
//        kolejna sprawa - gdzie jest addForce i addTorque ? Bo tam powienien być addPotential !
//
};
REGISTER_SERIALIZABLE(QMPotGeometry);

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   P H Y S I C S
*
*********************************************************************************/

/*! @brief QMPotPhysics is the physical parameters concerning interaction happening between two particles.
 *
 * Examples include: electric charge for Coulomb interaction or height of potential barrier
 *
 * Evolution of interaction is governed by children of Law2_QMPotGeometry_QMPotPhysics_QMStateDiscreteGlobal:
 * Those derived classes include hamiltonian elements for each interaction.
 */

class QMPotPhysics: public IPhys
{
	public:
		virtual ~QMPotPhysics();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY( QMPotPhysics /* class name */, IPhys /* base class */
			, "QMPotPhysics is the physical parameters concerning interaction happening between two particles" // class description
			, // attributes, public variables
			, // constructor
			  createIndex();
			, // python bindings
		);
	DECLARE_LOGGER;
	REGISTER_CLASS_INDEX(QMPotPhysics,IPhys);
};
REGISTER_SERIALIZABLE(QMPotPhysics);

/*********************************************************************************
*
* I G 2   B O X   W A V E F U N C T I O N   I N T E R A C T I O N                 (creates geometry of two overlapping entities)
*
*********************************************************************************/

/*! @brief When QMGeometry collides with a Box (with potential) the
 * geometry of their contact is calculated and stored in QMPotGeometry
 *
 */

#include <pkg/common/Dispatching.hpp>
#include <pkg/common/Box.hpp>
#include "QMGeometry.hpp"

class Ig2_Box_QMGeometry_QMPotGeometry : public IGeomFunctor
{
	public :
		virtual bool go(const shared_ptr<Shape>& qm1, const shared_ptr<Shape>& qm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
		virtual bool goReverse(	const shared_ptr<Shape>& qm1, const shared_ptr<Shape>& qm2, const State& state1, const State& state2, const Vector3r& shift2, const bool& force, const shared_ptr<Interaction>& c);
	YADE_CLASS_BASE_DOC(Ig2_Box_QMGeometry_QMPotGeometry,IGeomFunctor,"Create an interaction geometry :yref:`QMPotGeometry` from :yref:`Box` and :yref:`QMGeometry`, representing the box overlapped onto wavefunction in positional representation.")
	FUNCTOR2D(Box,QMGeometry);
	DEFINE_FUNCTOR_ORDER_2D(Box,QMGeometry);
};

REGISTER_SERIALIZABLE(Ig2_Box_QMGeometry_QMPotGeometry);

