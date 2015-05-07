// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPacketGaussianWave.hpp"

YADE_PLUGIN(
	(QMPacketGaussianWave)
	);

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T
*
*********************************************************************************/
CREATE_LOGGER(QMPacketGaussianWave);
// !! at least one virtual function in the .cpp file
QMPacketGaussianWave::~QMPacketGaussianWave(){};

Complexr QMPacketGaussianWave::waveFunctionValue_1D_positionRepresentation(
	Real x,    // position where wavepacket is calculated
	Real x0,   // initial position of wavepacket centar at time t=0
	Real t,    // time when wavepacket is evaluated
//
//	Real k,    // wavenumber at which wavepacket is calculated - commented out, since we use position representation here
//
	Real k0,   // initial wavenumber of wavepacket
	Real m,    // particle mass
	Real a,    // wavepacket width, sometimes called sigma, of the Gaussian distribution
	Real hbar  // Planck's constant divided by 2pi
)
{
	x -= x0;
	t -= t0;
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
		
Complexr QMPacketGaussianWave::getValPos(Vector3r pos)
{
	switch(this->dim) {
		case 1 : return waveFunctionValue_1D_positionRepresentation(pos[0],x0[0],this->t,k0[0],m,a0[0],hbar);

		case 2 : return waveFunctionValue_1D_positionRepresentation(pos[0],x0[0],this->t,k0[0],m,a0[0],hbar)*
		                waveFunctionValue_1D_positionRepresentation(pos[1],x0[1],this->t,k0[1],m,a0[1],hbar);

		case 3 : return waveFunctionValue_1D_positionRepresentation(pos[0],x0[0],this->t,k0[0],m,a0[0],hbar)*
		                waveFunctionValue_1D_positionRepresentation(pos[1],x0[1],this->t,k0[1],m,a0[1],hbar)*
				waveFunctionValue_1D_positionRepresentation(pos[2],x0[2],this->t,k0[2],m,a0[2],hbar);
				// FIXME - interesting thing: adding a new dimension is just a multiplication.
				//         but calculating probability is multiplication by conjugate.
				//         just as time is really just another dimension, but in Minkowski metric.
		default: break;
	}
	throw std::runtime_error("getValPos() works only in 1,2 or 3 dimensions.");
};

