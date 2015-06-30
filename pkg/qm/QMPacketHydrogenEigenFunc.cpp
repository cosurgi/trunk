// 2015 © Janek Kozicki <cosurgi@gmail.com>

#include "QMPacketHydrogenEigenFunc.hpp"

YADE_PLUGIN(
	(QMPacketHydrogenEigenFunc)
	(St1_QMPacketHydrogenEigenFunc)
	);

/*********************************************************************************
*
* Q U A N T U M   H A R M O N I C   O S C I L L A T O R   W A V E F U N C T I O N
* 
*********************************************************************************/

CREATE_LOGGER(QMPacketHydrogenEigenFunc);
// !! at least one virtual function in the .cpp file
QMPacketHydrogenEigenFunc::~QMPacketHydrogenEigenFunc(){};

vector<boost::rational<signed long> > St1_QMPacketHydrogenEigenFunc::hermitePolynomialCoefficients(unsigned int order, boost::rational<signed long> lambdaPerAlpha)
{
	vector<boost::rational<signed long> > c_even; // can't use static, because lambdaPerAlpha changes between calls
	vector<boost::rational<signed long> > c_odd;
	if(c_even.size() == 0)
	{
		c_even.push_back(1);
		c_even.push_back(0);
	}
	if(c_odd.size() == 0)
	{
		c_odd.push_back(0);
		c_odd.push_back(1);
	}
	vector<boost::rational<signed long>>& a( (order % 2 == 0) ?c_even:c_odd );
	if(order+1 < a.size())
		return a;

	for(size_t i = a.size() ; i <= order ; ++i)
	{
		boost::rational<signed long> next = a[i-2]*(2*(i-2)+1-lambdaPerAlpha)/(i*(i-1));
		a.push_back(next);
	} 
	return a;
}

vector<boost::rational<signed long> > St1_QMPacketHydrogenEigenFunc::hermitePolynomialScaled(unsigned int order, boost::rational<signed long> lambdaPerAlpha)
{
	vector<boost::rational<signed long>> a(hermitePolynomialCoefficients(order,lambdaPerAlpha));
	vector<boost::rational<signed long>> b;
	boost::rational<signed long> factor(std::pow(2,order)/a[order]);
	for(unsigned int i = 0 ; i<=order ; ++i)
		b.push_back(a[i]*factor);
	return b;
}

Complexr  St1_QMPacketHydrogenEigenFunc::quantumOscillatorWavefunction( // assume hbar=1, mass=1, frequency=1
      unsigned int n                             // n - order of wavefunction
    , Real x          // position
)
{
	// FIXME
	Real mass=1.0;   // mass - mass of particle
	Real omega=1.0;  // oscillator frequency
	Real hbar=1.0;
//	Real                      E      ( hbar*omega*(n+0.5)       );        // energy of oscillator // FIXME - really unused ????
//	Real                      lambda ( 2.0*mass*E / (hbar*hbar) );        // FIXME - where's lambda needed ????
	Real                      alpha  ( mass*omega / hbar        );
	Real                      alphaRoot(pow(alpha,0.5));
	//Real                      lambdaPerAlpha ( 2*n+1 );

	// N_n should be 1/sqrt(meter) because it has a dimension of quantum_wavefunction_1D
	Real                      N_n(  pow( pow(alpha/boost::math::constants::pi<Real>() , 0.5) /(std::pow(2,n)*boost::math::factorial<Real>(n) ) , 0.5)  );

	Complexr i(0,1);
	Complexr result(0);
	vector<boost::rational<signed long> > a(hermitePolynomialScaled(n,  2*n+1 ));
	for(unsigned int v=0 ; v<=n ; v++)
	{
		Real a_v(boost::rational_cast<Real>(a[v])); // Hermite Polynomial coefficient
		result+=N_n*a_v*pow(alphaRoot*x , v) *exp(-0.5*alpha*pow(x,2));
	}
	return result;
}

//int main()
//{
//	// without units, for now.
//	Real s=15.0;
//	Real ds=0.01;
//	for(Real x=-s ; x<=s ; x+=ds)
//	{
//		cout << x << " " << quantumOscillatorWavefunction(25,x) << "\n";
//	}
//	return true;
//}

// FIXME: ↓
#include <lib/time/TimeLimit.hpp>

Complexr St1_QMPacketHydrogenEigenFunc::getValPos(Vector3r pos, const QMParameters* par, const QMState* qms)
{
	const QMPacketHydrogenEigenFunc* p = static_cast<const QMPacketHydrogenEigenFunc*>(qms);

// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME,,,,,,,,
	static TimeLimit timeLimit; if(timeLimit.messageAllowed(10))
	std::cerr << "St1_QMPacketHydrogenEigenFunc: Muszę dodać hbar oraz m i 'omega' ?? do listy argumentów. Skąd brać omega, btw? Nowa klasa QMParameters ← QMOscillator w którym byłaby częstotliwość omega??\n";
// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME// FIXME,,,,,,,,

	switch(par->dim) {
		case 1 : return quantumOscillatorWavefunction((unsigned int)p->energyLevel[0],pos[0])*std::exp((-Mathr::I*(p->energyLevel[0]+0.5))*(p->t-p->t0));
		case 2 : return quantumOscillatorWavefunction((unsigned int)p->energyLevel[0],pos[0])*std::exp((-Mathr::I*(p->energyLevel[0]+0.5))*(p->t-p->t0))*
		                quantumOscillatorWavefunction((unsigned int)p->energyLevel[1],pos[1])*std::exp((-Mathr::I*(p->energyLevel[1]+0.5))*(p->t-p->t0));
		case 3 : return quantumOscillatorWavefunction((unsigned int)p->energyLevel[0],pos[0])*std::exp((-Mathr::I*(p->energyLevel[0]+0.5))*(p->t-p->t0))*
		                quantumOscillatorWavefunction((unsigned int)p->energyLevel[1],pos[1])*std::exp((-Mathr::I*(p->energyLevel[1]+0.5))*(p->t-p->t0))*
		                quantumOscillatorWavefunction((unsigned int)p->energyLevel[2],pos[2])*std::exp((-Mathr::I*(p->energyLevel[2]+0.5))*(p->t-p->t0));

		default: break;
	}
	throw std::runtime_error("\n\nSt1_QMPacketHydrogenEigenFunc::getValPos() works only in 1, 2 or 3 dimensions.\n\n");
};

