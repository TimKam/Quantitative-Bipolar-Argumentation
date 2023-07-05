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
 * @brief Given the final strengths of attackers and supporters, return the result of the aggregation function 'sum'.
 * Return -1 if an error has occurred.
 * 
 * @param attacker_strengths PyList of attackers' final strengths
 * @param supporter_strengths PyList of supporters' final strengths
 * @return double the result of the aggregation function 'sum', -1 if an error has occurred.
 */
double sum(PyObject *attacker_strengths, PyObject *supporter_strengths);

/**
 * @brief Given the final strengths of attackers and supporters, return the result of the aggregation function 'product'.
 * Return -1 if an error has occurred.
 * 
 * @param attacker_strengths PyList of attackers' final strengths
 * @param supporter_strengths PyList of supporters' final strengths
 * @return double the result of the aggregation function 'product', -1 if an error has occurred.
 */
double product(PyObject *attacker_strengths, PyObject *supporter_strengths);

/**
 * @brief Given the final strengths of attackers and supporters, return the result of the aggregation function 'top'.
 * Return -1 if an error has occurred.
 * 
 * @param attacker_strengths PyList of attackers' final strengths
 * @param supporter_strengths PyList of supporters' final strengths
 * @return double the result of the aggregation function 'top', -1 if an error has occurred.
 */
double top(PyObject *attacker_strengths, PyObject *supporter_strengths);

/**
 * @brief Return the influence result of the basic model.
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result of the influence function
 */
double simple_influence(double w, double s);

/**
 * @brief Return the influence function linear(1).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double linear_1(double w, double s);

/**
 * @brief Return the influence function Euler-based.
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double euler_based(double w, double s);

/**
 * @brief Return the influence function 2-Max(1).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double max_2_1(double w, double s);

/**
 * @brief Return the influence function 1-Max(1).
 * 
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result
 */
double max_1_1(double w, double s);

#endif