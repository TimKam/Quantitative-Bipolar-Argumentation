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

/**
 * @brief Return True if set1 is subset of set2, False if it is not, and -1 if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return int 1 if subset, 0 if not subset, and -1 if an error occurred
 */
int
PySet_IsSubset(PyObject *set1, PyObject *set2)
{
    PyObject *iterator = PyObject_GetIter(set1);
    PyObject *item;

    if (iterator == NULL) {
        return -1;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(set2, item);
        Py_DECREF(item);

        if (contains < 0) {
            Py_DECREF(iterator);
            return -1;
        }

        if (!contains) {
            Py_DECREF(iterator);
            return 0;   // return False
        }
    }

    Py_DECREF(iterator);

    return 1;   // return True
}

/**
 * @brief Return the union of two sets, NULL if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return PyObject* new PySet reference, NULL if an error occurred
 */
PyObject *
PySet_Union(PyObject *set1, PyObject *set2)
{
    PyObject *tmp;
    // We will iterate over set1, so we want it to be the smallest of them
    if (PySet_GET_SIZE(set1) > PySet_GET_SIZE(set2)) {
        tmp = set1;
        set1 = set2;
        set2 = tmp;
    }

    PyObject *new = PySet_New(set2); // We copy set2
    if (new == NULL) {
        return NULL;
    }

    PyObject *iterator = PyObject_GetIter(set1);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(new);
        return NULL;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(new, item);
        if (contains < 0) {
            Py_DECREF(new);
            Py_DECREF(item); Py_DECREF(iterator);
            return NULL;
        }

        if (!contains) {
            if (PySet_Add(new, item) < 0) {
                Py_DECREF(new);
                Py_DECREF(item); Py_DECREF(iterator);
                return NULL;
            }
        } else {
            Py_DECREF(item);
        }
    }

    Py_DECREF(iterator);

    return new; 
}

/**
 * @brief Return the intersection of two sets, NULL if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return PyObject* new PySet reference, NULL if an error occurred
 */
PyObject *
PySet_Intersection(PyObject *set1, PyObject *set2)
{
    PyObject *tmp;
    // We will iterate over set1, so we want it to be the smallest of them
    if (PySet_GET_SIZE(set1) > PySet_GET_SIZE(set2)) {
        tmp = set1;
        set1 = set2;
        set2 = tmp;
    }

    PyObject *new = PySet_New(NULL); // Raise TypeError if iterable is not actually iterable (not checked)

    PyObject *iterator = PyObject_GetIter(set1);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(new);
        return NULL;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(set2, item);
        if (contains < 0) {
            Py_DECREF(new);
            Py_DECREF(item); Py_DECREF(iterator);
            return NULL;
        }

        if (contains) {
            if (PySet_Add(new, item) < 0) {
                Py_DECREF(new);
                Py_DECREF(item); Py_DECREF(iterator);
                return NULL;
            }
        } else {
            Py_DECREF(item);
        }
    }

    Py_DECREF(iterator);

    return new; 
}

/**
 * @brief Return the difference of two sets, NULL if an error is encountered.
 * 
 * @param set1 a PySet object (not NULL)
 * @param set2 a PySet object (not NULL)
 * @return PyObject* new PySet reference, NULL if an error occurred
 */
PyObject *
PySet_Difference(PyObject *set1, PyObject *set2)
{
    PyObject *new = PySet_New(NULL); // Raise TypeError if iterable is not actually iterable (not checked)

    PyObject *iterator = PyObject_GetIter(set1);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(new);
        return NULL;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(set2, item);
        if (contains < 0) {
            Py_DECREF(new);
            Py_DECREF(item); Py_DECREF(iterator);
            return NULL;
        }

        if (!contains) {
            if (PySet_Add(new, item) < 0) {
                Py_DECREF(new);
                Py_DECREF(item); Py_DECREF(iterator);
                return NULL;
            }
        } else {
            Py_DECREF(item);
        }
    }

    Py_DECREF(iterator);

    return new; 
}

/**
 * @brief Return a new list containing objects returned by the iterable,
 * NULL if an error has occurred.
 * 
 * @param iterable instance of an iterable PyObject
 * @param len length of iterable
 * @return PyObject* New PyList Reference
 */
PyObject *
PyList_Copy(PyObject *iterable, Py_ssize_t len)
{
    PyObject *list = PyList_New(len);   // New reference

    if (list == NULL)
        return NULL;
    
    PyObject *iterator = PyObject_GetIter(iterable);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(list);
        return NULL;
    }

    Py_ssize_t index = 0;
    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        PyList_SET_ITEM(list, index, item);
        index++;
    }

    Py_DECREF(iterator);

    return list;
}

/**
 * @brief Return the concatenation of two lists, NULL if an error has occurred.
 * 
 * @param list1 an instance of PyList (not checked)
 * @param list2 an instance of PyList (not checked)
 * @return PyObject* new PyList
 */
PyObject *
PyList_Concat(PyObject *list1, PyObject *list2)
{
    PyObject *iterator1, *iterator2;
    PyObject *item;

    iterator1 = PyObject_GetIter(list1);    // New reference
    if (iterator1 == NULL) {
        return NULL;
    }

    iterator2 = PyObject_GetIter(list2);
    if (iterator2 == NULL) {
        Py_DECREF(iterator1);
        return NULL;
    }

    PyObject *new = PyList_New(PyList_GET_SIZE(list1) + PyList_GET_SIZE(list2));

    if (new == NULL) {
        Py_DECREF(iterator1);
        Py_DECREF(iterator2);
        return NULL;
    }

    Py_ssize_t index = 0;
    while ((item = PyIter_Next(iterator1))) {    // PyIter_Next returns a new reference

        PyList_SET_ITEM(new, index, item);
        
        index++;
    }

    Py_DECREF(iterator1);

    while ((item = PyIter_Next(iterator2))) {    // PyIter_Next returns a new reference

        PyList_SET_ITEM(new, index, item);
        
        index++;
    }

    Py_DECREF(iterator2);

    return new;
}