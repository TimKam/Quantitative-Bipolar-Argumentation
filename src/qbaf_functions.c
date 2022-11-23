/**
 * @file qbaf_functions.c
 * @author Jose Ruiz Alarcon
 * @brief Implementation for aggregation functions and influence functions
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "qbaf_functions.h"

/**
 * @brief Given two weights w1 and w2 return their sum.
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
 * @brief Given two weights w1 and w2 return their product.
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
 * @brief Given two weights w1 and w2 return the maximum.
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
 * @brief Return the influence result of the naive model.
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @return double the result of the influence function
 */
double simple_influence(double initial_weight, double aggregation)
{
    return initial_weight + aggregation;
}

/**
 * @brief Return the influence function linear(k).
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @param k a double
 * @return double the result
 */
static inline
double linear_k(double initial_weight, double aggregation, double k)
{
    double s = aggregation >= 0 ? 1.0 : -1.0;
    
    return initial_weight - (initial_weight/k) * max(0,-s) + ((1-initial_weight)/k) * max(0, s);
}

/**
 * @brief Return the influence function linear(1).
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double linear_1(double initial_weight, double aggregation)
{
    return linear_k(initial_weight, aggregation, 1);
}

/**
 * @brief Return the influence function Euler-based.
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double euler_based(double initial_weight, double aggregation)
{
    double s = aggregation >= 0 ? 1.0 : -1.0;

    return 1 - (1-pow(initial_weight, 2)) / (1+initial_weight*exp(s));
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
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @param p a natural number
 * @param k a double
 * @return double the result
 */
static inline
double p_max_k(double initial_weight, double aggregation, uint32_t p, double k)
{
    double s = aggregation >= 0 ? 1.0 : -1.0;

    return initial_weight - initial_weight * h(-s/k, p) + (1-initial_weight) * h(s/k, p);
}

/**
 * @brief Return the influence function 2-Max(1).
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double max_2_1(double initial_weight, double aggregation)
{
    return p_max_k(initial_weight, aggregation, 2, 1);
}

/**
 * @brief Return the influence function 1-Max(1).
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double max_1_1(double initial_weight, double aggregation)
{
    return p_max_k(initial_weight, aggregation, 1, 1);
}