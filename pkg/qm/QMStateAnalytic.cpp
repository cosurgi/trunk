// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QMStateAnalytic)
	(FreeMovingGaussianWavePacket)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QMStateAnalytic);
// !! at least one virtual function in the .cpp file
QMStateAnalytic::~QMStateAnalytic(){};

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T
*
*********************************************************************************/
CREATE_LOGGER(FreeMovingGaussianWavePacket);
// !! at least one virtual function in the .cpp file
FreeMovingGaussianWavePacket::~FreeMovingGaussianWavePacket(){};

std::complex<Real> FreeMovingGaussianWavePacket::waveFunctionValue_1D_positionRepresentation(
	Real x,    // position where wavepacket is calculated
	Real x0,   // initial position of wavepacket centar at time t=0
	Real t,    // time when wavepacket is evaluated
	Real k0,   // initial wavenumber of wavepacket
	Real m,    // particle mass
	Real a,    // wavepacket width, sometimes called sigma, of the Gaussian distribution
	Real hbar  // Planck's constant divided by 2pi
)
{
	x -= x0;
	return exp(
		-(
			(m*x*x+Mathr::I*a*a*k0*(k0*hbar*t-2.0*m*x))
			/
			(2.0*a*a*m+2.0*Mathr::I*hbar*t)
		)
	)
	/
	(
		pow(Mathr::PI,0.25)*(pow(a+Mathr::I*hbar*t/(a*m),0.5))
	);
};

