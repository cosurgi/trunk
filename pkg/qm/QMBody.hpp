// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/GlobalEngine.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   B O D Y
*
*********************************************************************************/

/*! @brief QMBody contains single 'body' (a particle) expressed in terms of quantum mechanics.
 *
 * It contains:
 * - QuantumMechanicalBound            → finding spatial collisions                            - FIXME - add this one FIXME,2 - Aabb is enough!
 * - QMGeometry  → OpenGL graphical display, no physics related things!
 * - QMParameters       → classification of indistinguishable quantum particles (fermions, bosons)
 * - QMState            → physical state specific to given particle instance
 *
 */
class QMBody: public Body
{
	public:
		virtual ~QMBody();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR_PY(
			  // class name
			QMBody
			, // base class
			Body
			, // class description
			"Quantum mechanical body is a single 'body' (a particle) expressed in terms of quantum mechanics: with wavefunctions."
			, // attributes, public variables
//  -----→ to fix the inheritance tree we should remove
//         following attributes from the base class
			((string,qtHide,"chain clumpId flags qtHide",Attr::readonly,
			"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class."))
			, // constructor
			, // python bindings
		);
};
REGISTER_SERIALIZABLE(QMBody);

