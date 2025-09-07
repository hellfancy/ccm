#ifndef SET_DATA_TO_BXARRY_H
#define SET_DATA_TO_BXARRY_H
/** 
 * \file set_data_to_bxArray.h
 * \brief 
 * \author Tiao Lu, tlu@pku.edu.cn
 * \version 1.0.0
 * \date 2022-10-16
 */

void set_vector_double(const std::vector<double> &x, bxArray * &p);
void set_matrix_double(const std::vector<std::vector<double> > &x, bxArray * &p);
void set_double(double x,  bxArray * &p);
void set_string(const std::string & str, const bxArray * &p); 
void set_vector_string(const std::vector<std::string> & str,  bxArray * &p);

void set_bool(bool x, bxArray * &p);
#endif

//END
//
