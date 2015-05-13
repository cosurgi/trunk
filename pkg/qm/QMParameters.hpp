// 2014 © Janek Kozicki <cosurgi@gmail.com>

#pragma once

#include <pkg/common/Dispatching.hpp>
#include <core/Scene.hpp>
#include <stdexcept>

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   P A R A M E T E R S
*
*********************************************************************************/

/*! @brief QMParameters contains information about material out of which each particle is made.
 *
 * It is used to categorize particles, like:
 * - quarks (up, down, etc.),
 * - electrons (muon, tau),
 * - neutrinos,
 * - photons (but - maybe photon should be an interaction)
 * - etc
 *
 * Also it is used to declare what kind of potential the particle creates:
 * - Coulomb potential is created by charged particles
 * - Harmonic potential is created by a "source" of harmonic potential
 * - a flat Barrier can also be defined here
 */

class QMParameters: public Material
{
	public:
		virtual ~QMParameters();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(
			  // class name
			QMParameters
			, // base class
			Material
			, // class description
"It is used to categorize particles, like: quarks (up, down, etc.), electrons (muon, tau), neutrinos, \
photons (but - maybe photon should be an interaction).\
\n\
Also it is used to declare what kind of potential the particle creates:\n\
- Coulomb potential is created by charged particles\n\
- Harmonic potential is created by a \"source\" of harmonic potential\n\
- a flat Barrier can also be defined here\
\n\
Important thing to note is that this Material is supposed to be shared between various different instances of particles, thus\
it will make sense to create material for a fundamental particle\n\
\n\
Maybe eg. fermions and bosons will derive from this class too."
			, // attributes, public variables
			((string,qtHide,"density qtHide",Attr::readonly,"Space separated list of variables to hide in qt4 interface. To fix the inheritance tree we should remove those attributes from the base class."))
			((Real  ,hbar  ,1               ,Attr::readonly,"Planck's constant $h$ divided by $2\\pi$: ħ=h/(2π)"))
			((size_t,dim   ,                ,Attr::readonly,"Describes in how many dimensions this quantum particle resides. First Vector3r[0] is used, then [1], then [2]."))
			, // constructor
			createIndex();
	);
	REGISTER_CLASS_INDEX(QMParameters,Material);
};
REGISTER_SERIALIZABLE(QMParameters);

/*********************************************************************************
*
* Q M   P A R T I C L E   with mass
*
*********************************************************************************/

/*! @brief QMParticle is an elementary particle with mass.
 */

class QMParticle: public QMParameters
{
	public:
		virtual ~QMParticle();
		YADE_CLASS_BASE_DOC_ATTRS_CTOR(QMParticle /* class name*/, QMParameters /* base class */
			, "It's a particle with mass" // class description
			, // attributes, public variables
			((Real    ,m   ,1               ,,"Particle mass"))
			, // constructor
			createIndex();
	);
	REGISTER_CLASS_INDEX(QMParticle,QMParameters);
};
REGISTER_SERIALIZABLE(QMParticle);

