// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   G E O M E T R Y   D I S P L A Y
*
*********************************************************************************/

/*! @brief QuantumMechanicalGeometryDisplay contains geometrical information used to display the particle on screen
 *
 * Things like color, or if it's a wireframe to display, etc.
 *
 * No physical/calculations related stuff here! Only display.
 *
 */
class QuantumMechanicalGeometryDisplay: public Shape
{
	public:
		virtual ~QuantumMechanicalGeometryDisplay();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			QuantumMechanicalGeometryDisplay
			, // base class
			Shape
			, // class description
			"Wave function geomterical (prepared, maybe precomputed, for display on screen) information about a particle."
			, // attributes, public variables
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(QuantumMechanicalGeometryDisplay,Shape);
};
REGISTER_SERIALIZABLE(QuantumMechanicalGeometryDisplay);



