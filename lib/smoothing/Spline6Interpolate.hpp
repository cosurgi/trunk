// 2015 © Janek Kozicki <cosurgi@gmail.com>
#pragma once

#include <vector>
#include <cassert>
#include "lib/base/Math.hpp"

/* Spline6 interpolation function, in 1D, 2D and 3D:
 * valueType spline6InterpolatePoint1D(const std::vector<valueType>                            & values1D, floatType posX                                )
 * valueType spline6InterpolatePoint2D(const std::vector<std::vector<valueType> >              & values2D, floatType posX, floatType posY                )
 * valueType spline6InterpolatePoint3D(const std::vector<std::vector<std::vector<valueType> > >& values3D, floatType posX, floatType posY, floatType posZ)
 * 
 * These functions:
 *	input  → N-dimensional table of values of type "valueType". This table is ,,indexed''
 *	         using "size_t" (int) parameter.
 *
 *	output → a single value of type "valueType" at floatType ,,index'' in the
 *	         table. Henceforth it is a value interpolated from its neighbours at
 *	         "size_t" ,,indexes''.
 *
 * Note: spline6 uses 6 neighbours in each dimension, 3 before and 3 after the
 * posX, posY, posZ coordinate. Therefore it cannot interpolate when closer than
 * 3 to the boundary of the table.
 *
 */

template<typename floatType>
floatType spline6Interpolation(floatType dist)
{ // http://www.path.unimelb.edu.au/%7Edersch/interpolator/interpolator.html
// this interpolation was also described in
// J. Kozicki , J. Tejchman , "Experimental investigations of strain
// localization in concrete using Digital Image Correlation (DIC) technique".
// Archives of Hydro–Engineering and Environmental Mechanics , Vol. 54, No 1,
// pages 3–24, 2007
//
// SPLINE RANGE is 6;
//
	dist=std::abs(dist);
	return dist<0 ? 0 : dist<1 ? (
		(   13.0/11.0  * dist - 453.0/ 209.0 ) * dist -   3.0/ 209.0  
	) * dist + 1.0 : dist<2 ? (
		( -  6.0/11.0  * (dist-1) + 270.0/ 209.0 ) * (dist-1) - 156.0/ 209.0  
	) *(dist-1) : dist<3 ? (
		(    1.0/11.0  * (dist-2) -  45.0/ 209.0 ) * (dist-2) +  26.0/ 209.0  
	) *(dist-2) : 0;
}

template<typename valueType, typename floatType>
valueType  spline6InterpolatePoint1D(
	const std::vector<valueType>& values1D,
	floatType posX
)
{
	valueType ret(ZeroInitializer<valueType>());
	static const size_t CHOSEN_RANGE=6;
	static const size_t STA=1-CHOSEN_RANGE/2;
	static const size_t END=1+CHOSEN_RANGE/2;
	int      start_x=(size_t)(std::floor(posX))+STA
		,end_x  =(size_t)(std::floor(posX))+END;
	
	assert(start_x>=0 and end_x<=values1D.size());

	for(int x=start_x ; x<end_x ; ++x)
		ret += values1D[x]*spline6Interpolation<floatType>((floatType)(x)-posX);

	return ret;
};

template<typename valueType, typename floatType>
valueType  spline6InterpolatePoint2D(
	const std::vector<std::vector< valueType> >& values2D,
	floatType posX, floatType posY
)
{
	valueType ret(ZeroInitializer<valueType>());
	static const size_t CHOSEN_RANGE=6;
	static const size_t STA=1-CHOSEN_RANGE/2;
	static const size_t END=1+CHOSEN_RANGE/2;
	int      start_x=(size_t)(std::floor(posX))+STA
		,end_x  =(size_t)(std::floor(posX))+END
		,start_y=(size_t)(std::floor(posY))+STA
		,end_y  =(size_t)(std::floor(posY))+END;
			
	assert(start_x>=0 and end_x<=values2D   .size());
	assert(start_y>=0 and end_y<=values2D[0].size());

	for(int x=start_x ; x<end_x ; ++x)
		for(int y=start_y ; y<end_y ; ++y)
			ret += values2D[x][y]*spline6Interpolation<floatType>((floatType)(x)-posX)
			                     *spline6Interpolation<floatType>((floatType)(y)-posY);

	return ret;
};

