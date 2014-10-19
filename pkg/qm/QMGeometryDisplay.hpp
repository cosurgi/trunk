// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   G E O M E T R Y   D I S P L A Y
*
*********************************************************************************/

/*! @brief QMGeometryDisplay contains geometrical information used to display the particle on screen.
 *
 * Quantum particles have no geometry in classical sense. So in this class only
 * stuff neeeded for display of the wavefunction stored in state.
 *
 * Things like color, or if it's a wireframe to display, etc.
 *
 * No physical/calculations related stuff here! Only display.
 *
 */
class QMGeometryDisplay: public Shape
{
	public:
		virtual ~QMGeometryDisplay();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			QMGeometryDisplay
			, // base class
			Shape
			, // class description
			"Wave function geomterical (prepared, maybe precomputed, for display on screen) information about a particle."
			, // attributes, public variables
			((Vector3r,halfSize,,,
			"Analytical wavefunction is infinite, but computers are finite, so this is the spatial size \
			in positional representation. It is used to create :yref:`Aabb`, so it also helps fiding particle collisions. \
			If the wavefunctions were really infinite, then it would work nicely, but due to computer limitations this \
			halfSize also covers the range of interactions between particles. Later it should work in 4D space-time."))
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(QMGeometryDisplay,Shape);
};
REGISTER_SERIALIZABLE(QMGeometryDisplay);



