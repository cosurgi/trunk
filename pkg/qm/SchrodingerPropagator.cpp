// 2014 © Janek Kozicki <cosurgi@gmail.com>

#include "QMStateAnalytic.hpp"
#include "QMStateDiscrete.hpp"
#include "SchrodingerPropagator.hpp"
#include "QMInteraction.hpp"
#include <core/Scene.hpp>
#include <unsupported/Eigen/FFT>
#include <functional>   // std::plus, std::multiplies
#include <algorithm>    // std::transform
#include <limits>

#ifdef YADE_FFTW3
#include <fftw3.h>
#endif

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

void SchrodingerKosloffPropagator::calcPsiPlus_1(const NDimTable<Complexr>& psiN___0,NDimTable<Complexr>& psiN___1,
	/*FIXME - remove*/QMStateDiscrete* psi)
{
	Real mass(1); // FIXME - this shouldn't be here
std::cerr << " ............ 4  \n";
	Real dt=scene->dt;
	Real R   = calcKosloffR(); // FIXME - this also should be calculated only once
	Real G   = calcKosloffG();

	// FIXME,FIXME ↓
	static bool hasTable(false);                                // k FIXME: kTable should be prepared only once
std::cerr << " ............ 5  \n";
	static NDimTable<Real    > kTable(psiN___0.dim());          // k FIXME: kTable should be prepared only once
	if(! hasTable){
		// FIXME - 1D only
		std::size_t size(kTable.size0(0));
		int i(size/2); // rotate kTable instead
std::cerr << " ............ 6  \n";


		//0 FOREACH(Real& k, kTable) k                  =-1.0*std::pow(psi->iToK((i++)%kTable.size()),2); // k FIXME: kTable should be prepared only once, FIXME - and not here!!
		      // FIXME 1D only
		      // używając mój increment, może po dorobieniu iteratora mógłbym wołać kTable.at(increment_zmienna++)= ....
		      for(int i=0;i<kTable.size0(0);i++) kTable.at(i)=-1.0*std::pow(psi->iToK((i++)%size),2);


		hasTable=true;
	}
std::cerr << " ............ 7  \n";
// FIXME: potrzebuję mnożenie, dodawanie, odejmowanie - to jest wszystko 1D, nieważne ileD jest naprawdę......
//        więc czy da się użyć fftw_malloc na std::vector na tym wszystkim? osobno real/imag?
//        i osobno dopisać metody do odczytywanie indywidualnych punktów w ileśD ?
//
	//1 std::vector<Complexr> psiN1_tmp2(psiN___0.size());             // ψ₁:
	//1 doFFT_1D(psiN___0,psiN1_tmp2);                                 // ψ₁: psiN1_tmp2=ℱ(ψ₀)
	NDimTable<Complexr> psiN1_tmp2={}; // FIXME - maybe make an FFT-constructor, that constructs by FFTof sth else.
<<<<<<< HEAD
//	                    psiN1_tmp2.becomesFFT(psiN___0);             // ψ₁: psiN1_tmp2=ℱ(ψ₀)
=======
std::cerr << " ............ 8  \n";
	                    psiN1_tmp2.becomesFFT(psiN___0);             // ψ₁: psiN1_tmp2=ℱ(ψ₀)
>>>>>>> SchrodingerKosloffPropagator has problems
			    // FIXME - maybe use operator=FFT<>(psiN___0); ? This FFT() is a type cast to call correct =

	//2 std::vector<Complexr> psiN1_tmp1(psiN___0);                    // ψ₁:
	//2 FOREACH(Complexr& psi_i, psiN1_tmp1) psi_i*=(1+G/R);           // ψ₁: psiN1_tmp1=(1+G/R)ψ₀
	                                   psiN___1 =psiN___0*(1+G/R);     // ψ₁:         ψ₁=(1+G/R)ψ₀
	

	//3 std::vector<Complexr> psiN1_tmp3(psiN___0.size());             // ψ₁:
	//3 							       // ψ₁: psiN1_tmp3=-k²ℱ(ψ₀)
	//3 std::transform(psiN1_tmp2.begin(), psiN1_tmp2.end(), kTable.begin(), psiN1_tmp3.begin(), std::multiplies<Complexr>());
	                                   psiN1_tmp2 *= kTable;       // ψ₁: psiN1_tmp2=-k²ℱ(ψ₀)
	
	//4 std::vector<Complexr> psiN1_tmp4(psiN___0.size());             // ψ₁:
	//4 doIFFT_1D(psiN1_tmp3,psiN1_tmp4);                              // ψ₁:            psiN1_tmp4=ℱ⁻¹(-k²ℱ(ψ₀))
<<<<<<< HEAD
//	                    psiN1_tmp2.becomesIFFT(/* no argument means in-place*/);  // ψ₁: psiN1_tmp2=ℱ⁻¹(-k²ℱ(ψ₀))
=======
	                    psiN1_tmp2.becomesIFFT(
			    //FIXME - remove that:
				psiN1_tmp2
			    /* no argument means in-place*/);  // ψ₁: psiN1_tmp2=ℱ⁻¹(-k²ℱ(ψ₀))
>>>>>>> SchrodingerKosloffPropagator has problems
			    // FIXME - alternatywa:  IFFT(psiN1_tmp2); // friend class, i wtedy działa na tym.
	
	//5 FOREACH(Complexr& psi_i, psiN1_tmp4) psi_i*=dt*hbar/(R*2*mass);// ψ₁: psiN1_tmp4=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	                                   psiN1_tmp2 *=dt*hbar/(R*2*mass);// ψ₁: psiN1_tmp2=(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)

	// std::vector<Complexr> psiN___1(psiN___0.size());             // ψ₁: (that's the output)
								       // ψ₁: psiN___1=(1+G/R)ψ₀+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₀)) )/(ℏ R 2 m)
	//6 std::transform(psiN1_tmp4.begin(), psiN1_tmp4.end(), psiN1_tmp1.begin(), psiN___1.begin(), std::plus<Complexr>());
	                            psiN___1 += psiN1_tmp2;
	// result is in psiN___1

	FOREACH(const shared_ptr<Interaction>& i, *scene->interactions){
		QMInteractionGeometry* igeom=dynamic_cast<QMInteractionGeometry*>(i->geom.get());
		if(igeom) {
			NDimTable<Complexr> psiN1_pot(igeom->potentialValues);      // ψ₁: (potential)
			//7 for(int j=0 ; j<psiN1_potential.size() ; j++) psiN1_potential[j]*=dt*psiN___1[j]/(hbar*R);
			                                                  psiN1_pot         *=   psiN___1;             // FIXME - should look nicer
			                                                  psiN1_pot         *=dt            /(hbar*R); // FIXME - should look nicer
			//8 std::vector<Complexr> tmp (psiN___0.size()  );                         // ψ₁: (potential)
			//8 std::transform(psiN___1.begin(), psiN___1.end(), psiN1_potential.begin(), tmp.begin(), std::minus<Complexr>());
			                                                  psiN___1 -= psiN1_pot      ;
			//8 psiN___1=tmp; // FIXME - inefficient
		}
	};
	// FIXME: return std::move(psiN___1);
}

