/**
 * @file qbaf_utils.h
 * @author Jose Ruiz Alarcon
 * @brief  Module with util functions for PySet and PyList
 */

#ifndef _QBAF_UTILS_H_
#define _QBAF_UTILS_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/**
 * @brief Return 1 if they are disjoint, 0 if they are not, and -1 if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return int 1 if they are disjoint, 0 if they are not, and -1 if an error is encountered
 */
int PySet_IsDisjoint(PyObject *set1, PyObject *set2);

#endif