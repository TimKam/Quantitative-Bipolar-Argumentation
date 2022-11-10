/**
 * @file relations.c
 * @author Jose Ruiz Alarcon
 * @brief Definition of the PyTypeObject QBAFARelations.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "relations.h"
#include "qbaf_utils.h"

/**
 * @brief Struct that defines the Object Type ARelations in a QBAF.
 * 
 */
//typedef struct {
//    PyObject_HEAD
//    PyObject *relations;        /* set of tuples (Agent: QBAFArgument, Patient: QBAFArgument) */
//    PyObject *agent_patients;   /* dictonary of (key, value) = (Agent, set of Patients) */
//    PyObject *patient_agents;   /* dictonary of (key, value) = (Patient, set of Agents) */
//} QBAFARelationsObject;

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
 * @param args the argument values that might be used by the initializator
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
            Py_INCREF(patient);
            // Add patient to the set
            if (PySet_Add(set, patient) < 0) {
                Py_DECREF(patient);
                break;
            }

            set = PyDict_GetItemDefaultPySet_New(self->patient_agents, patient);  // Return borrowed reference
            if (set == NULL) {
                break;
            }
            Py_INCREF(agent);
            // Add agent to the set
            if (PySet_Add(set, agent) < 0) {
                Py_DECREF(agent);
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
 * @return PyObject* the string representing the object
 */
static PyObject *
QBAFARelations___str__(QBAFARelationsObject *self)
{
    return PyUnicode_FromFormat("QBAFARelations%S", self->relations);
}

/**
 * @brief Return the amount of relations of the instance.
 * 
 * @param self instance of QBAFARelations
 * @return Py_ssize_t length of set relations
 */
static Py_ssize_t
QBAFARelations___len__(QBAFARelationsObject *self)
{
    return PySet_Size(self->relations);
}

/**
 * @brief Return whether or not exists the relation (agent, patient) in this instance.
 * 
 * @param self instance of QBAFARelations
 * @param key a tuple (agent: QBAFArgument, patient: QBAFArgument)
 * @return int 1 if found, 0 if not found, and -1 if an error is encountered
 */
static int
QBAFARelations___contains__(QBAFARelationsObject *self, PyObject *key)
{
    if (!PyTuple_Check(key) || (PyTuple_Size(key) != 2)) {
        PyErr_SetString(PyExc_TypeError,
                        "relation must be a tuple of size 2");
        return -1;
    }

    return PySet_Contains(self->relations, key);
}

/**
 * @brief Return the patients that undergo the effect of a certain action (e.g. attack, support)
 * initiated by the agent. Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param agent instance of QBAFArgument
 * @return PyObject* list of QBAFArgument
 */
PyObject *
_QBAFARelations_patients(QBAFARelationsObject *self, PyObject *agent) {
    PyObject *set, *list;
    int contains = PyDict_Contains(self->agent_patients, agent);
    if (contains < 0)
        return NULL;
    if (contains) {
        set = PyDict_GetItem(self->agent_patients, agent);
        list = PyList_New(PySet_GET_SIZE(set));

        if (list == NULL)
            return NULL;
        
        PyObject *iterator = PyObject_GetIter(set);
        PyObject *item;

        if (iterator == NULL) {
            /* propagate error */
            return NULL;
        }

        Py_ssize_t index = 0;
        while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
            /* do something with item */

            PyList_SET_ITEM(list, index, item);
            
            index++;
        }

        Py_DECREF(iterator);

        return list;
    }

    return PyList_New(0);
}

/**
 * @brief Return the patients that undergo the effect of a certain action (e.g. attack, support)
 * initiated by the agent. Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param args the argument values (agent: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* list of QBAFArgument 
 */
static PyObject *
QBAFARelations_patients(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"agent", NULL};
    PyObject *agent;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &agent))
        return NULL;

    return _QBAFARelations_patients(self, agent);
}

