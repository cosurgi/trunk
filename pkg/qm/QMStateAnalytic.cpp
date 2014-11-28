// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include <core/Scene.hpp>

YADE_PLUGIN(
	(QMStateAnalytic)
	(FreeMovingGaussianWavePacket)
	(HarmonicOscillator)
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

Complexr FreeMovingGaussianWavePacket::waveFunctionValue_1D_positionRepresentation(
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
		
Complexr FreeMovingGaussianWavePacket::getValPos(Vector3r pos)
{
	switch(this->dim) {
		case 1 : return waveFunctionValue_1D_positionRepresentation(pos[0],x0[0],this->t,k0[0],m,a,hbar);

		case 2 : return waveFunctionValue_1D_positionRepresentation(pos[0],x0[0],this->t,k0[0],m,a,hbar)*
		                waveFunctionValue_1D_positionRepresentation(pos[1],x0[1],this->t,k0[1],m,a,hbar);

		case 3 : return waveFunctionValue_1D_positionRepresentation(pos[0],x0[0],this->t,k0[0],m,a,hbar)*
		                waveFunctionValue_1D_positionRepresentation(pos[1],x0[1],this->t,k0[1],m,a,hbar)*
				waveFunctionValue_1D_positionRepresentation(pos[2],x0[2],this->t,k0[2],m,a,hbar);
				// FIXME - interesting thing: adding a new dimension is just a multiplication.
				//         but calculating probability is multiplication by conjugate.
				//         just as time is really just another dimension, but in Minkowski metric.
		default: throw std::runtime_error("getValPos() works only in 1,2 or 3 dimensions.");
	}
};

/*********************************************************************************
*
* Q U A N T U M   H A R M O N I C   O S C I L L A T O R   W A V E F U N C T I O N
* 
*********************************************************************************/

CREATE_LOGGER(HarmonicOscillator);
// !! at least one virtual function in the .cpp file
HarmonicOscillator::~HarmonicOscillator(){};

vector<boost::rational<signed long> > HarmonicOscillator::hermitePolynomialCoefficients(int order, boost::rational<signed long> lambdaPerAlpha)
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

	for(int i = a.size() ; i <= order ; ++i)
	{
		boost::rational<signed long> next = a[i-2]*(2*(i-2)+1-lambdaPerAlpha)/(i*(i-1));
		a.push_back(next);
	} 
	return a;
}

vector<boost::rational<signed long> > HarmonicOscillator::hermitePolynomialScaled(int order, boost::rational<signed long> lambdaPerAlpha)
{
	vector<boost::rational<signed long>> a(hermitePolynomialCoefficients(order,lambdaPerAlpha));
	vector<boost::rational<signed long>> b;
	boost::rational<signed long> factor(std::pow(2,order)/a[order]);
	for(int i = 0 ; i<=order ; ++i)
		b.push_back(a[i]*factor);
	return b;
}

Complexr  HarmonicOscillator::quantumOscillatorWavefunction( // assume hbar=1, mass=1, frequency=1
      int n                             // n - order of wavefunction
    , Real x          // position
)
{
	// FIXME
	Real mass=1.0;   // mass - mass of particle
	Real omega=1.0;  // oscillator frequency
	Real hbar=1.0;
	Real                      E      ( hbar*omega*(n+0.5)       );        // energy of oscillator
	Real                      lambda ( 2.0*mass*E / (hbar*hbar) );        // 
	Real                      alpha  ( mass*omega / hbar        );
	Real                      alphaRoot(pow(alpha,0.5));
	//Real                      lambdaPerAlpha ( 2*n+1 );

	// N_n should be 1/sqrt(meter) because it has a dimension of quantum_wavefunction_1D
	Real                      N_n(  pow( pow(alpha/boost::math::constants::pi<Real>() , 0.5) /(std::pow(2,n)*boost::math::factorial<Real>(n) ) , 0.5)  );

	Complexr i(0,1);
	Complexr result(0);
	vector<boost::rational<signed long> > a(hermitePolynomialScaled(n,  2*n+1 ));
	for(int v=0 ; v<=n ; v++)
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

Complexr HarmonicOscillator::getValPos(Vector3r pos)
{
	switch(this->dim) {
		case 1 : return quantumOscillatorWavefunction(order,pos[0])*std::exp((-Mathr::I*(order+0.5))*this->t); // FIXME,FIXME: ,x0[0],this->t,k0[0],m,a,hbar);

		default: throw std::runtime_error("getValPos() works only in 1,2 or 3 dimensions.");
	}
};

