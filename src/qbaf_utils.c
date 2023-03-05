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
    if (PySet_GET_SIZE(set1) > PySet_GET_SIZE(set2)) {
        return 0; // return False
    }

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
        if (PySet_Add(new, item) < 0) {
            Py_DECREF(new);
            Py_DECREF(item); Py_DECREF(iterator);
            return NULL;
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

/**
 * @brief Return a list containing an empty PySet, NULL if an error has occurred.
 * 
 * @return PyObject* a new PyList, NULL if an error occurred
 */
PyObject *
PyList_NewEmptySet(void)
{
    PyObject *empty_set = PySet_New(NULL);
    if (empty_set == NULL) {
        return NULL;
    }
    PyObject *list = PyList_New(1);
    if (list == NULL) {
        Py_DECREF(empty_set);
        return NULL;
    }
    PyList_SET_ITEM(list, 0, empty_set);
    return list;
}

/**
 * @brief Return the concatenation of all the sublists of one list, NULL if an error has occurred.
 * 
 * @param list a PyList of PyList
 * @return PyObject* a new PyList, NULL if an error occurred
 */
PyObject *
PyList_ConcatItems(PyObject *list)
{
    PyObject *iterator1, *iterator2;
    PyObject *sublist, *item;

    iterator1 = PyObject_GetIter(list);    // New reference
    if (iterator1 == NULL) {
        return NULL;
    }

    Py_ssize_t size = 0;
    while ((sublist = PyIter_Next(iterator1))) {    // PyIter_Next returns a new reference

        size += PyList_GET_SIZE(sublist);

        Py_DECREF(sublist);
    }
    Py_DECREF(iterator1);

    PyObject *new = PyList_New(size);
    if (new == NULL) {
        return NULL;
    }

    iterator1 = PyObject_GetIter(list);    // New reference
    if (iterator1 == NULL) {
        Py_DECREF(new);
        return NULL;
    }

    Py_ssize_t index = 0;
    while ((sublist = PyIter_Next(iterator1))) {    // PyIter_Next returns a new reference

        iterator2 = PyObject_GetIter(sublist);    // New reference
        if (iterator2 == NULL) {
            Py_DECREF(new); Py_DECREF(iterator1);
            Py_DECREF(sublist);
            return NULL;
        }

        while ((item = PyIter_Next(iterator2))) {

            PyList_SET_ITEM(new, index, item);
        
            index++;
        }
        Py_DECREF(iterator2);

        Py_DECREF(sublist);
    }
    Py_DECREF(iterator1);

    return new;
}

/**
 * @brief Return a list of subsets of size size from the set set, NULL if an error has occurred.
 * 
 * @param set a PySet
 * @param size the size of each subset (> 0)
 * @return PyObject* a new PyList, NULL if an error occurred
 */
PyObject *
PySet_SubSets(PyObject *set, Py_ssize_t size)
{
    if (PySet_GET_SIZE(set) == 0) {
        PyObject *new_set = PySet_New(NULL);
        PyObject *list = PyList_New(1);
        PyList_SET_ITEM(list, 0, new_set);
        return list;
    }
    if (size <= 0) {
        Py_ssize_t set_length = PySet_GET_SIZE(set);
        PyObject *list = PyList_New(set_length);
        for (Py_ssize_t index = 0; index < set_length; index++) {
            PyList_SET_ITEM(list, index, PySet_New(NULL));
        }
    }
    if (size == 1) {
        Py_ssize_t set_length = PySet_GET_SIZE(set);
        PyObject *list = PyList_New(set_length);
        if (list == NULL) {
            return NULL;
        }

        PyObject *iterator = PyObject_GetIter(set);
        PyObject *item;
        if (iterator == NULL) {
            Py_DECREF(list);
            return NULL;
        }

        Py_ssize_t index = 0;
        while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
            PyObject *new_set = PySet_New(NULL);
            if (PySet_Add(new_set, item) < 0) {
                Py_DECREF(list); Py_XDECREF(new_set);
                Py_DECREF(item); Py_DECREF(iterator);
                return NULL;
            }
            PyList_SET_ITEM(list, index, new_set);
            index++;
        }
        Py_DECREF(iterator);
        
        return list;
    }

    PyObject *list = PyList_New(0);
    if (list == NULL) {
        return NULL;
    }

    PyObject *myset = PySet_New(set); // Copy
    if (myset == NULL) {
        Py_DECREF(list);
        return NULL;
    }

    PyObject *item, *subset_list, *iterator, *subset;
    while(PySet_GET_SIZE(myset) > 0) {
        item = PySet_Pop(myset); // New reference
        if (item == NULL) {
            Py_DECREF(list); Py_DECREF(myset);
            return NULL;
        }

        subset_list = PySet_SubSets(myset, size-1);
        if (subset_list == NULL) {
            Py_DECREF(list); Py_DECREF(myset);
            Py_DECREF(item);
            return NULL;
        }

        iterator = PyObject_GetIter(subset_list);
        if (iterator == NULL) {
            Py_DECREF(list); Py_DECREF(myset);
            Py_DECREF(item); Py_DECREF(subset_list);
            return NULL;
        }
        while ((subset = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
            if (PySet_Add(subset, item) < 0) {
                Py_DECREF(list); Py_DECREF(myset);
                Py_DECREF(item); Py_DECREF(subset_list);
                Py_DECREF(iterator); Py_DECREF(subset);
                return NULL;
            }
            Py_INCREF(item);

            if (PyList_Append(list, subset) < 0) {
                Py_DECREF(list); Py_DECREF(myset);
                Py_DECREF(item); Py_DECREF(subset_list);
                Py_DECREF(iterator); Py_DECREF(subset);
                return NULL;
            }
        }
        Py_DECREF(iterator);

        Py_DECREF(subset_list);
        Py_DECREF(item);

        if (PySet_GET_SIZE(myset) < size) {
            break;
        }
    }

    Py_DECREF(myset);

    return list;
}

/**
 * @brief Return a list of set given a set of frozenset, NULL if an error has occurred.
 * 
 * @param set a PySet of PyFrozenSet
 * @return PyObject* a new PyList of PySet, NULL if an error occurred
 */
inline PyObject *
PyListOfPySet_FromPySetOfPyFrozenSet(PyObject *set)
{
    PyObject *iterator;
    PyObject *frozenset;

    PyObject *list = PyList_New(PySet_GET_SIZE(set));
    if (list == NULL) {
        return NULL;
    }

    iterator = PyObject_GetIter(set);    // New reference
    if (iterator == NULL) {
        Py_DECREF(list);
        return NULL;
    }

    Py_ssize_t index = 0;
    while ((frozenset = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference

        PyObject *new_set = PySet_New(frozenset);
        if (new_set == NULL) {
            Py_DECREF(list);
            Py_DECREF(iterator); Py_DECREF(frozenset);
            return NULL;
        }

        PyList_SET_ITEM(list, index, new_set);

        Py_DECREF(frozenset);
        index++;
    }
    Py_DECREF(iterator);

    return list;
}

/**
 * @brief Return a list of all subsets (empty set not included) from the set set sorted by size (in ascending order),
 * NULL if an error has occurred.
 * 
 * @param set a PySet
 * @return PyObject* a new PyList, NULL if an error occurred
 */
PyObject *
PySet_PowersetWihtoutEmptySet(PyObject *set)
{
    Py_ssize_t set_size = PySet_GET_SIZE(set);

    PyObject *subsets = PyList_New(set_size + 1);
    if (subsets == NULL) {
        return NULL;
    }

    PyObject *empty_list = PyList_New(0); // Empty Set is not added
    if (empty_list == NULL) {
        Py_DECREF(subsets);
        return NULL;
    }

    PyObject *size1_list = PySet_SubSets(set, 1); // List of subsets of size 1
    if (empty_list == NULL) {
        Py_DECREF(subsets); Py_DECREF(empty_list);
        return NULL;
    }

    PyList_SET_ITEM(subsets, 0, empty_list);
    PyList_SET_ITEM(subsets, 1, size1_list);

    for (Py_ssize_t size = 2; size <= set_size; size++)  {
        PyObject *frozen_sets = PySet_New(NULL); // Set of FrozenSet
        if (frozen_sets == NULL) {
            Py_DECREF(subsets);
            return NULL;
        }

        PyObject *subset_iterator = PyObject_GetIter(PyList_GET_ITEM(subsets, size-1)); // Iterator of subsets with length = size-1
        PyObject *subset;
        if (subset_iterator == NULL) {
            Py_DECREF(subsets); Py_DECREF(frozen_sets);
            return NULL;
        }

        while ((subset = PyIter_Next(subset_iterator))) {
            PyObject *set_iterator = PyObject_GetIter(set); // Iterator of the original set
            PyObject *item;
            if (set_iterator == NULL) {
                Py_DECREF(subsets); Py_DECREF(frozen_sets);
                Py_DECREF(subset_iterator); Py_DECREF(subset);
                return NULL;
            }

            while ((item = PyIter_Next(set_iterator))) {
                int contains = PySet_Contains(subset, item);
                if (contains < 0) {
                    Py_DECREF(subsets); Py_DECREF(frozen_sets);
                    Py_DECREF(subset_iterator); Py_DECREF(subset);
                    Py_DECREF(set_iterator); Py_DECREF(item);
                    return NULL;
                }

                if (!contains) {
                    if (PySet_Add(subset, item) < 0) {  // Modify the subset
                        Py_DECREF(subsets); Py_DECREF(frozen_sets);
                        Py_DECREF(subset_iterator); Py_DECREF(subset);
                        Py_DECREF(set_iterator); Py_DECREF(item);
                        return NULL;
                    }
                    Py_INCREF(item);

                    PyObject *new_set = PyFrozenSet_New(subset);
                    if (new_set == NULL) {
                        Py_DECREF(subsets); Py_DECREF(frozen_sets);
                        Py_DECREF(subset_iterator); Py_DECREF(subset);
                        Py_DECREF(set_iterator); Py_DECREF(item);
                        return NULL;
                    }

                    if (PySet_Add(frozen_sets, new_set) < 0) {
                        Py_DECREF(subsets); Py_DECREF(frozen_sets);
                        Py_DECREF(subset_iterator); Py_DECREF(subset);
                        Py_DECREF(set_iterator); Py_DECREF(item);
                        Py_DECREF(new_set);
                        return NULL;
                    }

                    if (PySet_Discard(subset, item) < 0) { // Restore the subset
                        Py_DECREF(subsets); Py_DECREF(frozen_sets);
                        Py_DECREF(subset_iterator); Py_DECREF(subset);
                        Py_DECREF(set_iterator); Py_DECREF(item);
                        return NULL;
                    }
                }

                Py_DECREF(item);
            }
            Py_DECREF(set_iterator);

            Py_DECREF(subset);
        }
        Py_DECREF(subset_iterator);

        PyObject *current_list = PyListOfPySet_FromPySetOfPyFrozenSet(frozen_sets);
        if (current_list == NULL) {
            Py_DECREF(subsets); Py_DECREF(frozen_sets);
            return NULL;
        }
        Py_DECREF(frozen_sets);

        PyList_SET_ITEM(subsets, size, current_list);
    }

    PyObject *list = PyList_ConcatItems(subsets);
    Py_DECREF(subsets);
    
    return list;
}

/**
 * @brief Return True if the list contains a set that is subset of the set superset,
 * False if it does not, -1 if an error has been encountered.
 * 
 * @param list a PyList of PySet
 * @param superset a PySet
 * @return int 1 if contained, 0 if not contained, -1 if an error occurred
 */
int
PyList_ContainsSubset(PyObject *list, PyObject *superset) {
    PyObject *iterator = PyObject_GetIter(list);
    PyObject *set;
    int issubset;

    if (iterator == NULL) {
        return -1;
    }

    while ((set = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        issubset = PySet_IsSubset(set, superset);
        if (issubset < 0) {
            Py_DECREF(set); Py_DECREF(iterator);
            return -1;
        }

        Py_DECREF(set);

        if (issubset) {
            Py_DECREF(iterator);
            return 1; // return True
        }
    }

    Py_DECREF(iterator);

    return 0; // return False
}