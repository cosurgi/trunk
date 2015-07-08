// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMState.hpp"
#include "QMPotential.hpp"
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

void QMState::setLaw2Generator(
	  shared_ptr<IGeom>& ig
	, shared_ptr<IPhys>& ip
	, shared_ptr<Interaction> I
	, shared_ptr<Law2_QMIGeom_QMIPhys_GlobalWavefunction> generator)
{
	qmIG = ig;
	qmIP = ip;
	qmI  = I;
	law2_generator = generator;
};
		
void QMState::update(){
	assert(isAnalytic());
	wasGenerated=false;
	if(not law2_generator) {
		st1_QMStateGen->calculateTableValuesPosition(qmParameters,this);
	} else {
		law2_generator->go(qmIG,qmIP,qmI.get());
	};
// Fx3			if(FIXME_extra_generator_used)
// Fx3				FIXME_extra_generator();
};

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   S T A T E   F U N C T O R
*
*********************************************************************************/
CREATE_LOGGER(St1_QMState);
// !! at least one virtual function in the .cpp file

void St1_QMState::go(const shared_ptr<State>&, const shared_ptr<Material>&, const Body*){
	throw std::runtime_error("\n\nSt1_QMState::go was called directly.\n\n");
};

