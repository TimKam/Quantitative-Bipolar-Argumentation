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
 * @brief Given the final strengths of attackers and supporters, return the result of the aggregation function 'sum'.
 * Return -1 if an error has occurred.
 * 
 * @param attacker_strengths PyList of attackers' final strengths
 * @param supporter_strengths PyList of supporters' final strengths
 * @return double the result of the aggregation function 'sum', -1 if an error has occurred.
 */
double sum(PyObject *attacker_strengths, PyObject *supporter_strengths)
{
    double attackers_aggregation = 0;
    double supporters_aggregation = 0;

    PyObject *iterator = PyObject_GetIter(attacker_strengths);
    PyObject *item;

    if (iterator == NULL) {
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double strength = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(iterator);
            return -1;
        }

        attackers_aggregation = attackers_aggregation + strength;
    }

    Py_DECREF(iterator);

    iterator = PyObject_GetIter(supporter_strengths);

    if (iterator == NULL) {
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double strength = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(iterator);
            return -1;
        }

        supporters_aggregation = supporters_aggregation + strength;
    }

    Py_DECREF(iterator);

    return supporters_aggregation - attackers_aggregation;
}

/**
 * @brief Given the final strengths of attackers and supporters, return the result of the aggregation function 'product'.
 * Return -1 if an error has occurred.
 * 
 * @param attacker_strengths PyList of attackers' final strengths
 * @param supporter_strengths PyList of supporters' final strengths
 * @return double the result of the aggregation function 'product', -1 if an error has occurred.
 */
double product(PyObject *attacker_strengths, PyObject *supporter_strengths)
{
    double attackers_aggregation = 1;
    double supporters_aggregation = 1;

    PyObject *iterator = PyObject_GetIter(attacker_strengths);
    PyObject *item;

    if (iterator == NULL) {
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double strength = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(iterator);
            return -1;
        }

        attackers_aggregation = attackers_aggregation * (1 - strength);
    }

    Py_DECREF(iterator);

    iterator = PyObject_GetIter(supporter_strengths);

    if (iterator == NULL) {
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double strength = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(iterator);
            return -1;
        }

        supporters_aggregation = supporters_aggregation * (1 - strength);
    }

    Py_DECREF(iterator);

    return attackers_aggregation - supporters_aggregation;
}

/**
 * @brief Given the final strengths of attackers and supporters, return the result of the aggregation function 'top'.
 * Return -1 if an error has occurred.
 * 
 * @param attacker_strengths PyList of attackers' final strengths
 * @param supporter_strengths PyList of supporters' final strengths
 * @return double the result of the aggregation function 'top', -1 if an error has occurred.
 */
double top(PyObject *attacker_strengths, PyObject *supporter_strengths)
{
    double attackers_aggregation = 0;
    double supporters_aggregation = 0;

    PyObject *iterator = PyObject_GetIter(attacker_strengths);
    PyObject *item;

    if (iterator == NULL) {
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double strength = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(iterator);
            return -1;
        }

        attackers_aggregation = max(attackers_aggregation, strength);
    }

    Py_DECREF(iterator);

    iterator = PyObject_GetIter(supporter_strengths);

    if (iterator == NULL) {
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double strength = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(iterator);
            return -1;
        }

        supporters_aggregation = max(supporters_aggregation, strength);
    }

    Py_DECREF(iterator);

    return supporters_aggregation - attackers_aggregation;
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