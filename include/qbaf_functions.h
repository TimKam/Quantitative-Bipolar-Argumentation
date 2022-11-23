/**
 * @file qbaf_utils.h
 * @author Jose Ruiz Alarcon
 * @brief  Module that defines aggregation functions and influence functions
 */

#ifndef _QBAF_FUNCTIONS_H_
#define _QBAF_FUNCTIONS_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/**
 * @brief Return the influence result of the naive model.
 * 
 * @param initial_weight the initial weight
 * @param aggregation the result of applying the aggregation function to all attackers and supporters
 * @return double the result of the influence function
 */
double simple_influence(double initial_weight, double aggregation);

/**
 * @brief Given two weights w1 and w2 return their sum.
 * 
 * @param w1 a double
 * @param w2 another double
 * @return double the sum of w1 and w2
 */
double sum(double w1, double w2);

/**
 * @brief Given two weights w1 and w2 return their product.
 * 
 * @param w1 a double
 * @param w2 another double
 * @return double the product of w1 and w2
 */
double product(double w1, double w2);

/**
 * @brief Given two weights w1 and w2 return the maximum.
 * 
 * @param w1 a double
 * @param w2 another double
 * @return double maximum of w1 and w2
 */
double top(double w1, double w2);

#endif