// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "SchrodingerPropagator.hpp"
#include "QMInteraction.hpp"
#include <core/Scene.hpp>
#include <unsupported/Eigen/FFT>
#include <functional>   // std::plus, std::multiplies
#include <algorithm>    // std::transform

YADE_PLUGIN(
	(SchrodingerAnalyticPropagator)
	(SchrodingerKosloffPropagator)
	);

/*********************************************************************************
*
* A N A L Y T I C   P R O P A G A T I O N   of   S H R Ö D I N G E R  E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerAnalyticPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerAnalyticPropagator::~SchrodingerAnalyticPropagator(){};

void SchrodingerAnalyticPropagator::action()
{
	const Real& dt=scene->dt;
//	const Real& time=scene->time;
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
		QMStateAnalytic* analytic=dynamic_cast<QMStateAnalytic*>(b->state.get());
		const Body::id_t& id=b->getId();
		if(analytic) {
			analytic->t += dt;
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

/*********************************************************************************
*
* K O S L O F F   P R O P A G A T I O N   o f   S H R O D I N G E R   E Q.
*
*********************************************************************************/
CREATE_LOGGER(SchrodingerKosloffPropagator);
// !! at least one virtual function in the .cpp file
SchrodingerKosloffPropagator::~SchrodingerKosloffPropagator(){};

// for (slow & safe) 2D, check http://stackoverflow.com/questions/17194451/how-to-use-eigen-fft-with-matrixxf
// for faster      - fftw3
//     even faster - CUDA fftw backend
void SchrodingerKosloffPropagator::doFFT_1D (const std::vector<Complexr>& inp,std::vector<Complexr>& out)
{	
	std::vector<Complexr> in(inp);
// ROTATE kTable instead.
//	std::rotate(in.begin(),in.begin()+(in.size()/2-0),in.end()); // prepare input: rotate to left by (size/2-1)
	// FIXME - muszę sprawdzić dokładniej, ale to  ↑  chyba dlatego ze mathematica numeruje od 1 a C++ od 0.
	static Eigen::FFT<Real>  fft;
	fft.fwd(out,in);                      // in mathematica that's InverseFourier[]*sqrt(N)
	Real factor=std::sqrt(out.size());
//	FOREACH(Complexr& c, out ) c/=factor; // so I need to divide by sqrt(N) // FIXME - probably unnecessary, just a constant

// ROTATE kTable instead.
//	std::rotate(out.rbegin(),out.rbegin()+(out.size()/2-0),out.rend()); // prepare output: rotate to right by (size/2-1)
}

void SchrodingerKosloffPropagator::doIFFT_1D(const std::vector<Complexr>& inp,std::vector<Complexr>& out)
{
	std::vector<Complexr> in(inp);
// ROTATE kTable instead.
//	std::rotate(in.begin(),in.begin()+(in.size()/2-0),in.end()); // prepare input: rotate to left by (size/2-1)

	static Eigen::FFT<Real>  fft;
	fft.inv(out,in);                      // in mathematica that's Fourier[]/sqrt(N)
	Real factor=std::sqrt(out.size());
//	FOREACH(Complexr& c, out ) c*=factor; // so I need to multiply by sqrt(N) // FIXME - probably unnecessary, just a constant
	
// ROTATE kTable instead.
//	std::rotate(out.rbegin(),out.rbegin()+(out.size()/2-0),out.rend()); // prepare output: rotate to right by (size/2-1)
}

