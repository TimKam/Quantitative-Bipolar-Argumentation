/**
 * @file framework.c
 * @author Jose Ruiz Alarcon
 * @brief Definition of the PyTypeObject QBAFramework.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "framework.h"
#include "relations.h"
#include "qbaf_utils.h"

#define TRUE 1
#define FALSE 0

/**
 * @brief Struct that defines the Object Type Framework in a QBAF.
 * 
 */
typedef struct {
    PyObject_HEAD
    PyObject *arguments;        /* a set of QBAFArgument */
    PyObject *initial_weights;  /* a dictionary (argument: QBAFArgument, initial_weight: double) */
    PyObject *attack_relations; /* an instance of QBAFARelations */
    PyObject *support_relations;/* an instance of QBAFARelations */
    PyObject *final_weights;    /* a dictionary (argument: QBAFArgument, final_weight: double) */
    int       modified;         /* 0 if the framework has not been modified after calculating the final weights. Otherwise, 1 */
} QBAFrameworkObject;

/**
 * @brief This function is used by the garbage collector to detect reference cycles.
 * 
 * @param self a object of type QBAFramework
 * @param visit 
 * @param arg 
 * @return int 0 if the function was successful. Otherwise, -1.
 */
static int
QBAFramework_traverse(QBAFrameworkObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->arguments);
    Py_VISIT(self->initial_weights);
    Py_VISIT(self->attack_relations);
    Py_VISIT(self->support_relations);
    Py_VISIT(self->final_weights);
    return 0;
}

/**
 * @brief Drop the instance’s references to those of its members that may be Python objects, and set its pointers to those members to NULL.
 * 
 * @param self a object of type QBAFramework
 * @return int 0 if the function was successful. Otherwise, -1.
 */
static int
QBAFramework_clear(QBAFrameworkObject *self)
{
    Py_CLEAR(self->arguments);
    Py_CLEAR(self->initial_weights);
    Py_CLEAR(self->attack_relations);
    Py_CLEAR(self->support_relations);
    Py_CLEAR(self->final_weights);
    return 0;
}

/**
 * @brief Destructor function that is called to free memory of a object that will no longer be used.
 * 
 * @param self a object of type QBAFramework
 */
static void
QBAFramework_dealloc(QBAFrameworkObject *self)
{
    PyObject_GC_UnTrack(self);
    QBAFramework_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * @brief Constructor of a QBAFramework.
 * 
 * @param type the Object type
 * @param args the argument values that might be used by the constructor
 * @param kwds the names of the argument values
 * @return PyObject* The created object of type QBAFramework
 */
static PyObject *
QBAFramework_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    QBAFrameworkObject *self;
    self = (QBAFrameworkObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        Py_INCREF(Py_None);
        self->arguments = Py_None;
        Py_INCREF(Py_None);
        self->initial_weights = Py_None;
        Py_INCREF(Py_None);
        self->attack_relations = Py_None;
        Py_INCREF(Py_None);
        self->support_relations = Py_None;
        Py_INCREF(Py_None);
        self->final_weights = Py_None;
        self->modified = TRUE;
    }
    return (PyObject *) self;
}

/**
 * @brief It creates a PyDict from a list of keys and a list of values.
 * 
 * @param keys a list of keys (not NULL)
 * @param values a list of values (not NULL) with same size as keys
 * @return PyObject* New reference
 */
static inline PyObject *
PyDict_FromLists(PyObject *keys, PyObject *values) {
    PyObject *key_iterator, *value_iterator;
    PyObject *key_item, *value_item;

    key_iterator = PyObject_GetIter(keys);
    if (key_iterator == NULL) {
        return NULL;
    }

    value_iterator = PyObject_GetIter(values);
    if (value_iterator == NULL) {
        Py_DECREF(key_iterator);
        return NULL;
    }

    PyObject *dict = PyDict_New();
    if (dict == NULL) {
        Py_DECREF(key_iterator);
        Py_DECREF(value_iterator);
        return NULL;
    }

    while ((key_item = PyIter_Next(key_iterator))) {    // PyIter_Next returns a new reference
        value_item = PyIter_Next(value_iterator);
        if (value_item == NULL) {
            Py_DECREF(key_item);
            Py_DECREF(key_iterator);
            Py_DECREF(value_iterator);
            Py_DECREF(dict);
            return NULL; 
        }

        if (PyDict_SetItem(dict, key_item, value_item) < 0) {
            Py_DECREF(key_item);
            Py_DECREF(value_item);
            Py_DECREF(key_iterator);
            Py_DECREF(value_iterator);
            Py_DECREF(dict);
            return NULL;
        }
    }

    Py_DECREF(key_iterator);
    Py_DECREF(value_iterator);

    return dict;
}

