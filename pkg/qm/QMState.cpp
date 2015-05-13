// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMState.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMState)
	(St1_QMState)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMState);
// !! at least one virtual function in the .cpp file
QMState::~QMState(){};

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E   F U N C T O R
*
*********************************************************************************/
CREATE_LOGGER(St1_QMState);
// !! at least one virtual function in the .cpp file

void St1_QMState::go(const shared_ptr<State>&, const shared_ptr<Material>&, const Body*){
	throw std::logic_error("St1_QMState::go was called directly.");
};

