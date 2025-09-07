/** 
 * \file get_data_from_bxArray.cpp
 * \brief 
 * \author Tiao Lu, tlu@pku.edu.cn
 * \version 1.0.0
 * \date 2022-08-24
 */

#include "get_data_from_bxArray.h"
/**
 * Matrix操作
 * baltam在存储矩阵的时候,是按列存储的,数据的内存布局为,第一列后面跟着第二列,
 * 第二列后跟着第三列,以此类推.
 * m 为矩阵的行数.
 */
#define mat_v(mat, m, i, j) (mat[(((j)-1) * (m) + ((i)-1))])

void extend_xy(std::vector<std::vector<double> > & x,
               size_t m, size_t n, bool col){

  /** x 一定要是一个行向量或者列向量, 否则就返回*/
  size_t m_x = x.size();
  if(m_x == 0) return;
  size_t n_x = x[0].size();
  if(n_x == 0 ) return;
  if(m_x !=1 && n_x != 1) return;

  if(col) { ///col为真，表示会先把x变成列向量，然后通过复制列向量得到扩展矩阵
    ///向量长度要 行数m 相等,
    if(m_x*n_x != m) return;
  }
  else {  ///col为假，表示会先把x变成行向量，然后通过复制行向量得到扩展矩阵
    /// 向量长度和 列数n 相等
    if(m_x*n_x != n) return;
  }
  /** 把向量的数据copy到tx中 */
  std::vector<double> tx;
  size_t len;
  if(col) len = m; else len = n;
  tx.resize(len);
  if(m_x == 1) for(size_t i=0; i<len; i++) tx[i]=x[0][i];
  else for(size_t i=0; i<len; i++) tx[i]=x[i][0];

  /** 先把矩阵的大小设置为m行n列*/
  x.resize(m);
  for(size_t i=0; i<m; i++) x[i].resize(n);

  for(size_t i=0; i<m; i++) {
    for(size_t j=0; j<n; j++) {
      if(col) x[i][j] = tx[i]; /// copy列
      else x[i][j] = tx[j];  ///copy 行
    }
  }
}

/**
 * z 是一个mxn的矩阵, x和y可能仅仅是向量，
 * extend_xy 通过复制行或者列把x和y扩展为mxn的矩阵
 */

void extend_xy(std::vector<std::vector<double> > & x,
               std::vector<std::vector<double> > & y,
               std::vector<std::vector<double> > & z){
  size_t m = z.size();
  if(m == 0) return;
  size_t n = z[0].size();
  if(n == 0) return;
  if( m<=1 || n <= 1) return;
  size_t m_x = x.size(); size_t m_y = y.size();
  if(m_x == 0 || m_y == 0) return;
  size_t n_x = x[0].size(); size_t n_y = y[0].size();
  if(n_x == 0 || n_y == 0) return;

  extend_xy(x, m, n, false); ///通过把x变成行向量，然后复制行扩展得到矩阵
  extend_xy(y, m, n, true);  ///通过把y变成列向量，然后复制列扩展得到矩阵
}



void get_vector_complex_double(std::vector<double> &x,
                               std::vector<double> &y, const bxArray *p) {
  if(bxIsComplexDouble(p)) {
    int m = bxGetM(p);
    int n = bxGetN(p);
    std::complex<double> *mat = (std::complex<double> *)bxGetComplexDoubles(p);
    if(m == 1 || n == 1) {
      if(m == 1) {
        x.resize(n);
        y.resize(n);
        for(int j = 1; j <= n; j++) {
          x[j - 1] = mat[j-1].real();
          y[j - 1] = mat[j-1].imag();
        }
      } else { /** n == 1 */
        x.resize(m);
        y.resize(m);
        for(int j = 1; j <= m; j++) {
          x[j - 1] = mat[j-1].real();
          y[j - 1] = mat[j-1].imag();
        }
      }
    }
    else{ //多列数据，就取第一列和第一行中较长的那个
      if(m>=n) {
        x.resize(m);
        y.resize(m);
        for(int j = 1; j <= m; j++) {
          x[j - 1] = mat[j-1].real();
          y[j - 1] = mat[j-1].imag();
        }
      }
      else{
        x.resize(n);
        y.resize(n);
        for(int j = 1; j <= n; j++) {
          x[j - 1] = mat[(j-1)*m].real();
          y[j - 1] = mat[(j-1)*m].imag();
        }
      }
    }
  }
  return;
}
/**
 * \brief get_vector_double
 * 获得通用指针p存储的double向量
 * 期望通用指针p的类型为double matrix,且为行向量或列向量.
 */