/**
 * @brief It creates a list of PyFloat from a list of numeric (PyFloat or PyLong),
 *        return NULL if an error occurred and raises an exception.
 *        Note: The error description assumes this is run in the init.
 * 
 * @param list a PyList (not NULL) of numeric values
 * @return PyObject* a new PyList
 */
static inline PyObject *
PyListFloat_FromPyListNumeric(PyObject *list) {
    PyObject *new = PyList_New(PyList_GET_SIZE(list));  // New reference
    if (new == NULL)
        return NULL;

    PyObject *iterator = PyObject_GetIter(list);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(new);
        return NULL;
    }

    Py_ssize_t index = 0;
    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference

        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "all items of initial_weights must be of a numeric type (int, float)");
            Py_DECREF(item);
            Py_DECREF(iterator);
            Py_DECREF(new);
            return NULL;
        }

        if (PyLong_Check(item)) {
            // Transform the PyLong to PyFloat
            double weight = PyLong_AsDouble(item);
            Py_DECREF(item);
            if (weight == -1.0 && PyErr_Occurred()) {
                Py_DECREF(iterator);
                Py_DECREF(new);
                return NULL;
            }

            item = PyFloat_FromDouble(weight);  // New reference
            if (item == NULL) {
                PyErr_SetString(PyExc_ValueError, "float could not be created");
                Py_DECREF(iterator);
                Py_DECREF(new);
                return NULL;
            }
        }

        PyList_SET_ITEM(new, index, item);

        index++;
    }

    Py_DECREF(iterator);

    return new;
}

/**
 * @brief Initializer of a QBAFramework instance. It is called right after the constructor by the python interpreter.
 * 
 * @param self the Object 
 * @param args the argument values that might be used by the initializer
 * @param kwds the names of the argument values
 * @return int 0 if it was executed with no errors. Otherwise, -1.
 */
static int
QBAFramework_init(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"arguments", "initial_weights", "attack_relations", "support_relations", NULL};
    PyObject *arguments, *initial_weights, *attack_relations, *support_relations, *tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO|", kwlist,
                                     &arguments, &initial_weights, &attack_relations, &support_relations))
        return -1;

    if (!PyList_Check(arguments)) {
        PyErr_SetString(PyExc_TypeError, "arguments must be of type list");
        return -1;
    }

    if (!PyList_Check(initial_weights)) {
        PyErr_SetString(PyExc_TypeError, "initial_weights must be of type list");
        return -1;
    }

    if (PyList_GET_SIZE(arguments) != PyList_GET_SIZE(initial_weights)) {
        PyErr_SetString(PyExc_ValueError, "the lengths of arguments and initial_weights must be equal");
        return -1;
    }
    
    // Initialize arguments
    tmp = self->arguments;
    self->arguments = PySet_New(arguments);
    if (self->arguments == NULL) {
        /* propagate error*/
        self->arguments = tmp;
        return -1;
    }
    Py_DECREF(tmp);

    // Check that all init weights are numerical values
    initial_weights = PyListFloat_FromPyListNumeric(initial_weights);   // New reference
    if (initial_weights == NULL) {
        return -1;
    }

    // Initialize initial_weights
    tmp = self->initial_weights;
    self->initial_weights = PyDict_FromLists(arguments, initial_weights);
    Py_DECREF(initial_weights);
    if (self->initial_weights == NULL) {
        /* propagate error*/
        self->initial_weights = tmp;
        return -1;
    }
    Py_DECREF(tmp);

    // Initialize attack relations
    tmp = self->attack_relations;
    self->attack_relations = QBAFARelations_Create(attack_relations);
    if (self->attack_relations == NULL) {
        /* propagate error*/
        self->attack_relations = tmp;
        return -1;
    }
    Py_DECREF(tmp);

    // Initialize support relations
    tmp = self->support_relations;
    self->support_relations = QBAFARelations_Create(support_relations);
    if (self->support_relations == NULL) {
        /* propagate error*/
        self->support_relations = tmp;
        return -1;
    }
    Py_DECREF(tmp);

    // Check that the arguments of attack relations are in the arguments
    int contained = QBAFARelations_ArgsContained(self->attack_relations, self->arguments);
    if (contained < 0) {
        return -1;
    }
    if (!contained)  {
        PyErr_SetString(PyExc_ValueError, "all relation components of attack_relations must be in arguments");
        return -1;
    }

    // Check that the arguments of support relations are in the arguments
    contained = QBAFARelations_ArgsContained(self->support_relations, self->arguments);
    if (contained < 0) {
        return -1;
    }
    if (!contained)  {
        PyErr_SetString(PyExc_ValueError, "all relation components of support_relations must be in arguments");
        return -1;
    }

    // Check attack and support relations are disjoint
    int disjoint = _QBAFARelations_isDisjoint(self->attack_relations, self->support_relations);
    if (disjoint < 0) {
        return -1;
    }
    if (!disjoint) {
        PyErr_SetString(PyExc_ValueError, "attack_relations and support_relations must be disjoint");
        return -1;
    }

    return 0;
}

