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
 * It is used to categorize wavefunctions to those representing the same indistinguishable particles, like:
 *  - quarks (up, down, etc.),
 *  - electrons (muon, tau),
 *  - neutrinos,
 *  - photons,
 *  - etc.
 *
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
			"Actually wave function has no material. For now this class is empty only for purposes of\
			keeping compatibility with yade. Although maybe later it may come useful to distinguish \
			fundamental particles in the standard model: quarks (up, down, etc.),\
			electrons (muon, tau), neutrinos, etc.\n\
			\n\
			Important thing to note is that this Material is\
			supposed to be shared between various different instances of particles, thus\
			it will make sense to create material for a fundamental particle\n\
			\n\
			But how does this apply to the simples Schrodinger wavepacket?\n\
			\n\
			Maybe eg. fermions and bosons will derive from this class too."
			, // attributes, public variables
//			((bool,isWaveFunction,true,,"This is only a placeholder in QMParameters, not used for anything."))
			((string,qtHide,"density qtHide",Attr::readonly,"Space separated list of variables to hide in qt4 interface. \
			To fix the inheritance tree we should remove those attributes from the base class."))
			, // constructor
			createIndex();
	);
	REGISTER_CLASS_INDEX(QMParameters,Material);
};
REGISTER_SERIALIZABLE(QMParameters);


