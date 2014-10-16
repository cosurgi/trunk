// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QuantumMechanicalState.hpp"
#include <yade/core/Scene.hpp>

YADE_PLUGIN(
	(QuantumMechanicalAnalyticalState)
	(FreePropagatingGaussianWavePacket)
	);

/*********************************************************************************
*
* Q U A N T U M   M E C H A N I C A L   A N A L Y T I C A L   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(QuantumMechanicalAnalyticalState);
// !! at least one virtual function in the .cpp file
QuantumMechanicalAnalyticalState::~QuantumMechanicalAnalyticalState(){};

/*********************************************************************************
*
* W A V E   F U N C T I O N   S T A T E
*
*********************************************************************************/
CREATE_LOGGER(GaussianAnalyticalPropagatingWavePacket_1D); // FIXME - rename, i jeszcze QuantumMechanicalDiscreteState, QMDiscreteState ?
// !! at least one virtual function in the .cpp file
GaussianAnalyticalPropagatingWavePacket_1D::~GaussianAnalyticalPropagatingWavePacket_1D(){};

std::complex<Real> GaussianAnalyticalPropagatingWavePacket_1D::waveFunctionValue_1D_positionRepresentation(
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