/**
 * @brief A list with the attributes of the class QBAFramework
 * 
 */
static PyMemberDef QBAFramework_members[] = {
    {NULL}  /* Sentinel */
};

/**
 * @brief Getter of the attribute arguments.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* copy of a set of QBAFramework
 */
static PyObject *
QBAFramework_getarguments(QBAFrameworkObject *self, void *closure)
{
    return PySet_New(self->arguments);
}

/**
 * @brief Getter of the attribute initial_weights.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* copy of a dict of (argument: QBAFArgument, initial_weight: float)
 */
static PyObject *
QBAFramework_getinitial_weights(QBAFrameworkObject *self, void *closure)
{
    return PyDict_Copy(self->initial_weights);
}

/**
 * @brief Getter of the attribute attack_relations.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* a instance of QBAFARelations 
 */
static PyObject *
QBAFramework_getattack_relations(QBAFrameworkObject *self, void *closure)
{
    Py_INCREF(self->attack_relations);
    return self->attack_relations;
}

/**
 * @brief Getter of the attribute support_relations.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* a instance of QBAFARelations 
 */
static PyObject *
QBAFramework_getsupport_relations(QBAFrameworkObject *self, void *closure)
{
    Py_INCREF(self->support_relations);
    return self->support_relations;
}

/**
 * @brief A list with the setters and getters of the class QBAFramework
 * 
 */
static PyGetSetDef QBAFramework_getsetters[] = {
    {"arguments", (getter) QBAFramework_getarguments, NULL,
     "Return a copy of the arguments of the instance.", NULL},
    {"initial_weights", (getter) QBAFramework_getinitial_weights, NULL,
     "Return a copy of the initial weights.", NULL},
    {"attack_relations", (getter) QBAFramework_getattack_relations, NULL,
     "Return the attack relations of the instance.", NULL},
    {"support_relations", (getter) QBAFramework_getsupport_relations, NULL,
     "Return the support relations of the instance.", NULL},
    {NULL}  /* Sentinel */
};

/**
 * @brief Modify the initial weight of the Argument argument.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument, initial_weight: float)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_modify_initial_weights(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", "initial_weight", NULL};
    PyObject *argument, *initial_weight;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &argument, &initial_weight))
        return NULL;

    if (!PyFloat_Check(initial_weight) && !PyLong_Check(initial_weight)) {
        PyErr_SetString(PyExc_TypeError, "initial_weight must be of a numeric type");
        return NULL;
    }

    if (PyLong_Check(initial_weight)) {
        // Transform the PyLong to PyFloat
        double weight = PyLong_AsDouble(initial_weight);
        if (weight == -1.0 && PyErr_Occurred()) {
            return NULL;
        }

        initial_weight = PyFloat_FromDouble(weight);    // New reference
        if (initial_weight == NULL) {
            PyErr_SetString(PyExc_ValueError, "initial_weight could not be transformed to float");
            return NULL;
        }
    } else {  
        Py_INCREF(initial_weight);
    }

    Py_INCREF(argument);
    if (PyDict_SetItem(self->initial_weights, argument, initial_weight) < 0) {
        Py_DECREF(argument);
        Py_DECREF(initial_weight);
        return NULL;
    }

    Py_DECREF(initial_weight);

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Return the initial weight of the Argument argument, NULL in case of error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyFloat
 */
