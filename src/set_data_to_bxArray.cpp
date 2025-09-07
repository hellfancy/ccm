#include <vector>
#include <string>
#include <complex>
#include <cassert>
#include <bex/bex.hpp>
#include "set_data_to_bxArray.h"
/** 
 * \file set_data_to_bxArray.cpp
 * \brief 
 * \author Tiao Lu, tlu@pku.edu.cn
 * \version 1.0.0
 * \date 2022-10-16
 */

void set_vector_double(const std::vector<double> &value, bxArray * &bp) {

	size_t n = value.size();
	size_t m = 0;
	if(n>0) m = 1;

	bp = bxCreateDoubleMatrix(m,n, bxREAL);  /// 生成mxn的实矩阵
	if(m*n >0 ){
		double * ptr = bxGetDoubles(bp);
		for(size_t j=0; j < n; j++){
			ptr[j] =  value[j];  
		}
	}
}

void set_matrix_double(const std::vector<std::vector<double> > &value, bxArray * &bp) {

	size_t m = value.size();
	size_t n = 0;
	if(m>0) n = value[0].size(); 

	bp = bxCreateDoubleMatrix(m,n, bxREAL);  /// 生成mxn的实矩阵
	if(m*n >0 ){
		double * ptr = bxGetDoubles(bp);
		for(size_t i=0; i < m; i++){
			for(size_t j=0; j < n; j++){
				ptr[j*m+i] =  value[i][j];  
			}
		}
	}

}


void set_double(double value, bxArray * &bp) {
	bp = bxCreateDoubleMatrix(1,1, bxREAL);
	double * ptr = bxGetDoubles(bp);
	*ptr =  value; 

}

void set_string(const std::string & value,  bxArray *& bp) {
	bp = bxCreateString(value.c_str()); 
}

void set_vector_string(const std::vector<std::string> &value, bxArray * &bp) {
	size_t n = value.size();
	size_t m = 0;
	if(n>0) m = 1;

	if(m*n >0 ){
		const char ** chval = new const char * [n];
		for(size_t j=0; j < n; j++){
			chval[j] =  value[j].c_str();  
		}
		bp = bxCreateStringMatrixFromStrings(m, n, chval);
		delete [] chval;
	}
	else{
		bp = bxCreateStringMatrix(m,n);
	}
}

void set_bool(bool value, bxArray * &bp) {
	bp = bxCreateLogicalScalar(value);
}


//END
//
