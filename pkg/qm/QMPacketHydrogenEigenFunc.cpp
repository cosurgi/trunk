// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPotential.hpp"
#include "QMPacketHydrogenEigenFunc.hpp"
#include <boost/math/special_functions/laguerre.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/math/special_functions/spherical_harmonic.hpp>
using boost::math::laguerre;
using boost::math::factorial;
using boost::math::spherical_harmonic;

YADE_PLUGIN(
	(QMPacketHydrogenEigenFunc)
	(St1_QMPacketHydrogenEigenFunc)
	(QMPacketHydrogenEigenGaussianWave)
	);

/*********************************************************************************
*
* Q U A N T U M   H Y D R O G E N   W A V E F U N C T I O N
* 
*********************************************************************************/

CREATE_LOGGER(QMPacketHydrogenEigenFunc);
// !! at least one virtual function in the .cpp file
QMPacketHydrogenEigenFunc::~QMPacketHydrogenEigenFunc(){};

// FIXME: ↓
#include <lib/time/TimeLimit.hpp>
Real St1_QMPacketHydrogenEigenFunc::En_1D(int n)
{
//	return (-1.0/(2.0*pow<2>(n)));
	return -1.0/(2.0*n*n);
};

Complexr  St1_QMPacketHydrogenEigenFunc::quantumHydrogenWavefunction_1D(int n_, bool even ,Real x)
{ // FIXME: assume hbar=1, mass=1, frequency=1
	if(n_<=0) {
		std::cerr << "Wrong n= " << n_ << " it should be nonzero, positive integer\n";
		return 0;
	};
	Real n(n_);
	auto R_even=[](Real n,Real z)->Complexr{
		return sqrt(1/(4*n*n))*exp(-(abs(z)/2))* abs(z)*laguerre(n-1,1,abs(z));
	};
	auto R_odd =[](Real n,Real z)->Complexr{
		return sqrt(1/(4*n*n))*exp(-(abs(z)/2))*     z *laguerre(n-1,1,abs(z));
	};
	Real z=(2*x)/n;
//std::cerr << n << " " << x << " " << sqrt(2/n)*R_even(n,(2*x)/n) << " sqrt(2/n)=" << sqrt(2/n) << " sqrt(1/(4*n*n))=" << sqrt(1/(4*n*n)) << " exp(-(abs(z)/2.0))=" << exp(-(abs(z)/2.0)) << " laguerre(n-1,1,abs(z))=" << laguerre(n-1,1,abs(z)) << "\n";
	if(even) return sqrt(2/n)*R_even(n,z);
	else     return sqrt(2/n)*R_odd (n,z);
};

Real St1_QMPacketHydrogenEigenFunc::En_2D(int n, Real hbar, Real mi, Real a0)
{
//	return -0.5/(4.0*pow(n - 0.5,2)); // FIXMEatomowe !!!!!!!!
//	return -1.0/(4.0*pow(n - 0.5,2));
	return -pow(hbar,2)/(2*mi*pow(a0,2)*pow(n-0.5,2));
};

Complexr  St1_QMPacketHydrogenEigenFunc::quantumHydrogenWavefunction_2D(int n_, int l_ ,Real x,Real y)
{ // FIXME: assume hbar=1, mass=1, frequency=1
	if(not (n_>0 and abs(l_)<n_)) {
		std::cerr << "Wrong n= " << n_ << " or wrong l="<< l_ <<"\n";
		return 0;
	};
	Real n=n_, l=l_;
	auto R_nl=[](Real n,Real l,Real r,Real beta)->Complexr{
		return
		  beta
		 *sqrt(factorial<Real>(n-1-abs(l))/(factorial<Real>(abs(l)+n-1)*(2*n-1))) // FIXME - precalculate this
		 *exp(-((beta*r)/2))
		 *pow((beta*r),abs(l))
		 *laguerre(-abs(l)+n-1,2*abs(l),beta*r);
	};
	auto Psi_nl=[&R_nl](Real n, Real l, Real r, Real phi, Real a0)->Complexr{
		return
		  R_nl(n, l, r, 2.0/((n-0.5)*a0))
		 *exp(Mathr::I*l*phi)/Mathr::SQRT_TWO_PI;
	};
	Real r=sqrt(x*x+y*y),phi=atan2(y,x);
	Real a0 = FIXMEatomowe().get<2>();
	return Psi_nl(n,l,r,phi,a0);
};

Real St1_QMPacketHydrogenEigenFunc::En_3D(int n)
{
//	return -((Z^2 e^2)/(2 Subscript[a, 0] n^2))
	return -0.5/(2.0*n*n); // FIXMEatomowe
//	return -1.0/(2.0*n*n);
};

Complexr  St1_QMPacketHydrogenEigenFunc::quantumHydrogenWavefunction_3D(int n_, int l_, int m_ ,Vector3r xyz)
{ // FIXME: assume hbar=1, mass=1, frequency=1
	if(not (n_>0 and l_>=0 and l_<n_ and m_>=-l_ and m_<=l_ )) {
		std::cerr << "Wrong n= " << n_ << " or wrong l="<< l_ << " or wrong m=" << m_ <<"\n";
		return 0;
	};
	Real n=n_, l=l_,m=m_;

	auto R_nl=[](Real n,Real l,Real r,Real a,Real Z)->Complexr{
		return 
		  sqrt(
			 pow((2*Z)/(n*a),3)
			*factorial<Real>(n-l-1)/(2*n*(factorial<Real>(n+l)))
		  )
		 *exp(-((Z*r)/(n*a)))
		 *pow((2*Z*r)/(n*a),l)
		 *laguerre(n-l-1,2*l+1,(2*Z*r)/(a*n));
	};

	auto psi_nlm=[&R_nl](Real n,Real l,Real m, Real phi, Real theta, Real r, Real Z, Real a0)->Complexr{
		return
		  R_nl(n,l,r,a0,Z)
		 *spherical_harmonic(l,m,theta,phi);
	};
	Real x=xyz[0],y=xyz[1],z=xyz[2];
	Real r=sqrt(x*x+y*y+z*z),theta=(r!=0)?(acos(z/r)):Mathr::HALF_PI,phi=atan2(y,x);
	Real Z=1;
	Real a0=1;
	return psi_nlm(n,l,m,phi,theta,r,Z,a0);
};

