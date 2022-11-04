/**
 * @file relations.c
 * @author Jose Ruiz Alarcon
 * @brief Definition of the PyTypeObject QBAFARelations.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "relations.h"

/**
 * @brief Struct that defines the Object Type ARelations in a QBAF.
 * 
 */
typedef struct {
    PyObject_HEAD
    PyObject *relations;        /* set of tuples (Agent: QBAFArgument, Patient: QBAFArgument) */
    PyObject *agent_patients;   /* dictonary of (key, value) = (Agent, set of Patients) */
    PyObject *patient_agents;   /* dictonary of (key, value) = (Patient, set of Agents) */
} QBAFARelationsObject;

/**
 * @brief This function is used by the garbage collector to detect reference cycles.
 * 
 * @param self a object of type QBAFARelations
 * @param visit 
 * @param arg 
 * @return int 0 if the function was successful. Otherwise, -1.
 */
static int
QBAFARelations_traverse(QBAFARelationsObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->relations);
    Py_VISIT(self->agent_patients);
    Py_VISIT(self->patient_agents);
    return 0;
}

/**
 * @brief Drop the instance’s references to those of its members that may be Python objects, and set its pointers to those members to NULL.
 * 
 * @param self a object of type QBAFARelations
 * @return int 0 if the function was successful. Otherwise, -1.
 */
static int
QBAFARelations_clear(QBAFARelationsObject *self)
{
    Py_CLEAR(self->relations);
    Py_CLEAR(self->agent_patients);
    Py_CLEAR(self->patient_agents);
    return 0;
}

/**
 * @brief Destructor function that is called to free memory of a object that will no longer be used.
 * 
 * @param self a object of type QBAFARelations
 */
static void
QBAFARelations_dealloc(QBAFARelationsObject *self)
{
    PyObject_GC_UnTrack(self);
    QBAFARelations_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * @brief Constructor of a QBAFARelations.
 * 
 * @param type the Object type
 * @param args the argument values that might be used by the constructor
 * @param kwds the names of the argument values
 * @return PyObject* The created object of type QBAFARelations
 */
static PyObject *
QBAFARelations_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    QBAFARelationsObject *self;
    self = (QBAFARelationsObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->relations = PySet_New(NULL);
        if (self->relations == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->agent_patients = PyDict_New();
        if (self->agent_patients == NULL) {
            Py_DECREF(self->relations);
            Py_DECREF(self);
            return NULL;
        }
        self->patient_agents = PyDict_New();
        if (self->patient_agents == NULL) {
            Py_DECREF(self->relations);
            Py_DECREF(self->agent_patients);
            Py_DECREF(self);
            return NULL;
        }
    }
    return (PyObject *) self;
}

/**
 * @brief Return the object from dictionary dict which has a key key. If the key does not exist return a new set.
 * Return NULL with an exception set if an exception occurred.
 * 
 * @param dict the dictionary
 * @param key the key
 * @return PyObject* Borrowed reference
 */
static inline PyObject *
PyDict_GetItemDefaultPySet_New(PyObject *dict, PyObject *key)
{
    PyObject *set;
    int contains = PyDict_Contains(dict, key);
    if (contains < 0) { //PyDict_Contains returns -1 in case of error
        return NULL;
    }
    if (!contains) {
        set = PySet_New(NULL);      // Return new reference
        if (set == NULL)
            return NULL;
        // Add a new set to the dictionary
        if (PyDict_SetItem(dict, key, set) < 0) {
            Py_DECREF(set);
            return NULL;
        }
        return set;
    }
    return PyDict_GetItemWithError(dict, key);
}

/**
 * @brief Initializer of a QBAFARelations. It is called right after the constructor by the python interpreter.
 * 
 * @param self the Object 
 * @param args the argument values that might be used by the constructor
 * @param kwds the names of the argument values
 * @return int 0 if it was executed with no errors. Otherwise, -1.
 */
static int
QBAFARelations_init(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"relations", NULL};
    PyObject *relations, *tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &relations))
        return -1;

    if (relations) { // It will always be true

        if (!PySet_Check(relations) && !PyList_Check(relations)) {
            PyErr_SetString(PyExc_TypeError,
                            "relations parameter must be a set or a list");
            return -1;
        }

        // Initialize relations
        tmp = self->relations;
        relations = PySet_New(relations);           // It creates a new reference
        if (relations == NULL) { // If any item is not hashable it raises and error
            return -1;
        }
        self->relations = relations;
        Py_DECREF(tmp);                             // This instance stops owning the set created in the constructor

        // Initialize agent_patients & patient_agents
        PyObject *iterator = PyObject_GetIter(self->relations);
        PyObject *item;
        PyObject *agent, *patient;
        PyObject *set;

        if (iterator == NULL) {
            /* propagate error */
            return -1;
        }

        while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
            /* do something with item */
            if (!PyTuple_Check(item) || (PyTuple_Size(item) != 2)) {
                PyErr_SetString(PyExc_TypeError,
                            "every item of relations must be a tuple of size 2");
                Py_DECREF(item);
                break;
            }

            agent = PyTuple_GetItem(item, 0);       // Returns borrowed reference. NULL if the index is wrong.
            patient = PyTuple_GetItem(item, 1);     // Returns borrowed reference. NULL if the index is wrong.

            Py_DECREF(item);

            if (agent == NULL || patient == NULL) {
                break;
            }

            set = PyDict_GetItemDefaultPySet_New(self->agent_patients, agent);  // Return borrowed reference
            if (set == NULL) {
                break;
            }
            Py_INCREF(agent);
            // Add patient to the set
            if (PySet_Add(set, patient) < 0) {
                Py_DECREF(agent);
                break;
            }

            set = PyDict_GetItemDefaultPySet_New(self->patient_agents, patient);  // Return borrowed reference
            if (set == NULL) {
                break;
            }
            Py_INCREF(patient);
            // Add agent to the set
            if (PySet_Add(set, agent) < 0) {
                break;
            }
            
        }

        Py_DECREF(iterator);

        if (PyErr_Occurred()) {
            /* propagate error */
            return -1;
        }

    }
    return 0;
}

