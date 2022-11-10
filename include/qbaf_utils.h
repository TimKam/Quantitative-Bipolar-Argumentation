/**
 * @file qbaf_utils.h
 * @author Jose Ruiz Alarcon
 * @brief  Module that defines utility functions for PySet and PyList
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

/**
 * @brief Return the concatenation of two lists, NULL if an error has occurred.
 * 
 * @param list1 a PyList object (not NULL)
 * @param list2 a PyList object (not NULL)
 * @return PyObject* new PyList
 */
PyObject *PyList_Concat(PyObject *list1, PyObject *list2);

#endif