static PyObject *
QBAFramework_initial_weight(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", NULL};
    PyObject *argument;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &argument))
        return NULL;

    return PyDict_GetItemWithError(self->initial_weights, argument);
}

/**
 * @brief Add an Argument to the Framework. If it exists already it does nothing.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument, initial_weight: float)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_add_argument(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", "initial_weight", NULL};
    PyObject *argument, *initial_weight = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist,
                                     &argument, &initial_weight))
        return NULL;

    double default_initial_weight = 0.0;

    if (initial_weight == NULL) {

        initial_weight = PyFloat_FromDouble(default_initial_weight);    // New reference
        if (initial_weight == NULL)
            return NULL;

    } else {

        if (!PyFloat_Check(initial_weight) && !PyLong_Check(initial_weight)) {
            PyErr_SetString(PyExc_TypeError, "initial_weight must be of a numeric type");
            return NULL;
        }

        if (PyLong_Check(initial_weight)) {
            // Transform the PyLong to PyFloat
            double weight = PyLong_AsDouble(initial_weight);
            if (weight == -1.0 && PyErr_Occurred()) {
                return NULL;
            }

            initial_weight = PyFloat_FromDouble(weight);    // New reference
            if (initial_weight == NULL) {
                PyErr_SetString(PyExc_ValueError, "initial_weight could not be transformed to float");
                return NULL;
            }
        } else {  
            Py_INCREF(initial_weight);
        }

    }

    int contains = PySet_Contains(self->arguments, argument);
    if (contains < 0) {
        Py_DECREF(initial_weight);
        return NULL;
    }
    if (contains) { // If the argument already exists it does nothing
        Py_RETURN_NONE;
    }

    Py_INCREF(argument);
    if (PyDict_SetItem(self->initial_weights, argument, initial_weight) < 0) {
        Py_DECREF(argument);
        Py_DECREF(initial_weight);
        return NULL;
    }

    Py_DECREF(initial_weight);

    Py_INCREF(argument);
    if (PySet_Add(self->arguments, argument) < 0) {
        Py_DECREF(argument);
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Remove an Argument from the Framework. If it does not exists already it does nothing.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_remove_argument(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", NULL};
    PyObject *argument;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &argument))
        return NULL;

    int contains = PySet_Contains(self->arguments, argument);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) { // If the argument does not exist it does nothing
        Py_RETURN_NONE;
    }

    // Check that the argument is not in attack relations
    contains = QBAFARelations_contains_argument(self->attack_relations, argument);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        PyErr_SetString(PyExc_ValueError,
                        "cannot remove argument that is contained in attack relations");
        return NULL;
    }

    // Check that the argument is not in support relations
    contains = QBAFARelations_contains_argument(self->support_relations, argument);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        PyErr_SetString(PyExc_ValueError,
                        "cannot remove argument that is contained in support relations");
        return NULL;
    }

    if (PySet_Discard(self->arguments, argument) < 0) {
        return NULL;
    }

    if (PyDict_DelItem(self->initial_weights, argument) < 0) {
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Add the Attack relation (attacker, attacked) to the Framework.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (attacker: QBAFArgument, attacked: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_add_attack_relation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"attacker", "attacked", NULL};
    PyObject *agent, *patient;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    contains = PySet_Contains(self->arguments, agent);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument attacker is not contained in the framework");
        return NULL;
    }

    contains = PySet_Contains(self->arguments, patient);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument attacked is not contained in the framework");
        return NULL;
    }

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->support_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        PyErr_SetString(PyExc_ValueError,
                        "attack relation already exists as support relation");
        return NULL;
    }

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->attack_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        Py_RETURN_NONE;
    }

    if (_QBAFARelations_add((QBAFARelationsObject*) self->attack_relations, agent, patient) < 0) {
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Add the Support relation (supporter, supported) to the Framework.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (supporter: QBAFArgument, supported: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_add_support_relation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"supporter", "supported", NULL};
    PyObject *agent, *patient;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    contains = PySet_Contains(self->arguments, agent);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument supporter is not contained in the framework");
        return NULL;
    }

    contains = PySet_Contains(self->arguments, patient);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument supported is not contained in the framework");
        return NULL;
    }

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->attack_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        PyErr_SetString(PyExc_ValueError,
                        "support relation already exists as attack relation");
        return NULL;
    }

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->support_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        Py_RETURN_NONE;
    }

    if (_QBAFARelations_add((QBAFARelationsObject*) self->support_relations, agent, patient) < 0) {
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Remove the Attack relation (attacker, attacked) from the Framework.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (attacker: QBAFArgument, attacked: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_remove_attack_relation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"attacker", "attacked", NULL};
    PyObject *agent, *patient;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->attack_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        Py_RETURN_NONE;
    }

    if (_QBAFARelations_remove((QBAFARelationsObject*) self->attack_relations, agent, patient) < 0) {
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Remove the Support relation (supporter, supported) from the Framework.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (supporter: QBAFArgument, supported: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_remove_support_relation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"supporter", "supported", NULL};
    PyObject *agent, *patient;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->support_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        Py_RETURN_NONE;
    }

    if (_QBAFARelations_remove((QBAFARelationsObject*) self->support_relations, agent, patient) < 0) {
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Return True if the argument is contained, False if it is not, NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyBool, NULL in case of error
 */
