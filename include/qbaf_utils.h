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
 * @brief Return True if they are disjoint, False if they are not, and -1 if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return int 1 if they are disjoint, 0 if they are not, and -1 if an error is encountered
 */
int PySet_IsDisjoint(PyObject *set1, PyObject *set2);

/**
 * @brief Return True if set1 is subset of set2, False if it is not, and -1 if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return int 1 if subset, 0 if not subset, and -1 if an error occurred
 */
int PySet_IsSubset(PyObject *set1, PyObject *set2);

/**
 * @brief Return the union of two sets, NULL if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return PyObject* new PySet reference, NULL if an error occurred
 */
PyObject * PySet_Union(PyObject *set1, PyObject *set2);

/**
 * @brief Return the intersection of two sets, NULL if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return PyObject* new PySet reference, NULL if an error occurred
 */
PyObject * PySet_Intersection(PyObject *set1, PyObject *set2);

/**
 * @brief Return the difference of two sets, NULL if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return PyObject* new PySet reference, NULL if an error occurred
 */
PyObject * PySet_Difference(PyObject *set1, PyObject *set2);

/**
 * @brief Return a new list containing objects returned by the iterable,
 * NULL if an error has occurred.
 * 
 * @param iterable instance of an iterable PyObject
 * @param len length of the iterable object
 * @return PyObject* New PyList Reference
 */
PyObject *PyList_Copy(PyObject *iterable, Py_ssize_t len);


/**
 * @brief Return the concatenation of two lists, NULL if an error has occurred.
 * 
 * @param list1 a PyList object (not NULL)
 * @param list2 a PyList object (not NULL)
 * @return PyObject* new PyList
 */
PyObject *PyList_Concat(PyObject *list1, PyObject *list2);

#endif