/**
 * @brief Return the agents that initiate a certain action (e.g. attack, support)
 * which effects are undergone by the patient. Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param patient instance of QBAFArgument
 * @return PyObject* list of QBAFArgument
 */
PyObject *
_QBAFARelations_agents(QBAFARelationsObject *self, PyObject *patient) {
    PyObject *set, *list;
    int contains = PyDict_Contains(self->patient_agents, patient);
    if (contains < 0)
        return NULL;
    if (contains) {
        set = PyDict_GetItem(self->patient_agents, patient);
        list = PyList_New(PySet_GET_SIZE(set));

        if (list == NULL)
            return NULL;
        
        PyObject *iterator = PyObject_GetIter(set);
        PyObject *item;

        if (iterator == NULL) {
            /* propagate error */
            return NULL;
        }

        Py_ssize_t index = 0;
        while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
            /* do something with item */

            PyList_SET_ITEM(list, index, item);
            
            index++;
        }

        Py_DECREF(iterator);

        return list;
    }

    return PyList_New(0);
}

/**
 * @brief Return the agents that initiate a certain action (e.g. attack, support)
 * which effects are undergone by the patient. Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param args the argument values (patient: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* list of QBAFArgument
 */
static PyObject *
QBAFARelations_agents(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"patient", NULL};
    PyObject *patient;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &patient))
        return NULL;

    return _QBAFARelations_agents(self, patient);
}

/**
 * @brief Return whether or not exists the relation (agent, patient) in this instance.
 * Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param args the argument values (agent: QBAFArgument, patient: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyBool
 */
static PyObject *
QBAFARelations_contains(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"agent", "patient", NULL};
    PyObject *agent, *patient;
    PyObject *tuple;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    Py_INCREF(agent);
    Py_INCREF(patient);
    tuple = PyTuple_Pack(2, agent, patient);    // new reference
    if (tuple == NULL) {
        Py_DECREF(agent);
        Py_DECREF(patient);
        return NULL;
    }
    
    int contains = QBAFARelations___contains__(self, tuple);
    if (contains < 0) {
        Py_DECREF(tuple);
        return NULL;
    }

    Py_DECREF(tuple);

    if (contains)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

/**
 * @brief Add the relation (agent, patient) to this instance. Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param args the argument values (agent: QBAFArgument, patient: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_NONE
 */
static PyObject *
QBAFARelations_add(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"agent", "patient", NULL};
    PyObject *agent, *patient;
    PyObject *tuple, *set;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    Py_INCREF(agent);
    Py_INCREF(patient);
    tuple = PyTuple_Pack(2, agent, patient);    // new reference
    if (tuple == NULL) {
        Py_DECREF(agent);
        Py_DECREF(patient);
        return NULL;
    }

    int contains = PySet_Contains(self->relations, tuple);
    if (contains < 0) {
        Py_DECREF(tuple);
        return NULL;
    }
    if (contains) {
        Py_DECREF(tuple);
        Py_RETURN_NONE;
    }

    // If not contains it is inserted

    if (PySet_Add(self->relations, tuple) < 0) {
        Py_DECREF(tuple);
        return NULL;
    }

    set = PyDict_GetItemDefaultPySet_New(self->agent_patients, agent);  // Return borrowed reference
    if (set == NULL) {
        return NULL;
    }
    Py_INCREF(patient);
    // Add patient to the set
    if (PySet_Add(set, patient) < 0) {
        Py_DECREF(patient);
        return NULL;
    }

    set = PyDict_GetItemDefaultPySet_New(self->patient_agents, patient);  // Return borrowed reference
    if (set == NULL) {
        return NULL;
    }
    Py_INCREF(agent);
    // Add agent to the set
    if (PySet_Add(set, agent) < 0) {
        Py_DECREF(agent);
        return NULL;
    }

    Py_RETURN_NONE;
}

/**
 * @brief Remove the relation (agent, patient) of this instance. Return NULL if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param args the argument values (agent: QBAFArgument, patient: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_NONE
 */
