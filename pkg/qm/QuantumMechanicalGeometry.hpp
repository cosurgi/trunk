// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <yade/pkg/common/Dispatching.hpp>
#include <yade/core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* W A V E   F U N C T I O N   G E O M E T R Y
*
*********************************************************************************/

/*! @brief QuantumMechanicalGeometry contains geometrical information used to display the particle on screen
 *
 * Things like color, or if it's a wireframe to display, etc.
 *
 */
class QuantumMechanicalGeometry: public Shape
{
	public:
		virtual ~QuantumMechanicalGeometry();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			QuantumMechanicalGeometry
			, // base class
			Shape
			, // class description
			"Wave function geomterical (prepared, maybe precomputed, for display on screen) information about a particle."
			, // attributes, public variables
			, // constructor
			createIndex();
		);
		REGISTER_CLASS_INDEX(QuantumMechanicalGeometry,Shape);
};
REGISTER_SERIALIZABLE(QuantumMechanicalGeometry);