// FIXME - later http://stackoverflow.com/questions/2763006/change-the-current-branch-to-master-in-git
void SchrodingerKosloffPropagator::action()
{
	timeLimit.readWallClock();
	Real R   = calcKosloffR(); // FIXME -  that's duplicate here
	Real G   = calcKosloffG(); // FIXME -  that's duplicate here
	Real R13 = 1.3*R;
	Real min = 100.0*std::numeric_limits<Real>::min(); // get the numeric minimum, smallest number. To compare if anything is smaller than it, this one must be larger.
std::cerr << " ............ 1  \n";
	// FIXME - not sure about this parallelization. Currently I have only one wavefunction.
	YADE_PARALLEL_FOREACH_BODY_BEGIN(const shared_ptr<Body>& b, scene->bodies){
		QMStateDiscrete* psi=dynamic_cast<QMStateDiscrete*>(b->state.get());
		const Body::id_t& id=b->getId();
std::cerr << " ............ 2  \n";
		if(psi) {
			// prepare for the loop, FIXME: it's 1D only
			               //8 ↓
			NDimTable<Complexr>& psi_final(psi->tableValuesPosition);     // will become ψ(t+dt):
			NDimTable<Complexr>  psiN___0 (psi_final);                    // psiN___0=ψ₀
			NDimTable<Complexr>  psiN___1 = {};               // ψ₁:
std::cerr << " ............ 3  \n";
			calcPsiPlus_1(psiN___0,psiN___1,psi);
			
			Complexr ak(1);
			Complexr ak0=calcAKseriesCoefficient(0,R);
			Complexr ak1=calcAKseriesCoefficient(1,R);
			//1 int j(0);                                                      // ψ(t+dt): psi_final=a₀ψ₀+a₁ψ₁
			//1 FOREACH(Complexr& psi_i, psi_final) psi_i=ak0*psi_i + ak1*psiN___1[j++];
			                                        psi_final*=ak0;           // FIXME - make it look nicer
			                                        psi_final+=psiN___1*ak1;  // FIXME - make it look nicer

			int i(0);
			// never stop when i<R*1.3, unless steps is positive. Auto stop based on std::numeric_limits<Real>::min()
			for(i=2 ; (steps > 1) ? (i<=steps):(i<R13 or (std::abs(std::real(ak))>min or std::abs(std::imag(ak))>min) ) ; i++)
			{
				//2 std::vector<Complexr> psi_tmp(psiN___0.size());        // ψ₂:
				    NDimTable<Complexr> psiN___2;        // ψ₂:

				//3 calcPsiPlus_1(psiN___1,psi_tmp,psi);                   // ψ₂: psi_tmp  =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)
				    calcPsiPlus_1(psiN___1,psiN___2,psi);                  // ψ₂: psiN___2 =     (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m)

				//4 FOREACH(Complexr& psi_i, psi_tmp) psi_i*=2;            // ψ₂: psi_tmp  = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) )
				    psiN___2 *= 2;                                         // ψ₂: psiN___2 = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) )
				
				//5 std::vector<Complexr> psiN___2(psiN___0.size());       // ψ₂:
				//5                                                        // ψ₂: psiN___2 = 2*( (1+G/R)ψ₁+(dt ℏ² ℱ⁻¹(-k²ℱ(ψ₁)) )/(ℏ R 2 m) ) - ψ₀
				//5 std::transform(psi_tmp.begin(), psi_tmp.end(), psiN___0.begin(), psiN___2.begin(), std::minus<Complexr>());
				    psiN___2 -= psiN___0;

				// FIXME - all ak can be precalculated, because R=const
				ak = calcAKseriesCoefficient(i,R);

				//1 j=0;                                                   // ψ(t+dt):  psi_final+=a₂ψ₂
				//6 FOREACH(Complexr& psi_i, psi_final) psi_i+=ak*psiN___2[j++]; 
				                                                              //
				                                                            //!!!!!
				                                                        //!!!!!!!!!!!!!!
				    psi_final += psiN___2*ak; // FIXME - (maybe) inefficient !!!!!!!!!!!!!!!!!!!!!\\


				psiN___0=std::move(psiN___1);                                     // ψ₀ ← ψ₁
				psiN___1=std::move(psiN___2);                                     // ψ₁ ← ψ₂
			}
			//7 Complexr expiRG = std::exp(-1.0*Mathr::I*(R+G));
			//7 FOREACH(Complexr& psi_i, psi_final) psi_i*=expiRG;             // ψ(t+dt): psi_final=exp(-i(R+G))*(a₀ψ₀+a₁ψ₁+a₂ψ₂+...)
			    psi_final *= std::exp(-1.0*Mathr::I*(R+G));

			//std::cerr << "middle ak " << ak << "\n";
			//8 ↑psi->tableValuesPosition=psi_final;
			   //↑ zrobione na górze

			if(timeLimit.messageAllowed(4)) std::cerr << "final ak=" << std::abs(ak) << " iterations: " << i-1 << "/" << steps << "\n";
		}
	} YADE_PARALLEL_FOREACH_BODY_END();
}

