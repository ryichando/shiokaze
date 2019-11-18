#ifndef BLAS_WRAPPER_H
#define BLAS_WRAPPER_H

// Simplified BLAS wrapper (overloaded readable names)
// with stride==1 assumed, and for std::vector's.
// For the moment, no complex number support, and many routines have been dropped.

#include <vector>
#include <gsl/gsl_blas.h>

namespace BLAS{

// dot products ==============================================================

inline float dotf(int n, const float *x, const float *y)
{ return cblas_sdot(n, x, 1, y, 1); }

inline float dotf(const std::vector<float> &x, const std::vector<float> &y)
{ return cblas_sdot((int)x.size(), &x[0], 1, &y[0], 1); }

inline double dot(int n, const float *x, const float *y)
{ return cblas_dsdot(n, x, 1, y, 1); } // double precision calculation

inline double dot(const std::vector<float> &x, const std::vector<float> &y)
{ return cblas_dsdot((int)x.size(), &x[0], 1, &y[0], 1); } // double precision calculation

inline double dot(int n, const double *x, const double *y)
{ return cblas_ddot(n, x, 1, y, 1); }

inline double dot(const std::vector<double> &x, const std::vector<double> &y)
{ return cblas_ddot((int)x.size(), &x[0], 1, &y[0], 1); }

// 2-norm ====================================================================

inline float norm2(int n, const float *x)
{ return cblas_snrm2(n, x, 1); }

inline float norm2(const std::vector<float> &x)
{ return cblas_snrm2((int)x.size(), &x[0], 1); }

inline double norm2(int n, const double *x)
{ return cblas_dnrm2(n, x, 1); }

inline double norm2(const std::vector<double> &x)
{ return cblas_dnrm2((int)x.size(), &x[0], 1); }

// 1-norm (sum of absolute values) ===========================================

inline float abs_sum(int n, const float *x)
{ return cblas_sasum(n, x, 1); }

inline float abs_sum(const std::vector<float> &x)
{ return cblas_sasum((int)x.size(), &x[0], 1); }

inline double abs_sum(int n, const double *x)
{ return cblas_dasum(n, x, 1); }

inline double abs_sum(const std::vector<double> &x)
{ return cblas_dasum((int)x.size(), &x[0], 1); }

// inf-norm (maximum absolute value: index of max returned) ==================

inline int index_abs_max(int n, const float *x)
{ return cblas_isamax(n, x, 1); }

inline int index_abs_max(const std::vector<float> &x)
{ return cblas_isamax((int)x.size(), &x[0], 1); }

inline int index_abs_max(int n, const double *x)
{ return cblas_idamax(n, x, 1); }

inline int index_abs_max(const std::vector<double> &x)
{ return cblas_idamax((int)x.size(), &x[0], 1); }

// inf-norm (maximum absolute value) =========================================
// technically not part of BLAS, but useful

inline float abs_max(int n, const float *x)
{ return std::fabs(x[index_abs_max(n, x)]); }

inline float abs_max(const std::vector<float> &x)
{ return std::fabs(x[index_abs_max(x)]); }

inline double abs_max(int n, const double *x)
{ return std::fabs(x[index_abs_max(n, x)]); }

inline double abs_max(const std::vector<double> &x)
{ return std::fabs(x[index_abs_max(x)]); }

// saxpy (y=alpha*x+y) =======================================================

inline void add_scaled(int n, float alpha, const float *x, float *y)
{ cblas_saxpy(n, alpha, x, 1, y, 1); }

inline void add_scaled(float alpha, const std::vector<float> &x, std::vector<float> &y)
{ cblas_saxpy((int)x.size(), alpha, &x[0], 1, &y[0], 1); }

inline void add_scaled(int n, double alpha, const double *x, double *y)
{ cblas_daxpy(n, alpha, x, 1, y, 1); }

inline void add_scaled(double alpha, const std::vector<double> &x, std::vector<double> &y)
{ cblas_daxpy((int)x.size(), alpha, &x[0], 1, &y[0], 1); }

// scale (x=alpha*x) =========================================================

inline void scale(int n, float alpha, float *x)
{ cblas_sscal(n, alpha, x, 1); }

inline void scale(float alpha, std::vector<float> &x)
{ cblas_sscal((int)x.size(), alpha, &x[0], 1); }

inline void scale(int n, double alpha, double *x)
{ cblas_dscal(n, alpha, x, 1); }

inline void scale(double alpha, std::vector<double> &x)
{ cblas_dscal((int)x.size(), alpha, &x[0], 1); }

};

#endif