void get_vector_double(std::vector<double> &x, const bxArray *p) {
  assert(bxIsDouble(p));

  int m = bxGetM(p);
  int n = bxGetN(p);
  double *mat = bxGetDoubles(p);
  if(m == 1 || n == 1) {
    if(m == 1) {
      x.resize(n);
      for(int j = 1; j <= n; j++) {
        x[j - 1] = mat_v(mat, m, 1, j);
      }
    } else { /** n == 1 */
      x.resize(m);
      for(int j = 1; j <= m; j++) {
        x[j - 1] = mat_v(mat, m, j, 1);
      }
    }
  }
  else{ //多列数据，就取第一列和第一行中较长的那个
    if(m>=n) {
      x.resize(m);
      for(int j = 1; j <= m; j++) {
        x[j - 1] = mat_v(mat, m, j, 1);
      }
    }
    else{
      x.resize(n);
      for(int j = 1; j <= n; j++) {
        x[j - 1] = mat_v(mat, m, 1, j);
      }
    }
  }
  return;
}

int get_vector_double_check(std::vector<double> &x, const bxArray *p) {
	if(!bxIsDouble(p)){
		return 1;
	}	
	else{
		int m = bxGetM(p);
		int n = bxGetN(p);
		if( n != 1 && m != 1){
			return 2; 
		}
		get_vector_double(x,p); 
	}
	return 0;
}

/**
 * \brief get_matrix_double
 * 获得通用指针p存储的二维double向量
 * 期望通用指针p的类型为double matrix,且为二维矩阵.
 */
void get_matrix_double(std::vector<std::vector<double> > &x, const bxArray *p) {
  assert(bxIsDouble(p));

  int m = bxGetM(p);
  int n = bxGetN(p);
  double *mat = bxGetDoubles(p);
  x.resize(m);
  for(int i = 0; i < m; ++i)
    x[i].resize(n);
  for(int i = 1; i <= m; i++) {
    for(int j = 1; j <= n; j++) {
      x[i - 1][j - 1] = mat_v(mat, m, i, j);
    }
  }

  return;
}

int get_matrix_double_check(std::vector<std::vector<double>> &x, const bxArray *p) {
	if(!bxIsDouble(p)){
		return 1;
	}	
	else{
		get_matrix_double(x,p); 
	}
	return 0;
}

/**
 * \brief get_double
 * 获得通用指针p存储的double单个值
 * 期望通用指针p的类型为double matrix,且为单个值.
 */
void get_double(double &x, const bxArray *p) {
  assert(bxIsDouble(p));
  int m = bxGetM(p);
  int n = bxGetN(p);
  assert(m == 1 && n == 1);
  double *mat = bxGetDoubles(p);
  x = mat_v(mat, m, 1, 1);
  return;
}

int get_double_check(double &x, const bxArray *p) {
	if(!bxIsDouble(p)){
		return 1;
	}	
  int m = bxGetM(p);
  int n = bxGetN(p);
  if( !(m == 1 && n == 1)) return 2;

	get_double(x,p); 
	return 0;
}

/** 
 * \brief get_string  取得一个字符串
 * 
 * \param p  是一个字符串标量 或者 一行的字符矩阵
 * 
 * \return  字符串
 */
std::string get_string(const bxArray *p) {
  if(bxIsString(p)) {
    const char * chars = bxGetStringDataPr(p);
    int n = bxGetStringLen(p);
    std::string s;
    s.resize(n);
    for(int i = 0; i < n; i++) {
      s[i] = chars[i];
    }
    return s;
  }

  assert(bxIsChar(p));
  int m = bxGetM(p);
  int n = bxGetN(p);
  assert(m == 1);
  std::string s;
  const char *chars = bxGetChars(p);
  s.resize(n);
  for(int i = 0; i < n; i++) {
    s[i] = chars[i];
  }
  return s;
}

/** 
 * \brief get_vector_string  取得字符串向量
 * 
 * \param p  是一个字符串向量(一行或者一列的矩阵)
 *     或者 字符矩阵 
 * 
 * \return  字符串向量
 */
std::vector<std::string> get_vector_string(const bxArray *p) {
  std::vector<std::string> s;
  if(bxIsString(p)) {
    baSize m = bxGetM(p);
    baSize n = bxGetN(p);
    if( m!=1 && n != 1){
      bxPrintf("get_vector_string 出错: %dx%d的string不是向量", m,n); 
      return s; 
    }
    s.resize(m*n);
    for(baIndex i=0; i < m*n; i++){
      const char * chars = bxGetString(p,i);
      baSize len = bxGetStringLength(p,i);
      s[i].resize(len);
      for(baSize j = 0; j < len; j++) {
        s[i][j] = chars[j];
      }
    }
    return s;
  }

  assert(bxIsChar(p));
  int m = bxGetM(p);
  int n = bxGetN(p);
  if(m==0) return s; 
  s.resize(m);
  const char *chars = bxGetChars(p);
  for(baIndex i = 1; i <= m; i++){
    s[i-1].resize(n);
    for(baIndex j = 1; j <= n; j++) {
      s[i-1][j-1] = mat_v(chars, m, i, j);
    }
  }
  return s;
}

void get_bool(bool &x, const bxArray *p){
  assert(bxIsLogical(p) || bxIsInt32(p) || bxIsDouble(p));
  int m = bxGetM(p);
  int n = bxGetN(p);
  assert(m == 1 && n == 1);
  x = bxGetLogicals(p);
  return;
}


//END
//
