// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMPacketGaussianWave.hpp"

YADE_PLUGIN(
	(QMPacketGaussianWave)
	(St1_QMPacketGaussianWave)
	);

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T
*
*********************************************************************************/
CREATE_LOGGER(QMPacketGaussianWave);
// !! at least one virtual function in the .cpp file
QMPacketGaussianWave::~QMPacketGaussianWave(){};

/*********************************************************************************
*
* F R E E L Y   M O V I N G   G A U S S I A N   W A V E P A C K E T   F U N C T O R
*
*********************************************************************************/

Complexr St1_QMPacketGaussianWave::waveFunctionValue_1D_positionRepresentation(
	Real x,    // position where wavepacket is calculated
	Real x0,   // initial position of wavepacket centar at time t=0
	Real t,    // time when wavepacket is evaluated
	Real t0,   // initial time
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
		
Complexr St1_QMPacketGaussianWave::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	const QMPacketGaussianWave* p = static_cast<const QMPacketGaussianWave*>(qms);
	const QMParticle* par = dynamic_cast<const QMParticle*>(pm);
	if(not par) { throw std::runtime_error("\n\nERROR: St1_QMPacketGaussianWave nas no QMParticle, but rather `"
		+std::string(pm?pm->getClassName():"")+"`\n\n");};
//std::cerr << "renderuję dla t = " << (p->t-p->t0) << "\n";
	switch(par->dim) {
		case 1 : return waveFunctionValue_1D_positionRepresentation(pos[0],p->x0[0],p->t,p->t0,p->k0[0],par->m,p->a0[0],par->hbar);

		case 2 : return waveFunctionValue_1D_positionRepresentation(pos[0],p->x0[0],p->t,p->t0,p->k0[0],par->m,p->a0[0],par->hbar)*
		                waveFunctionValue_1D_positionRepresentation(pos[1],p->x0[1],p->t,p->t0,p->k0[1],par->m,p->a0[1],par->hbar);

		case 3 : return waveFunctionValue_1D_positionRepresentation(pos[0],p->x0[0],p->t,p->t0,p->k0[0],par->m,p->a0[0],par->hbar)*
		                waveFunctionValue_1D_positionRepresentation(pos[1],p->x0[1],p->t,p->t0,p->k0[1],par->m,p->a0[1],par->hbar)*
				waveFunctionValue_1D_positionRepresentation(pos[2],p->x0[2],p->t,p->t0,p->k0[2],par->m,p->a0[2],par->hbar);
				// FIXME - interesting thing: adding a new dimension is just a multiplication.
				//         but calculating probability is multiplication by conjugate.
				//         just as time is really just another dimension, but in Minkowski metric.
		default: break;
	}
	throw std::runtime_error("\n\nSt1_QMPacketGaussianWave::getValPos() works only in 1,2 or 3 dimensions.\n\n");
};