static PyObject *
QBAFARelations_remove(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"agent", "patient", NULL};
    PyObject *agent, *patient;
    PyObject *tuple, *set;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    Py_INCREF(agent);
    Py_INCREF(patient);
    tuple = PyTuple_Pack(2, agent, patient);    // new reference
    if (tuple == NULL) {
        Py_DECREF(agent);
        Py_DECREF(patient);
        return NULL;
    }

    int contains = PySet_Contains(self->relations, tuple);
    if (contains < 0) {
        Py_DECREF(tuple);
        return NULL;
    }
    if (!contains) {
        Py_DECREF(tuple);
        Py_RETURN_NONE;
    }

    // If contains it is removed

    if (PySet_Discard(self->relations, tuple) < 0) {
        Py_DECREF(tuple);
        return NULL;
    }

    Py_DECREF(tuple);

    set = PyDict_GetItem(self->agent_patients, agent);  // Return borrowed reference
    if (set == NULL) {
        return NULL;
    }
    // Remove patient from the set
    if (PySet_Discard(set, patient) < 0) {
        return NULL;
    }

    set = PyDict_GetItem(self->patient_agents, patient);  // Return borrowed reference
    if (set == NULL) {
        return NULL;
    }
    // Add agent to the set
    if (PySet_Discard(set, agent) < 0) {
        return NULL;
    }

    Py_RETURN_NONE;
}

/**
 * @brief Return a copy of this instance.
 * New references are created for the copy, except for the QBAFArgument objects.
 * 
 * @param self instance of QBAFARelations
 * @param Py_UNUSED 
 * @return PyObject* new instance of QBAFARelations
 */
PyObject *
QBAFARelations_copy(QBAFARelationsObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject *kwds = NULL;
    PyObject *args = PyTuple_Pack(1, self->relations);
    if (args == NULL)
        return NULL;

    QBAFARelationsObject *copy = QBAFARelations_new(Py_TYPE(self), args, kwds);
    if (copy == NULL) {
        Py_DECREF(args);
        return NULL;
    }
    if (QBAFARelations_init(copy, args, kwds) < 0) {
        Py_DECREF(copy);
        Py_DECREF(args);
        return NULL;
    }
    Py_DECREF(args);
    return copy;
}

/**
 * @brief Return 1 if their relations are disjoint, 0 if they are not, and -1 if an error is encountered.
 * 
 * @param self a QBAFARelations instance (not NULL)
 * @param other a different QBAFARelations instance (not NULL)
 * @return int 1 if they are disjoint, 0 if they are not, and -1 if an error is encountered
 */
int
_QBAFARelations_isDisjoint(QBAFARelationsObject *self, QBAFARelationsObject *other) {
    return PySet_IsDisjoint(self->relations, other->relations);
}

/**
 * @brief Return True if their relations are disjoint, False if they are not, and NULL if an error is encountered.
 * 
 * @param self a QBAFARelations instance (not NULL)
 * @param other a different QBAFARelations instance (not NULL)
 * @return PyObject* Py_True if they are disjoint, Py_False if they are not, and NULL if an error is encountered
 */
static PyObject *
QBAFARelations_isDisjoint(QBAFARelationsObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", NULL};
    PyObject * other;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &other))
        return NULL;

    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "Disjoint operation of 'QBAFARelations' with instance of a different type not supported");
        return NULL;
    }

    int disjoint = _QBAFARelations_isDisjoint(self, (QBAFARelationsObject*) other);
    if (disjoint < 0)
        return NULL;
    
    if (disjoint)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

/**
 * @brief List of functions of the class QBAFARelations.
 * 
 */