void SchrodingerKosloffPropagator::fftTest()
{
	std::vector<Complexr> in(0),out(8);
	in.push_back(10);
	in.push_back(20);
	in.push_back(1);
	in.push_back(-4);
	in.push_back(5);
	in.push_back(6);
	in.push_back(0);
	in.push_back(1);
	FOREACH(const Complexr& c, in ) { std::cerr << c << ", " ; }; std::cerr << "\n";
	FOREACH(const Complexr& c, out) { std::cerr << c << ", " ; }; std::cerr << "\n";
	doFFT_1D(in,out);
	FOREACH(const Complexr& c, in ) { std::cerr << c << ", " ; }; std::cerr << "\n";
	FOREACH(const Complexr& c, out) { std::cerr << c << ", " ; }; std::cerr << "\n";
	doIFFT_1D(out,in);
	FOREACH(const Complexr& c, in ) { std::cerr << c << ", " ; }; std::cerr << "\n";
	FOREACH(const Complexr& c, out) { std::cerr << c << ", " ; }; std::cerr << "\n";
// debug output:
// FIXME, FIXME - add this to yade --check or test.
// FIXME, FIXME - this is important, because various FFT libraries divide by sqrt(N) or some other numbers.
// (10,-2.14306e-16), (20,4.71028e-16), (1,2.14306e-16), (-4,0), (5,-2.14306e-16), (6,-4.71028e-16), (1.57009e-16,2.14306e-16), (1,0), 
// (-0.767767,3.44975), (-10.253,4.94975), (-2.76777,-6.44975), (13.7886,0), (-2.76777,6.44975), (-10.253,-4.94975), (-0.767767,-3.44975), (2.47487,0), 
}

Real SchrodingerKosloffPropagator::eMax()
{
	Real ret(0); // assume that negative maximum energy is not possible
	FOREACH(const shared_ptr<Body>& b, *scene->bodies){
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		if(psi) {
			ret=std::max(ret,std::pow(psi->kMax()* 1/* FIXME: must be `hbar` here */,2)/(2 /*FIXME: must be mass here psi->m */) );
		}
	};
	return ret;
}

void SchrodingerKosloffPropagator::calcPsiPlus_1(const std::vector<Complexr>& psiN___0,std::vector<Complexr>& psiN___1,/*FIXME - remove*/QMStateDiscrete* psi)
{
	Real mass(1); // FIXME - this shouldn't be here
	Real dt=scene->dt;
	Real R   = calcKosloffR(); // FIXME - this also should be calculated only once
	Real G   = calcKosloffG();

	// FIXME,FIXME ↓
	static bool hasTable(false);                                   // k FIXME: kTable should be prepared only once
	static std::vector<Real    > kTable(psiN___0.size());          // k FIXME: kTable should be prepared only once
	if(! hasTable){
		int i(0);
		FOREACH(Real& k, kTable) k=-1.0*std::pow(psi->iToK(i++),2); // k FIXME: kTable should be prepared only once, FIXME - and not here!!
		hasTable=true;
		std::rotate(kTable.begin(),kTable.begin()+(kTable.size()/2-0),kTable.end()); // ROTATE kTable instead.
	}
// FIXME: potrzebuję mnożenie, dodawanie, odejmowanie - to jest wszystko 1D, nieważne ileD jest naprawdę......
//        więc czy da się użyć fftw_malloc na std::vector na tym wszystkim? osobno real/imag?
//        i osobno dopisać metody do odczytywanie indywidualnych punktów w ileśD ?
//
	std::vector<Complexr> psiN1_tmp1(psiN___0);                    // ψ₁:
	FOREACH(Complexr& psi_i, psiN1_tmp1) psi_i*=(1+G/R);           // ψ₁: psiN1_tmp1=(1+G/R)ψ₀
	
	std::vector<Complexr> psiN1_tmp2(psiN___0.size());             // ψ₁:
	doFFT_1D(psiN___0,psiN1_tmp2);                                 // ψ₁: psiN1_tmp2=ℱ(ψ₀)

	std::vector<Complexr> psiN1_tmp3(psiN___0.size());             // ψ₁:
								       // ψ₁: psiN1_tmp3=-k²ℱ(ψ₀)
	std::transform(psiN1_tmp2.begin(), psiN1_tmp2.end(), kTable.begin(), psiN1_tmp3.begin(), std::multiplies<Complexr>());
	
	std::vector<Complexr> psiN1_tmp4(psiN___0.size());             // ψ₁:
	doIFFT_1D(psiN1_tmp3,psiN1_tmp4);                              // ψ₁: psiN1_tmp4=ℱ⁻¹(-k²ℱ(ψ₀))
	FOREACH(Complexr& psi_i, psiN1_tmp4) psi_i*=dt*hbar/(R*2*mass);// ψ₁: psiN1_tmp4=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	
	//std::vector<Complexr> psiN___1(psiN___0.size());             // ψ₁: (that's the output)
								       // ψ₁: psiN___1=(1+G/R)ψ₀+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	std::transform(psiN1_tmp4.begin(), psiN1_tmp4.end(), psiN1_tmp1.begin(), psiN___1.begin(), std::plus<Complexr>());
	// result is in psiN___1

	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){
		QMInteractionGeometry* igeom=dynamic_cast<QMInteractionGeometry*>(i->geom.get());
		if(igeom) {
			std::vector<Complexr>    psiN1_potential(igeom->potentialValues);      // ψ₁: (potential)
			for(int j=0 ; j<psiN1_potential.size() ; j++) psiN1_potential[j]*=dt*psiN___1[j]/(hbar*R);
			std::vector<Complexr> tmp (psiN___0.size()  );                         // ψ₁: (potential)
			std::transform(psiN___1.begin(), psiN___1.end(), psiN1_potential.begin(), tmp.begin(), std::minus<Complexr>());
			psiN___1=tmp; // FIXME - inefficient
		}
	};
}