static PyObject *
QBAFramework_contains_argument(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", NULL};
    PyObject *argument;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &argument))
        return NULL;

    contains = PySet_Contains(self->arguments, argument);
    if (contains < 0) {
        return NULL;
    }

    if (contains) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}

/**
 * @brief Return True if the Attack relation is contained, False if it is not, NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (attacker: QBAFArgument, attacked: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyBool, NULL in case of error
 */
static PyObject *
QBAFramework_contains_attack_relation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"attacker", "attacked", NULL};
    PyObject *agent, *patient;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->attack_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }

    if (contains) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}

/**
 * @brief Return True if the Support relation is contained, False if it is not, NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (supporter: QBAFArgument, supported: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyBool, NULL in case of error
 */
static PyObject *
QBAFramework_contains_support_relation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"supporter", "supported", NULL};
    PyObject *agent, *patient;
    int contains;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &agent, &patient))
        return NULL;

    contains = _QBAFARelations_contains((QBAFARelationsObject*) self->support_relations, agent, patient);
    if (contains < 0) {
        return NULL;
    }

    if (contains) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}

/**
 * @brief Return a shallow copy of this instance.
 * New references are created for the copy, except for the QBAFArgument and QBAFARelations.
 * 
 * @param self instance of QBAFramework
 * @param Py_UNUSED 
 * @return PyObject* new instance of QBAFramework
 */
