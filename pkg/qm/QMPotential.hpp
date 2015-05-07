// 2015 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <lib/base/NDimTable.hpp>
#include <core/IGeom.hpp> 

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   G E O M E T R Y
*
*********************************************************************************/

/*! @brief QMPotGeometry is the geometrical data about interaction happening between two particles.
 *
 * Namely, since the interaction happens on an FFT grid, the `gridSize` and and `positional` size must be selected such that
 * both participating particles are covered in full.
 */

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/

/*! @brief QMPotGeometry should describe geometrical aspects of interaction between two wave functions.
 *
 * Currently I have only free moving particle without interactions, so it is not used yet.
 *
 */
class QMPotGeometry: public IGeom
{
	public:
		virtual ~QMPotGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMPotGeometry
			, // base class
			IGeom
			, // class description
			"Geometric representation of a single interaction of the WaveFunction"
			, // attributes, public variables
//			((Vector3r , relativePosition21    , ,, "Relative position    of two boxes with wavefunctions or potentials."))
//			((         , relativeOrientation21 , ,, "Relative orientation of two boxes with wavefunctions or potentials."))
//			((Vector3r , halfSize1             , ,, "Size of 1st box."))
//			((Vector3r , halfSize2             , ,, "Size of 2nd box."))
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

		NDimTable<Complexr> potentialValues;     // Discrete values of potential

};
REGISTER_SERIALIZABLE(QMPotGeometry);