static PyMethodDef QBAFARelations_methods[] = {
    {"patients", (PyCFunctionWithKeywords) QBAFARelations_patients, METH_VARARGS | METH_KEYWORDS,
    "Return the patients that undergo the effect of a certain action (e.g. attack, support) initiated by the agent."
    },
    {"agents", (PyCFunctionWithKeywords) QBAFARelations_agents, METH_VARARGS | METH_KEYWORDS,
    "Return the agents that initiate a certain action (e.g. attack, support) which effects are undergone by the patient."
    },
    {"contains", (PyCFunctionWithKeywords) QBAFARelations_contains, METH_VARARGS | METH_KEYWORDS,
    "Return whether or not exists the relation (agent, patient) in this instance."
    },
    {"add", (PyCFunctionWithKeywords) QBAFARelations_add, METH_VARARGS | METH_KEYWORDS,
    "Add the relation (agent, patient) to the instance."
    },
    {"remove", (PyCFunctionWithKeywords) QBAFARelations_remove, METH_VARARGS | METH_KEYWORDS, 
    "Remove the relation (agent, patient) from the instance."
    },
    {"__copy__", (PyCFunction) QBAFARelations_copy, METH_NOARGS,
    "Return a copy of the instance."
    },
    {"copy", (PyCFunction) QBAFARelations_copy, METH_NOARGS,
    "Return a copy of the instance."
    },
    {"isdisjoint", (PyCFunctionWithKeywords) QBAFARelations_isDisjoint, METH_VARARGS | METH_KEYWORDS, 
    "Return True if their relations are disjoint, False if they are not."
    },
    {NULL}  /* Sentinel */
};

static PySequenceMethods QBAFARelations_sequencemethods = {
    .sq_length = (lenfunc) QBAFARelations___len__,              // __len__
    .sq_contains = (objobjproc) QBAFARelations___contains__,    // __contains__
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
    .tp_str = (reprfunc) QBAFARelations___str__,                // __str__
    .tp_repr = (reprfunc) QBAFARelations___str__,               // __repr__
    .tp_as_sequence = &QBAFARelations_sequencemethods,          // __len__, __contains__
};

/**
 * @brief Get the QBAFARelationsType object created above that defines the class QBAFARelations
 * 
 * @return PyTypeObject* a pointer to the QBAFARelations class definition
 */
PyTypeObject *get_QBAFARelationsType() {
    return &QBAFARelationsType;
}

/**
 * @brief Create a new object QBAFARelations.
 * 
 * @param relations a set/list of tuples (Agent: QBAFArgument, Patient QBAFArgument)
 * @return PyObject* New reference
 */
PyObject *
QBAFARelations_Create(PyObject *relations)
{
    PyObject *kwds = NULL;
    PyObject *args = PyTuple_Pack(1, relations);
    if (args == NULL)
        return NULL;

    QBAFARelationsObject *new = QBAFARelations_new(&QBAFARelationsType, args, kwds);
    if (new == NULL) {
        Py_DECREF(args);
        return NULL;
    }

    if (QBAFARelations_init(new, args, kwds) < 0) {
        Py_DECREF(new);
        Py_DECREF(args);
        return NULL;
    }

    Py_DECREF(args);
    return (PyObject*) new;
}

/**
 * @brief Return True if all the arguments of self are contained in arguments, if not False,
 *        and -1 if there is an error.
 * 
 * @param self an instance of QBAFARelations
 * @param arguments a set of QBAFArgument
 * @return int 1 if contained, 0 if not contained, and -1 if an error is encountered
 */