PyObject *
QBAFramework_copy(QBAFrameworkObject *self, PyObject *Py_UNUSED(ignored))
{
    PyObject *kwds = NULL;
    PyObject *args = NULL;
    PyObject *tmp;

    QBAFrameworkObject *copy = (QBAFrameworkObject*) QBAFramework_new(Py_TYPE(self), args, kwds);
    if (copy == NULL) {
        return NULL;
    }

    Py_DECREF(copy->arguments);
    copy->arguments = PySet_New(self->arguments);
    if (copy->arguments == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    Py_DECREF(copy->initial_weights);
    copy->initial_weights = PyDict_Copy(self->initial_weights);
    if (copy->initial_weights == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    Py_DECREF(copy->attack_relations);
    copy->attack_relations = QBAFARelations_copy((QBAFARelationsObject*)self->attack_relations, NULL);
    if (copy->attack_relations == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    Py_DECREF(copy->support_relations);
    copy->support_relations = QBAFARelations_copy((QBAFARelationsObject*)self->support_relations, NULL);
    if (copy->support_relations == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    if (PyDict_Check(copy->final_weights)) {
        Py_DECREF(copy->final_weights);
        copy->final_weights = PyDict_Copy(self->final_weights);
        if (copy->final_weights == NULL) {
            Py_DECREF(copy);
            return NULL;
        }
    }

    copy->modified = self->modified;
    
    return (PyObject*)copy;
}

/**
 * @brief Return a list with the arguments that are being attacked/supported by Argument argument (itself included)
 * that are in a cycle, NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param argument an instance of QBAFArgument
 * @param not_visited a PySet of QBAFArgument that have not been visited yet (this set is modified in this function)
 * @param visiting a PySet of QBAFArgument that are being visited in this function
 * @return PyObject* a new PyList of QBAFArgument objects that contain at least one cycle 
 */
static PyObject *
_QBAFramework_incycle_arguments(QBAFrameworkObject *self, PyObject *argument, PyObject *not_visited, PyObject *visiting)
{
    int contains = PySet_Contains(visiting, argument);
    if (contains < 0) {
        return NULL;
    }
    if (contains) { // If argument is being visited, do not visit it again but return it
        PyObject *list = PyList_New(1);
        if (list == NULL)
            return NULL;
        Py_INCREF(argument);
        PyList_SET_ITEM(list, 0, argument);
        return list;
    }

    if (PySet_Add(visiting, argument) < 0) {    // We add the argument to visiting
        return NULL;
    }
    PyObject *attack_patients, *support_patients;
    PyObject *patients, *result, *previous_result, *incycle;

    attack_patients = _QBAFARelations_patients((QBAFARelationsObject*)self->attack_relations, argument);
    if (attack_patients == NULL) {
        return NULL;
    }
    support_patients = _QBAFARelations_patients((QBAFARelationsObject*)self->support_relations, argument);
    if (support_patients == NULL) {
        Py_DECREF(attack_patients);
    }

    patients = PyList_Concat(attack_patients, support_patients);
    Py_DECREF(attack_patients);
    Py_DECREF(support_patients);
    if (patients == NULL) {
        return NULL;
    }

    result = PyList_New(0);
    if (result == NULL) {
        Py_DECREF(patients);
        return NULL;
    }

    PyObject *iterator = PyObject_GetIter(patients);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(patients);
        Py_DECREF(result);
        return NULL;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        contains = PySet_Contains(not_visited, item);
        if (contains < 0) {
            Py_DECREF(patients); Py_DECREF(result);
            Py_DECREF(iterator); Py_DECREF(item);
            return NULL;
        }

        if (contains) {
            previous_result = result;
            incycle = _QBAFramework_incycle_arguments(self, item, not_visited, visiting);
            if (incycle == NULL) {
                Py_DECREF(patients); Py_DECREF(previous_result);
                Py_DECREF(iterator); Py_DECREF(item);
                return NULL;
            }

            result = PyList_Concat(previous_result, incycle);
            Py_DECREF(previous_result);
            Py_DECREF(incycle);
            if (result == NULL) {
                Py_DECREF(patients);
                Py_DECREF(iterator); Py_DECREF(item);
                return NULL;
            }
        }

        Py_DECREF(item);
    }

    Py_DECREF(iterator);
    Py_DECREF(patients);

    if (PySet_Discard(not_visited, argument) < 0) { // We remove the argument from not visited
        Py_DECREF(result);
        return NULL;
    }

    if (PySet_Discard(visiting, argument) < 0) { // We remove the argument from visiting
        Py_DECREF(result);
        return NULL;
    }

    return result;
}

/**
 * @brief Return True if the relations of the Framework are acyclic, False if not,
 * -1 if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @return PyObject* 1 if acyclic, 0 if not acyclic, -1 if an error occurred
 */
static inline int
_QBAFramework_isacyclic(QBAFrameworkObject *self)
{
    PyObject *not_visited, *visiting;

    visiting = PySet_New(NULL); // New empty set
    if (visiting == NULL)
        return -1;

    not_visited = PySet_New(self->arguments);   // Copy of self->arguments
    if (not_visited == NULL) {
        Py_DECREF(visiting);
        return -1;
    }

    PyObject *argument, *incycle;

    while (PySet_GET_SIZE(not_visited) > 0) {
        argument = PySet_Pop(not_visited);  // New reference. Unchecked errors
        PySet_Add(not_visited, argument);   // We return the argument to the set

        incycle = _QBAFramework_incycle_arguments(self, argument, not_visited, visiting);
        if (incycle == NULL) {
            Py_DECREF(visiting); Py_DECREF(not_visited);
            return -1;
        }
        if (PyList_GET_SIZE(incycle) > 0) { // If detected cycle
            Py_DECREF(visiting); Py_DECREF(not_visited);
            Py_DECREF(incycle);
            return 0; // Return False
        }
        Py_DECREF(incycle);
    }

    Py_DECREF(visiting);
    Py_DECREF(not_visited);

    return 1;   // Return True
}

/**
 * @brief Return True if the relations of the Framework are acyclic, False if not,
 * NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param Py_UNUSED 
 * @return PyObject* new PyBool.
 */
static PyObject *
QBAFramework_isacyclic(QBAFrameworkObject *self, PyObject *Py_UNUSED(ignored))
{
    int isacyclic = _QBAFramework_isacyclic(self);
    if (isacyclic < 0) {
        return NULL;
    }

    if (isacyclic)
        Py_RETURN_TRUE;
        
    Py_RETURN_FALSE;
}

/**
 * @brief List of functions of the class QBAFramework
 * 
 */
static PyMethodDef QBAFramework_methods[] = {
    {"modify_initial_weight", (PyCFunctionWithKeywords) QBAFramework_modify_initial_weights, METH_VARARGS | METH_KEYWORDS,
    "Modify the initial weight of the Argument argument."
    },
    {"initial_weight", (PyCFunctionWithKeywords) QBAFramework_initial_weight, METH_VARARGS | METH_KEYWORDS,
    "Return the initial weight of the Argument argument."
    },
    {"add_argument", (PyCFunctionWithKeywords) QBAFramework_add_argument, METH_VARARGS | METH_KEYWORDS,
    "Add an Argument to the Framework. If it exists already it does nothing."
    },
    {"remove_argument", (PyCFunctionWithKeywords) QBAFramework_remove_argument, METH_VARARGS | METH_KEYWORDS,
    "Remove the Argument argument from the Framework. If it does not exist it does nothing."
    },
    {"add_attack_relation", (PyCFunctionWithKeywords) QBAFramework_add_attack_relation, METH_VARARGS | METH_KEYWORDS,
    "Add the Attack relation (attacker, attacked) to the Framework."
    },
    {"add_support_relation", (PyCFunctionWithKeywords) QBAFramework_add_support_relation, METH_VARARGS | METH_KEYWORDS,
    "Add the Support relation (supporter, supported) to the Framework."
    },
    {"remove_attack_relation", (PyCFunctionWithKeywords) QBAFramework_remove_attack_relation, METH_VARARGS | METH_KEYWORDS,
    "Remove the Attack relation (attacker, attacked) from the Framework."
    },
    {"remove_support_relation", (PyCFunctionWithKeywords) QBAFramework_remove_support_relation, METH_VARARGS | METH_KEYWORDS,
    "Remove the Support relation (supporter, supported) from the Framework."
    },
    {"contains_argument", (PyCFunctionWithKeywords) QBAFramework_contains_argument, METH_VARARGS | METH_KEYWORDS,
    "Return whether or not the Framework contains the Argument argument."
    },
    {"contains_attack_relation", (PyCFunctionWithKeywords) QBAFramework_contains_attack_relation, METH_VARARGS | METH_KEYWORDS,
    "Return whether or not the Attack relation (attacker, attacked) is contained in the Framework."
    },
    {"contains_support_relation", (PyCFunctionWithKeywords) QBAFramework_contains_support_relation, METH_VARARGS | METH_KEYWORDS,
    "Return whether or not the Support relation (supporter, supported) is contained in the Framework."
    },
    {"__copy__", (PyCFunction) QBAFramework_copy, METH_NOARGS,
    "Return shallow a copy of the instance."
    },
    {"copy", (PyCFunction) QBAFramework_copy, METH_NOARGS,
    "Return shallow a copy of the instance."
    },
    {"isacyclic", (PyCFunction) QBAFramework_isacyclic, METH_NOARGS,
    "Return whether or not the relations of the Framework are acyclic."
    },
    {NULL}  /* Sentinel */
};

/**
 * @brief Python definition for the class QBAFramework
 * 
 */
static PyTypeObject QBAFrameworkType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "qbaf.QBAFramework",
    .tp_doc = PyDoc_STR("QBAFramework objects"),
    .tp_basicsize = sizeof(QBAFrameworkObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_new = QBAFramework_new,
    .tp_init = (initproc) QBAFramework_init,
    .tp_dealloc = (destructor) QBAFramework_dealloc,
    .tp_traverse = (traverseproc) QBAFramework_traverse,
    .tp_clear = (inquiry) QBAFramework_clear,
    .tp_members = QBAFramework_members,
    .tp_methods = QBAFramework_methods,
    .tp_getset = QBAFramework_getsetters,
};

/**
 * @brief Get the QBAFrameworkType object created above that defines the class QBAFramework
 * 
 * @return PyTypeObject* a pointer to the QBAFramework class definition
 */
PyTypeObject *get_QBAFrameworkType() {
    return &QBAFrameworkType;
}