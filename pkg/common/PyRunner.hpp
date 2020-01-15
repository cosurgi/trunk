// 2008 © Václav Šmilauer <eudoxos@arcig.cz>
#pragma once
#include<core/GlobalEngine.hpp>
#include<core/Scene.hpp>
#include<pkg/common/PeriodicEngines.hpp>
#include<lib/pyutil/gil.hpp>

namespace yade { // Cannot have #include directive inside.

class PyRunner: public PeriodicEngine {
	public :
		/* virtual bool isActivated: not overridden, PeriodicEngine handles that */
		virtual void action(){ if(command.size()>0) pyRunString(command); }
	// clang-format off
	YADE_CLASS_BASE_DOC_ATTRS(PyRunner,PeriodicEngine,
		"Execute a python command periodically, with defined (and adjustable) periodicity. See :yref:`PeriodicEngine` documentation for details.",
		((string,command,"",,"Command to be run by python interpreter. Not run if empty."))
	);
	// clang-format on
};
REGISTER_SERIALIZABLE(PyRunner);

} // namespace yade

