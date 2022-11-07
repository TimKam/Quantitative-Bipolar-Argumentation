/**
 * @file qbaf_utils.c
 * @author Jose Ruiz Alarcon
 * @brief Implementation for the utility functions in qbaf_utils.h
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "qbaf_utils.h"

/**
 * @brief Return 1 if they are disjoint, 0 if they are not, and -1 if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return int 1 if they are disjoint, 0 if they are not, and -1 if an error is encountered
 */
int PySet_IsDisjoint(PyObject *set1, PyObject *set2) {
    PyObject *tmp;
    // We will iterate over set1, so we want it to be the smallest of them
    if (PySet_GET_SIZE(set1) > PySet_GET_SIZE(set2)) {
        tmp = set1;
        set1 = set2;
        set2 = tmp;
    }

    PyObject *iterator = PyObject_GetIter(set1);
    PyObject *item;

    if (iterator == NULL) {
        /* propagate error */
        return -1;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(set2, item);
        Py_DECREF(item);

        if (contains < 0) {
            /* propagate error */
            Py_DECREF(iterator);
            return -1;
        }

        if (contains) {
            Py_DECREF(iterator);
            return 0;   // return False
        }
    }

    Py_DECREF(iterator);

    return 1;   // return True
}