/**
 * @brief A list with the attributes of the class QBAFARelations
 * 
 */
static PyMemberDef QBAFARelations_members[] = {
    {NULL}  /* Sentinel */
};

/**
 * @brief Getter of the attribute relations.
 * 
 * @param self the QBAFARelations instance
 * @param closure 
 * @return PyObject* a copy of the relations set
 */
static PyObject *
QBAFArgument_getrelations(QBAFARelationsObject *self, void *closure)
{
    return PySet_New(self->relations);
}

/**
 * @brief A list with the setters and getters of the class QBAFARelations
 * 
 */
static PyGetSetDef QBAFARelations_getsetters[] = {
    {"relations", (getter) QBAFArgument_getrelations, NULL,
     "copy of relations", NULL},
    {NULL}  /* Sentinel */
};

/**
 * @brief Return the string format of a QBAFARelations object
 * 
 * @param self the QBAFARelations instance
 * @param Py_UNUSED 
 * @return PyObject* the string representing the object
 */
static PyObject *
QBAFARelations_str(QBAFARelationsObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromFormat("QBAFARelations%S", self->relations);
}

/**
 * @brief List of functions of the class QBAFARelations.
 * 
 */
static PyMethodDef QBAFARelations_methods[] = {
    {NULL}  /* Sentinel */
};

/**
 * @brief Python definition for the class QBAFARelations
 * 
 */
static PyTypeObject QBAFARelationsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "qbaf.QBAFARelations",
    .tp_doc = PyDoc_STR("QBAFARelations objects"),
    .tp_basicsize = sizeof(QBAFARelationsObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_new = QBAFARelations_new,
    .tp_init = (initproc) QBAFARelations_init,
    .tp_dealloc = (destructor) QBAFARelations_dealloc,
    .tp_traverse = (traverseproc) QBAFARelations_traverse,
    .tp_clear = (inquiry) QBAFARelations_clear,
    .tp_members = QBAFARelations_members,
    .tp_methods = QBAFARelations_methods,
    .tp_getset = QBAFARelations_getsetters,
    .tp_str = (reprfunc) QBAFARelations_str,                      // __str__
    .tp_repr = (reprfunc) QBAFARelations_str,                     // __repr__
};

/**
 * @brief Get the QBAFARelationsType object created above that defines the class QBAFARelations
 * 
 * @return PyTypeObject* a pointer to the QBAFARelations class definition
 */
PyTypeObject *get_QBAFARelationsType() {
    return &QBAFARelationsType;
}