int
QBAFARelations_ArgsContained(QBAFARelationsObject *self, PyObject *arguments) {
    PyObject *iterator = PyObject_GetIter(self->relations);
    PyObject *item;
    PyObject *agent, *patient;
    int contains;

    if (iterator == NULL) {
        /* propagate error */
        return -1;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        agent = PyTuple_GetItem(item, 0);       // Returns borrowed reference. NULL if the index is wrong.
        patient = PyTuple_GetItem(item, 1);     // Returns borrowed reference. NULL if the index is wrong.
        Py_DECREF(item);

        if (agent == NULL || patient == NULL) {
            Py_DECREF(iterator);
            return -1;
        }

        contains = PySet_Contains(arguments, agent);
        if (contains < 0) {
            Py_DECREF(iterator);
            return -1;
        }
        if (!contains) {
            Py_DECREF(iterator);
            return 0;   // return False
        }

        contains = PySet_Contains(arguments, patient);
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
 * @brief Return True if if the argument is contained, False if not contained, and -1 if an error is encountered.
 * 
 * @param self an instance of QBAFARelations
 * @param argument an instance of QBAFArgument
 * @return int 1 if contained, 0 if not contained, and -1 if an error is encountered
 */
int
QBAFARelations_contains_argument(QBAFARelationsObject *self, PyObject *argument) {
    PyObject *tuple, *list;

    tuple = PyTuple_Pack(1, argument); // New reference

    list = QBAFARelations_agents(self, tuple, NULL);
    if (list == NULL) {
        Py_DECREF(tuple);
        return -1;
    }
    if (PyList_GET_SIZE(list) > 0) {
        Py_DECREF(tuple);
        return 1;   // return True
    }

    list = QBAFARelations_patients(self, tuple, NULL);
    if (list == NULL) {
        Py_DECREF(tuple);
        return -1;
    }

    Py_DECREF(tuple);
    if (PyList_GET_SIZE(list) > 0) {
        Py_DECREF(tuple);
        return 1;   // return True
    }

    Py_DECREF(tuple);

    return 0;   // return False
}

/**
 * @brief Return whether or not exists the relation (agent, patient) in this instance.
 * Return -1 if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param agent instance of QBAFArgument
 * @param patient instance of QBAFArgument
 * @return PyObject* 1 if is contained, 0 if not contained, -1 if an error has occurred
 */
int
_QBAFARelations_contains(QBAFARelationsObject *self, PyObject *agent, PyObject *patient)
{
    PyObject *tuple;

    Py_INCREF(agent);
    Py_INCREF(patient);
    tuple = PyTuple_Pack(2, agent, patient);    // new reference
    if (tuple == NULL) {
        Py_DECREF(agent);
        Py_DECREF(patient);
        return NULL;
    }
    
    int contains = QBAFARelations___contains__(self, tuple);
    if (contains < 0) {
        Py_DECREF(tuple);
        return NULL;
    }

    Py_DECREF(tuple);

    if (contains)
        return 1;
    return 0;
}

/**
 * @brief Add the relation (agent, patient) to this instance. Return -1 if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param agent instance of QBAFArgument
 * @param patient instance of QBAFArgument
 * @return int 0 if success, -1 in case of error
 */
int
_QBAFARelations_add(QBAFARelationsObject *self, PyObject *agent, PyObject *patient)
{
    static char *kwds = NULL;
    PyObject *args;

    Py_INCREF(agent);
    Py_INCREF(patient);
    args = PyTuple_Pack(2, agent, patient); // New reference
    if (args == NULL) {
        Py_DECREF(agent);
        Py_DECREF(patient);
        return -1;
    }

    PyObject *none = QBAFARelations_add(self, args, kwds);
    Py_DECREF(args);
    if (none == NULL) {
        return -1;
    }

    Py_DECREF(none);

    return 0;
}

/**
 * @brief Remove the relation (agent, patient) to this instance. Return -1 if an error has ocurred.
 * 
 * @param self instance of QBAFARelations
 * @param agent instance of QBAFArgument
 * @param patient instance of QBAFArgument
 * @return int 0 if success, -1 in case of error
 */
int
_QBAFARelations_remove(QBAFARelationsObject *self, PyObject *agent, PyObject *patient)
{
    static char *kwds = NULL;
    PyObject *args;

    Py_INCREF(agent);
    Py_INCREF(patient);
    args = PyTuple_Pack(2, agent, patient); // New reference
    if (args == NULL) {
        Py_DECREF(agent);
        Py_DECREF(patient);
        return -1;
    }

    PyObject *none = QBAFARelations_remove(self, args, kwds);
    Py_DECREF(args);
    if (none == NULL) {
        return -1;
    }

    Py_DECREF(none);

    return 0;
}