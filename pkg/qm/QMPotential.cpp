// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"

YADE_PLUGIN(
	(QMPotGeometry)
	);

/*********************************************************************************
*
* Q M   interaction   P O T E N T I A L   G E O M E T R Y
*
*********************************************************************************/

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   I N T E R A C T I O N   G E O M E T R Y   (geometrical/spatial parameters of the contact)
*
*********************************************************************************/
CREATE_LOGGER(QMPotGeometry);
// !! at least one virtual function in the .cpp file
QMPotGeometry::~QMPotGeometry(){};

