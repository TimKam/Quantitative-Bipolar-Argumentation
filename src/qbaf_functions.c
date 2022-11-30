/**
 * @file qbaf_functions.c
 * @author Jose Ruiz Alarcon
 * @brief Implementation for aggregation functions and influence functions
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "qbaf_functions.h"

#define max(a,b) (((a)>(b))?(a):(b))

/**
 * @brief Given two strengths w1 and w2 return their sum.
 * 
 * @param w1 a double
 * @param w2 another double
 * @return double the sum of w1 and w2
 */
double sum(double w1, double w2)
{
    return w1 + w2;
}

/**
 * @brief Given two strengths w1 and w2 return their product.
 * 
 * @param w1 a double
 * @param w2 another double
 * @return double the product of w1 and w2
 */
double product(double w1, double w2)
{
    return w1 * w2;
}

/**
 * @brief Given two strengths w1 and w2 return the maximum.
 * 
 * @param w1 a double
 * @param w2 another double
 * @return double maximum of w1 and w2
 */
double top(double w1, double w2)
{
    return max(w1, w2);
}

/**
 * @brief Return the influence result of the basic model.
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result of the influence function
 */
double simple_influence(double w, double s)
{
    return w + s;
}

/**
 * @brief Return the influence function linear(k).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @param k a double
 * @return double the result
 */
static inline
double linear_k(double w, double s, double k)
{   
    return w - (w/k) * max(0,-s) + ((1-w)/k) * max(0, s);
}

/**
 * @brief Return the influence function linear(1).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double linear_1(double w, double s)
{
    return linear_k(w, s, 1);
}

/**
 * @brief Return the influence function Euler-based.
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double euler_based(double w, double s)
{
    return 1 - (1-pow(w, 2)) / (1+w*exp(s));
}

/**
 * @brief Support function for p_max_k.
 * 
 * @param x a double
 * @param p a natural number
 * @return double the result
 */
static inline
double h(double x, uint32_t p)
{
    return pow(max(0, x), p) / (1 + pow(max(0, x), p));
}

/**
 * @brief Return the influence function p-Max(k).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @param p a natural number
 * @param k a double
 * @return double the result
 */
static inline
double p_max_k(double w, double s, uint32_t p, double k)
{
    return w - w * h(-s/k, p) + (1-w) * h(s/k, p);
}

/**
 * @brief Return the influence function 2-Max(1).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double max_2_1(double w, double s)
{
    return p_max_k(w, s, 2, 1);
}

/**
 * @brief Return the influence function 1-Max(1).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double max_1_1(double w, double s)
{
    return p_max_k(w, s, 1, 1);
}