void SchrodingerKosloffPropagator::action()
{
	Real R   = calcKosloffR(); // FIXME -  that's duplicate here
	Real G   = calcKosloffG(); // FIXME -  that's duplicate here
	int STEPS(steps<0 ? std::max(3*R,5.0) : steps);

	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		const Body::id_t& id=b->getId();
		if(psi) {
			// prepare for the loop, FIXME: it's 1D only
			std::vector<Complexr> psiN___0(psi->tableValuesPosition[0][0]);// psiN___0=ψ₀
			std::vector<Complexr> psiN___1(psiN___0.size());               // ψ₁:
			calcPsiPlus_1(psiN___0,psiN___1,psi);
			
			std::vector<Complexr> psi_final(psiN___0);                     // ψ(t+dt):
			Complexr ak(1);
			Complexr ak0=calcAKseriesCoefficient(0,R);
			Complexr ak1=calcAKseriesCoefficient(1,R);
			int j(0);                                                      // ψ(t+dt): psi_final=a₀ψ₀+a₁ψ₁
			FOREACH(Complexr& psi_i, psi_final) psi_i=ak0*psi_i + ak1*psiN___1[j++];
			
			int i(0);
			for(i=2 ; i<=STEPS /*and std::abs(ak)>1e-40*/ ; i++)
			{
				std::vector<Complexr> psi_tmp(psiN___0.size());        // ψ₂:
				calcPsiPlus_1(psiN___1,psi_tmp,psi);                   // ψ₂: psi_tmp  =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
				FOREACH(Complexr& psi_i, psi_tmp) psi_i*=2;            // ψ₂: psi_tmp  = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) )
				
				std::vector<Complexr> psiN___2(psiN___0.size());       // ψ₂:
				                                                       // ψ₂: psiN___2 = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				std::transform(psi_tmp.begin(), psi_tmp.end(), psiN___0.begin(), psiN___2.begin(), std::minus<Complexr>());

				ak = calcAKseriesCoefficient(i,R);
				j=0;                                                   // ψ(t+dt):  psi_final+=a₂ψ₂
				FOREACH(Complexr& psi_i, psi_final) psi_i+=ak*psiN___2[j++]; 

				psiN___0=psiN___1;                                     // ψ₀ ← ψ₁
				psiN___1=psiN___2;                                     // ψ₁ ← ψ₂
			}
			Complexr expiRG = std::exp(-1.0*Mathr::I*(R+G));
			FOREACH(Complexr& psi_i, psi_final) psi_i*=expiRG;             // ψ(t+dt): psi_final=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)

			psi->tableValuesPosition[0][0]=psi_final;
			if(errorAllowed()) std::cerr << "final ak=" << std::abs(ak) << " iterations: " << i-1 << "/" << STEPS << "\n";
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

