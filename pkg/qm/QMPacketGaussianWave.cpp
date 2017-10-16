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

	,Real harmonic
	,Real w    // harmonic potential omega
)
{
	if(harmonic == 0 or t == 0) {
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
	} else if(harmonic == 1) {
		/*
		 * NOTKA: bardzo ordynarnie szukałem jak naprawić ten błąd ze znakiem, ale wygląda na to, że się udało...
		 *
		 * Real tttime = Omega::instance().getScene()->time;
		static Real prev_time(tttime);
		if(prev_time != tttime) {
			prev_time = tttime;
			std::cerr << "1: " << (sin(w*t*0.5              ) > 0 ? "+ " : "- ");
			std::cerr << "2: " << (sin(w*t*0.5-Mathr::PI    ) > 0 ? "+ " : "- ");
			std::cerr << "3: " << (sin(w*t*0.5+Mathr::PI    ) > 0 ? "+ " : "- ");
			std::cerr << "4: " << (sin(w*t*0.5-Mathr::HALF_PI) > 0 ? "+ " : "- ");
			std::cerr << "5: " << (sin(w*t*0.5+Mathr::HALF_PI) > 0 ? "+ " : "- ");
			std::cerr << "6: " << (cos(w*t*0.5              ) > 0 ? "+ " : "- ");
			std::cerr << "7: " << (cos(w*t*0.5-Mathr::PI    ) > 0 ? "+ " : "- ");
			std::cerr << "8: " << (cos(w*t*0.5+Mathr::PI    ) > 0 ? "+ " : "- ");
			std::cerr << "9: " << (cos(w*t*0.5-Mathr::HALF_PI) > 0 ? "+ " : "- ");
			std::cerr << "a: " << (cos(w*t*0.5+Mathr::HALF_PI) > 0 ? "+ " : "- ");
			std::cerr << "b: " << (tan(w*t*0.5              ) > 0 ? "+ " : "- ");
			std::cerr << "c: " << (tan(w*t*0.5-Mathr::PI    ) > 0 ? "+ " : "- ");
			std::cerr << "d: " << (tan(w*t*0.5+Mathr::PI    ) > 0 ? "+ " : "- ");
			std::cerr << "e: " << (tan(w*t*0.5-Mathr::HALF_PI) > 0 ? "+ " : "- ");
			std::cerr << "f: " << (tan(w*t*0.5+Mathr::HALF_PI) > 0 ? "+\n" : "-\n");
		}*/
		Real sign_correction = sin(w*t*0.5+Mathr::HALF_PI) > 0 ? 1.0 : -1.0;
		return  a*sqrt(hbar)*sign_correction
			 *sqrt(-Mathr::I/sin(t*w))
			 *sqrt(m*w/(a*hbar))
			 *exp(
				(a*a*
					(-
						(k0*k0*hbar*hbar+m*m*x*x*w*w)
					)
				 +Mathr::I*m*w*hbar*(x*x+x0*(x0+2.0*Mathr::I*a*a*k0))/(tan(w*t))
				 +2.0*m*x*w*hbar*(a*a*k0-Mathr::I*x0)/sin(w*t)
				)
				/
				(2.0*hbar*(hbar-Mathr::I*a*a*m*w/tan(w*t)))
			 )
			 /
			 (
				pow(Mathr::PI,0.25)*(pow(hbar-Mathr::I*a*a*m*w/tan(t*w),0.5))
			 );
	} else {
		std::cerr << "St1_QMPacketGaussianWave: the harmonic parameter must be either 0 or 1"; exit(1);
		return 0;
	}
};
		
Complexr St1_QMPacketGaussianWave::getValPos(Vector3r pos , const QMParameters* pm, const QMState* qms)
{
	const QMPacketGaussianWave* p = static_cast<const QMPacketGaussianWave*>(qms);
	const QMParticle* par = dynamic_cast<const QMParticle*>(pm);
	if(not par) { throw std::runtime_error("\n\nERROR: St1_QMPacketGaussianWave nas no QMParticle, but rather `"
		+std::string(pm?pm->getClassName():"")+"`\n\n");};
//std::cerr << "renderuję dla t = " << (p->t-p->t0) << "\n";
	switch(par->dim) {
		case 1 : return waveFunctionValue_1D_positionRepresentation(pos[0],p->x0[0],p->t,p->t0,p->k0[0],par->m,p->a0[0],par->hbar, p->harmonic[0], p->w0[0]);

		case 2 : return waveFunctionValue_1D_positionRepresentation(pos[0],p->x0[0],p->t,p->t0,p->k0[0],par->m,p->a0[0],par->hbar, p->harmonic[0], p->w0[0])*
		                waveFunctionValue_1D_positionRepresentation(pos[1],p->x0[1],p->t,p->t0,p->k0[1],par->m,p->a0[1],par->hbar, p->harmonic[1], p->w0[1]);

		case 3 : return waveFunctionValue_1D_positionRepresentation(pos[0],p->x0[0],p->t,p->t0,p->k0[0],par->m,p->a0[0],par->hbar, p->harmonic[0], p->w0[0])*
		                waveFunctionValue_1D_positionRepresentation(pos[1],p->x0[1],p->t,p->t0,p->k0[1],par->m,p->a0[1],par->hbar, p->harmonic[1], p->w0[1])*
				waveFunctionValue_1D_positionRepresentation(pos[2],p->x0[2],p->t,p->t0,p->k0[2],par->m,p->a0[2],par->hbar, p->harmonic[2], p->w0[2]);
				// FIXME - interesting thing: adding a new dimension is just a multiplication.
				//         but calculating probability is multiplication by conjugate.
				//         just as time is really just another dimension, but in Minkowski metric.
		default: break;
	}
	throw std::runtime_error("\n\nSt1_QMPacketGaussianWave::getValPos() works only in 1,2 or 3 dimensions.\n\n");
};

