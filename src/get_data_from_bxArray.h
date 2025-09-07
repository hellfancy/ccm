#ifndef GET_DATA_FROM_BXARRY_H
#define GET_DATA_FROM_BXARRY_H
/** 
 * \file get_data_from_bxArray.h
 * \brief 
 * \author Tiao Lu, tlu@pku.edu.cn
 * \version 1.0.0
 * \date 2022-08-24
 */
#include <vector>
#include <string>
#include <complex>
#include <cassert>
#include <bex/bex.hpp>

void extend_xy(std::vector<std::vector<double> > & x,
               size_t m, size_t n, bool col);

/**
 * z 是一个mxn的矩阵, x和y可能仅仅是向量，
 * extend_xy 通过复制行或者列把x和y扩展为mxn的矩阵
 */

void extend_xy(std::vector<std::vector<double> > & x,
               std::vector<std::vector<double> > & y,
               std::vector<std::vector<double> > & z);

void get_vector_complex_double(std::vector<double> &x,
                               std::vector<double> &y, const bxArray *p);
/**
 * \brief get_vector_double
 * 获得通用指针p存储的double向量
 * 期望通用指针p的类型为double matrix,且为行向量或列向量.
 */
void get_vector_double(std::vector<double> &x, const bxArray *p);

int get_vector_double_check(std::vector<double> &x, const bxArray *p);

/**
 * \brief get_matrix_double
 * 获得通用指针p存储的二维double向量
 * 期望通用指针p的类型为double matrix,且为二维矩阵.
 */
void get_matrix_double(std::vector<std::vector<double> > &x, const bxArray *p);

int get_matrix_double_check(std::vector<std::vector<double>> &x, const bxArray *p);
/**
 * \brief get_double
 * 获得通用指针p存储的double单个值
 * 期望通用指针p的类型为double matrix,且为单个值.
 */
void get_double(double &x, const bxArray *p);

int get_double_check(double &x, const bxArray *p);

/** 
 * \brief get_string  取得一个字符串
 * 
 * \param p  是一个字符串标量 或者 一行的字符矩阵
 * 
 * \return  字符串
 */
std::string get_string(const bxArray *p); 

/** 
 * \brief get_vector_string  取得字符串向量
 * 
 * \param p  是一个字符串向量(一行或者一列的矩阵)
 *     或者 字符矩阵 
 * 
 * \return  字符串向量
 */
std::vector<std::string> get_vector_string(const bxArray *p);


#endif

//END
//