template<typename valueType, typename floatType>
valueType  spline6InterpolatePoint3D(
	const std::vector<std::vector<std::vector<valueType> > >& values3D,
	floatType posX, floatType posY, floatType posZ
)
{
	valueType ret(ZeroInitializer<valueType>());
	static const size_t CHOSEN_RANGE=6;
	static const size_t STA=1-CHOSEN_RANGE/2;
	static const size_t END=1+CHOSEN_RANGE/2;
	int      start_x=(size_t)(std::floor(posX))+STA
		,end_x  =(size_t)(std::floor(posX))+END
		,start_y=(size_t)(std::floor(posY))+STA
		,end_y  =(size_t)(std::floor(posY))+END
		,start_z=(size_t)(std::floor(posZ))+STA
		,end_z  =(size_t)(std::floor(posZ))+END;
			
	assert(start_x>=0 and end_x<=values3D      .size());
	assert(start_y>=0 and end_y<=values3D[0]   .size());
	assert(start_z>=0 and end_z<=values3D[0][0].size());

	for(int x=start_x ; x<end_x ; ++x)
		for(int y=start_y ; y<end_y ; ++y)
			for(int z=start_z ; z<end_z ; ++z)
				ret += values3D[x][y][z]*spline6Interpolation<floatType>((floatType)(x)-posX)
							*spline6Interpolation<floatType>((floatType)(y)-posY)
							*spline6Interpolation<floatType>((floatType)(z)-posZ);

	return ret;
};

// FIXME - add this to `yade --test`, or `yade --check`
#if 0
// test program

#include <iostream>
#include "lib/smoothing/Spline6Interpolate.hpp"
int main(void){
	int debug(0);
	bool error(false);
	std::vector<double> v;
	for(int i=0;i<100;i++) { // fill the 1D table
		v.push_back(std::sin(i*1.0/100) );
	}
	for(int i=(0+100);i<(1000-100);i+=17){
		double val=std::sin(i*1.0/1000);
		double res=spline6InterpolatePoint1D<double,float>(v,i*1.0/10);
		if(debug>0) std::cout << val << " " << res << "\n";
		if( std::abs(val-res) > 0.00001 ) {
			error=true;
			if(debug>0) std::cout << " BAD !" << "\n";
		}
	}
	std::cout << "1D test: " << ((error)?("fail"):(" OK ")) << "\n";
	
	error=false;
	std::vector<std::vector<double> > vv;
	vv.resize(100);
	for(int i=0;i<100;i++)		// fill the 2D table
	for(int j=0;j<100;j++){	// fill the 2D table
		vv[i].push_back( std::sin(i*1.0/100)*std::exp(j*1.0/100) );
	}
	for(int i=(0+100);i<(1000-100);i+=33)
	for(int j=(0+100);j<(1000-100);j+=17){
		double val=std::sin(i*1.0/1000)*std::exp(j*1.0/1000);
		double res=spline6InterpolatePoint2D<double,float>(vv,i*1.0/10,j*1.0/10);
		if(debug>0) std::cout << val << " " << res << "\n";
		if( std::abs(val-res) > 0.00001 ) {
			error=true;
			if(debug>0) std::cout << " BAD !" << "\n";
		}
	}
	std::cout << "2D test: " << ((error)?("fail"):(" OK ")) << "\n";
	
	error=false;
	std::vector<std::vector<std::vector<double> > > vvv;
	vvv.resize(100);
	for(int i=0;i<100;i++)		// fill the 2D table
	for(int j=0;j<100;j++){	// fill the 2D table
		vvv[i].resize(100);
	for(int k=0;k<100;k++){	// fill the 2D table
		vvv[i][j].push_back( std::sin(i*1.0/100)*std::exp(j*1.0/100)*std::pow(k*1.0/100,3.3) );
	}}
	for(int i=(0+100);i<(1000-100);i+=33)   // actually could be i++ , but that's too slow
	for(int j=(0+100);j<(1000-100);j+=37)   // actually could be j++ , but that's too slow
	for(int k=(0+100);k<(1000-100);k+=17){  // actually could be k++ , but that's too slow
		double val=std::sin(i*1.0/1000)*std::exp(j*1.0/1000)*std::pow(k*1.0/1000,3.3);
		double res=spline6InterpolatePoint3D<double,float>(vvv,i*1.0/10,j*1.0/10,k*1.0/10);
		if(debug>0) std::cout << val << " " << res << "\n";
		if( std::abs(val-res) > 0.00001 ) {
			error=true;
			if(debug>0) std::cout << " BAD !" << "\n";
		}
	}
	std::cout << "3D test: " << ((error)?("fail"):(" OK ")) << "\n";
}

#endif 