Complexr St1_QMPacketHydrogenEigenFunc::getValPos(Vector3r pos, const QMParameters* par, const QMState* qms)
{
	const QMPacketHydrogenEigenFunc* p = static_cast<const QMPacketHydrogenEigenFunc*>(qms);

// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME,,,,,,,,
	static TimeLimit timeLimit; if(timeLimit.messageAllowed(10))
	std::cerr << "St1_QMPacketHydrogenEigenFunc: Muszę dodać hbar oraz m i 'omega' ?? do listy argumentów. Skąd brać omega, btw? Nowa klasa QMParameters ← QMOscillator w którym byłaby częstotliwość omega??\n";
// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME,,,,,,,,

	// FIXMEatomowe
	auto FIXMEatomowe_vars = FIXMEatomowe();
	Real hbar = FIXMEatomowe_vars.get<0>();
	Real mi   = FIXMEatomowe_vars.get<1>();
	Real a0   = FIXMEatomowe_vars.get<2>();

//std::cerr << "renderuję dla t = " << (p->t-p->t0) << "\n";
	switch(par->dim) {
		case 1 : return quantumHydrogenWavefunction_1D((int)p->energyLevel[0], p->energyLevel[1]==0                , p->x0[0]-pos[0]                )*std::exp(-Mathr::I*En_1D(p->energyLevel[0])*(p->t-p->t0));
		case 2 : return quantumHydrogenWavefunction_2D((int)p->energyLevel[0], p->energyLevel[1]                   , p->x0[0]-pos[0],p->x0[1]-pos[1])*std::exp(-Mathr::I*En_2D(p->energyLevel[0],hbar,mi,a0)*(p->t-p->t0));
		case 3 : return quantumHydrogenWavefunction_3D((int)p->energyLevel[0], p->energyLevel[1], p->energyLevel[2], p->x0-pos                      )*std::exp(-Mathr::I*En_3D(p->energyLevel[0])*(p->t-p->t0));

		default: break;
	}

	throw std::runtime_error("\n\n St1_QMPacketHydrogenEigenFunc::getValPos() works only in 1, 2 or 3 dimensions.\n\n");
};

/*********************************************************************************
*
* Q U A N T U M   H Y D R O G E N  -  F R E E   P R O P A G A T I N G
* 
*********************************************************************************/

CREATE_LOGGER(QMPacketHydrogenEigenGaussianWave);
// !! at least one virtual function in the .cpp file
QMPacketHydrogenEigenGaussianWave::~QMPacketHydrogenEigenGaussianWave(){};

St1_QMPacketHydrogenEigenGaussianWave::~St1_QMPacketHydrogenEigenGaussianWave(){};

Complexr St1_QMPacketHydrogenEigenGaussianWave::getValPos_2particles(
	  Vector3r xyz1, Vector3r xyz2
	, const QMParameters* par1, const QMParameters* par2
	, const QMState* qms)
{
	const QMPacketHydrogenEigenGaussianWave* p = static_cast<const QMPacketHydrogenEigenGaussianWave*>(qms);
	const QMPacketHydrogenEigenFunc*         hydr = dynamic_cast<const QMPacketHydrogenEigenFunc*>(p->hydrogenEigenFunc.get());
	const QMPacketGaussianWave*              gaus = dynamic_cast<const QMPacketGaussianWave*>     (p->gaussianWave.get());
// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME,,,,,,,,
	static TimeLimit timeLimit; if(timeLimit.messageAllowed(30))
	std::cerr << "St1_QMPacketHydrogenEigenGaussianWave: Muszę dodać hbar oraz m i 'omega' ?? do listy argumentów. Skąd brać omega, btw? Nowa klasa QMParameters ← QMOscillator w którym byłaby częstotliwość omega??\n";
// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME,,,,,,,,
	if(not hydr or not gaus) {
		if(timeLimit.messageAllowed(10)) std::cerr << "\n\nSt1_QMPacketHydrogenEigenGaussianWave dynamic_cast error\n";
		return 0;
	}

	St1_QMPacketHydrogenEigenFunc st1_Hydr;
	St1_QMPacketGaussianWave      st1_Gauss;

//	switch(par->dim) {
//		case 1 : return hydr->getValPos()*gaus->getValPos();
//		case 2 : return hydr->getValPos()*gaus->getValPos();
//		case 3 : return hydr->getValPos()*gaus->getValPos();
//
//		default: break;
//	}

	return st1_Hydr.getValPos(xyz1-xyz2,par1,hydr)*st1_Gauss.getValPos((xyz1+xyz2)*0.5,par2,gaus);


	throw std::runtime_error("\n\n St1_QMPacketHydrogenEigenGaussianWave::getValPos() works only in 1, 2 or 3 dimensions.\n\n");
};


