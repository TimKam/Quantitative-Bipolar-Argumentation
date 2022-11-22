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
    PyObject *arguments;            /* a set of QBAFArgument */
    PyObject *initial_weights;      /* a dictionary (argument: QBAFArgument, initial_weight: double) */
    PyObject *attack_relations;     /* an instance of QBAFARelations */
    PyObject *support_relations;    /* an instance of QBAFARelations */
    PyObject *final_weights;        /* a dictionary (argument: QBAFArgument, final_weight: double) */
    int       modified;             /* 0 if the framework has not been modified after calculating the final weights. Otherwise, 1 */
    int       disjoint_relations;   /* 1 if the attack/support relations must be disjoint, 0 if they do not have to */
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
        self->disjoint_relations = TRUE;
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
    static char *kwlist[] = {"arguments", "initial_weights", "attack_relations", "support_relations",
                            "disjoint_relations", "semantics", "aggregation_function", "influence_function",
                            "min_weight", "max_weight", NULL};
    PyObject *arguments, *initial_weights, *attack_relations, *support_relations, *tmp;
    int disjoint_relations = TRUE;
    char *semantics = NULL; // (e.g. "naive") If None it will be NULL, otherwise it is a pointer to char that is only accesible in this function.
    PyObject *aggregation_function = NULL, *influence_function = NULL;
    double min_weight = DBL_MIN, max_weight = DBL_MAX;
    // TODO: Implement semantics, aggregation function and influence function, min_weight and max_weight

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO|pzOOdd", kwlist,
                                     &arguments, &initial_weights, &attack_relations, &support_relations,
                                     &disjoint_relations, &semantics, &aggregation_function, &influence_function,
                                     &min_weight, &max_weight))
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
    int contained = QBAFARelations_ArgsContained((QBAFARelationsObject*)self->attack_relations, self->arguments);
    if (contained < 0) {
        return -1;
    }
    if (!contained)  {
        PyErr_SetString(PyExc_ValueError, "all relation components of attack_relations must be in arguments");
        return -1;
    }

    // Check that the arguments of support relations are in the arguments
    contained = QBAFARelations_ArgsContained((QBAFARelationsObject*)self->support_relations, self->arguments);
    if (contained < 0) {
        return -1;
    }
    if (!contained)  {
        PyErr_SetString(PyExc_ValueError, "all relation components of support_relations must be in arguments");
        return -1;
    }

    
    self->disjoint_relations = disjoint_relations;

    if (self->disjoint_relations) {
        // Check attack and support relations are disjoint
        int disjoint = _QBAFARelations_isDisjoint((QBAFARelationsObject*)self->attack_relations, (QBAFARelationsObject*)self->support_relations);
        if (disjoint < 0) {
            return -1;
        }
        if (!disjoint) {
            PyErr_SetString(PyExc_ValueError, "attack_relations and support_relations must be disjoint");
            return -1;
        }
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
 * @brief Getter of the attribute disjoint relations.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* a new PyBool
 */
static PyObject *
QBAFramework_getdisjoint_relations(QBAFrameworkObject *self, void *closure)
{
    if (self->disjoint_relations)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

/**
 * @brief Setter of the attribute disjoint_relations.
 * 
 * @param self the QBAFramework object
 * @param value PyBool value for disjoint_relations
 * @param closure 
 * @return int 0 if it was executed with no errors. Otherwise, -1.
 */
static int
QBAFramework_setdisjoint_relations(QBAFrameworkObject *self, PyObject *value, void *closure)
{
    if (!PyBool_Check(value)) {
        PyErr_SetString(PyExc_TypeError,
                        "disjoint_relations must be of type boolean");
        return -1;
    }
    
    int disjoint_relations = PyObject_IsTrue(value);

    if (self->disjoint_relations == disjoint_relations) {
        return 0;
    }

    self->disjoint_relations = disjoint_relations;

    if (self->disjoint_relations) {
        // Check attack and support relations are disjoint
        int disjoint = _QBAFARelations_isDisjoint((QBAFARelationsObject*)self->attack_relations, (QBAFARelationsObject*)self->support_relations);
        if (disjoint < 0) {
            return -1;
        }
        if (!disjoint) {
            PyErr_SetString(PyExc_ValueError, "attack_relations and support_relations are not disjoint");
            return -1;
        }
    }

    return 0;
}

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

    int contains = PyDict_Contains(self->initial_weights, argument);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument must be contained in the QBAFramework");
        return NULL;
    }

    PyObject *initial_weight = PyDict_GetItem(self->initial_weights, argument);
    Py_INCREF(initial_weight);
    return initial_weight;
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
    contains = QBAFARelations_contains_argument((QBAFARelationsObject*)self->attack_relations, argument);
    if (contains < 0) {
        return NULL;
    }
    if (contains) {
        PyErr_SetString(PyExc_ValueError,
                        "cannot remove argument that is contained in attack relations");
        return NULL;
    }

    // Check that the argument is not in support relations
    contains = QBAFARelations_contains_argument((QBAFARelationsObject*)self->support_relations, argument);
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

    if (self->disjoint_relations) {
        contains = _QBAFARelations_contains((QBAFARelationsObject*) self->support_relations, agent, patient);
        if (contains < 0) {
            return NULL;
        }
        if (contains) {
            PyErr_SetString(PyExc_ValueError,
                            "attack relation already exists as support relation");
            return NULL;
        }
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

    if (self->disjoint_relations) {
        contains = _QBAFARelations_contains((QBAFARelationsObject*) self->attack_relations, agent, patient);
        if (contains < 0) {
            return NULL;
        }
        if (contains) {
            PyErr_SetString(PyExc_ValueError,
                            "support relation already exists as attack relation");
            return NULL;
        }
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
    copy->disjoint_relations = self->disjoint_relations;
    
    return (PyObject*)copy;
}

/**
 * @brief Return a list with the arguments that are being attacked/supported by Argument argument (itself included)
 * that are in a cycle, NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param argument an instance of QBAFArgument
 * @param not_visited a PySet of QBAFArgument that have not been visited yet (this set is modified in this function)
 * @param visiting a PySet of QBAFArgument that are being visited within this function
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

    Py_INCREF(argument);
    if (PySet_Add(visiting, argument) < 0) {    // We add the argument to visiting
        Py_DECREF(argument);
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
 * @brief Return the final weight of a specific argument, -1.0 if an error occurred.
 * This function calls itself recursively. So, it only works with acyclic arguments.
 * It stores all the calculated final weights in self.__final_weights.
 * 
 * @param self an instance of QBAFramework
 * @param argument the QBAFArgument
 * @return PyObject* borrowed PyFloat reference, -1.0 if an error has occurrred
 */
static double
_QBAFramework_calculate_final_weight(QBAFrameworkObject *self, PyObject *argument)
{
    int contains = PyDict_Contains(self->final_weights, argument);
    if (contains < 0) { // TODO: Remove error checks in this function (not needed since this is only used internally)
        return -1.0;
    }
    if (contains) {
        return PyFloat_AsDouble(PyDict_GetItem(self->final_weights, argument));
    }

    double final_weight = PyFloat_AsDouble(PyDict_GetItem(self->initial_weights, argument));

    PyObject *attackers = _QBAFARelations_agents((QBAFARelationsObject*)self->attack_relations, argument);
    PyObject *iterator = PyObject_GetIter(attackers);
    PyObject *item;

    if (iterator == NULL) {
        Py_XDECREF(attackers);
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        final_weight -= _QBAFramework_calculate_final_weight(self, item);
        Py_DECREF(item);
    }

    Py_DECREF(attackers);
    Py_DECREF(iterator);

    PyObject *supporters = _QBAFARelations_agents((QBAFARelationsObject*)self->support_relations, argument);
    iterator = PyObject_GetIter(supporters);

    if (iterator == NULL) {
        Py_XDECREF(supporters);
        return -1.0;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        final_weight += _QBAFramework_calculate_final_weight(self, item);
        Py_DECREF(item);
    }

    Py_DECREF(supporters);
    Py_DECREF(iterator);

    // final_weights[argument] = final_weight
    PyObject *pyfinal_weight = PyFloat_FromDouble(final_weight);    // New reference
    Py_INCREF(argument);
    if (PyDict_SetItem(self->final_weights, argument, pyfinal_weight) < 0) {
        Py_DECREF(argument); Py_XDECREF(pyfinal_weight);
        return -1.0;
    }
    Py_XDECREF(pyfinal_weight);

    return final_weight;
}

/**
 * @brief Calculate the final weights of all the arguments of the Framework.
 * It stores all the calculated final weights in self.__final_weights.
 * 
 * @param self the QBAFramework
 * @return int 0 if succesful, -1 if an error occurred
 */
static int
_QBAFRamework_calculate_final_weights(QBAFrameworkObject *self)
{
    int isacyclic = _QBAFramework_isacyclic(self);
    if (isacyclic < 0) {
        return -1;
    }
    if (!isacyclic) {
        PyErr_SetString(PyExc_NotImplementedError,
                        "calculate final weights of non-acyclic framework not implemented");
        return -1;
    }

    Py_CLEAR(self->final_weights);
    self->final_weights = PyDict_New();

    PyObject *iterator = PyObject_GetIter(self->arguments);
    PyObject *item;
    if (iterator == NULL) {
        return -1;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        if (_QBAFramework_calculate_final_weight(self, item) == -1.0 && PyErr_Occurred()) {
            Py_DECREF(item); Py_DECREF(iterator);
            return -1;
        }
        Py_DECREF(item);
    }

    Py_DECREF(iterator);

    return 0;
}

/**
 * @brief Return the final weights of arguments of the Framework, NULL if an error occurred.
 * If the framework has been modified from the last time they were calculated
 * they are calculated again. Otherwise, it returns the already calculated final weights.
 * 
 * @param self the QBAFramework
 * @param closure 
 * @return PyObject* a new PyDict, NULL if an error occurred
 */
static PyObject *
QBAFramework_getfinal_weights(QBAFrameworkObject *self, void *closure)
{
    if (self->modified) {   // Calculate final weights if the framework has been modified
        if (_QBAFRamework_calculate_final_weights(self) < 0) {
            return NULL;
        }
        self->modified = FALSE;
    }

    return PyDict_Copy(self->final_weights);
}

/**
 * @brief Return the final weight of the Argument argument, NULL in case of error.
 * 
 * @param self an instance of QBAFramework
 * @param argument the QBAFArgument
 * @return PyObject* borrowed PyFloat
 */
static PyObject *
_QBAFramework_final_weight(QBAFrameworkObject *self, PyObject *argument)
{
    if (self->modified) {   // Calculate final weights if the framework has been modified
        if (_QBAFRamework_calculate_final_weights(self) < 0) {
            return NULL;
        }
        self->modified = FALSE;
    }

    int contains = PyDict_Contains(self->final_weights, argument);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument must be contained in the QBAFramework");
        return NULL;
    }

    return PyDict_GetItem(self->final_weights, argument);
}

/**
 * @brief Return the final weight of the Argument argument, NULL in case of error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyFloat
 */
static PyObject *
QBAFramework_final_weight(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", NULL};
    PyObject *argument;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &argument))
        return NULL;

    PyObject *final_weight = _QBAFramework_final_weight(self, argument); // Borrowed reference
    Py_XINCREF(final_weight);
    return final_weight;
}

/**
 * @brief Return True if a pair of arguments are strength consistent between two frameworks,
 * -1 if an error has occurred.
 * 
 * @param self instance of QBAFramework
 * @param other another instance of QBAFramework
 * @param arg1 an instance of QBAFArgument (must be contained in both frameworks)
 * @param arg2 an instance of QBAFArgument (must be contained in both frameworks)
 * @return int 1 if strength consistent, 0 if not strength consistent, -1 if an error occurred
 */
static inline int
_QBAFramework_are_strength_consistent(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *arg1, PyObject *arg2)
{
    if (self->modified) {   // Calculate final weights if the framework has been modified
        if (_QBAFRamework_calculate_final_weights(self) < 0) {
            return -1;
        }
        self->modified = FALSE;
    }
    if (other->modified) {   // Calculate final weights if the framework has been modified
        if (_QBAFRamework_calculate_final_weights(other) < 0) {
            return -1;
        }
        other->modified = FALSE;
    }

    // Check that the arguments are contained in both frameworks
    int contains = PyDict_Contains(self->final_weights, arg1);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg1 must be an argument of this QBAFramework");
        return -1;
    }
    contains = PyDict_Contains(self->final_weights, arg2);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg2 must be an argument of this QBAFramework");
        return -1;
    }
    contains = PyDict_Contains(other->final_weights, arg1);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg1 must be an argument of the QBAFramework other");
        return -1;
    }
    contains = PyDict_Contains(other->final_weights, arg2);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg2 must be an argument of the QBAFramework other");
        return -1;
    }

    double self_final_weight_arg1 = PyFloat_AS_DOUBLE(PyDict_GetItem(self->final_weights, arg1));
    double self_final_weight_arg2 = PyFloat_AS_DOUBLE(PyDict_GetItem(self->final_weights, arg2));
    double other_final_weight_arg1 = PyFloat_AS_DOUBLE(PyDict_GetItem(other->final_weights, arg1));
    double other_final_weight_arg2 = PyFloat_AS_DOUBLE(PyDict_GetItem(other->final_weights, arg2));

    if (self_final_weight_arg1 < self_final_weight_arg2)
        return other_final_weight_arg1 < other_final_weight_arg2;
    if (self_final_weight_arg1 > self_final_weight_arg2)
        return other_final_weight_arg1 > other_final_weight_arg2;
    return other_final_weight_arg1 == other_final_weight_arg2;
}

/**
 * @brief Return True if a pair of arguments are strength consistent between two frameworks,
 * False if not, NULL if an error has occurred.
 * 
 * @param self instance of QBAFramework
 * @param args a tuple with arguments (other: QBAFramework, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds name of the arguments args
 * @return PyObject* new PyBool, NULL if an error occurred
 */

static PyObject *
QBAFramework_are_strength_consistent(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", "arg1", "arg2", NULL};
    PyObject *other, *arg1, *arg2;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|", kwlist,
                                     &other, &arg1, &arg2))
        return NULL;

    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "other must be an instance of QBAFramework");
        return NULL;
    }
    
    int strength_consistent = _QBAFramework_are_strength_consistent(self, (QBAFrameworkObject*)other, arg1, arg2);
    if (strength_consistent < 0) {
        return NULL;
    }
    if (strength_consistent)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

/**
 * @brief Return the reversal framework of self to other w.r.t. set, NULL if an error is encountered.
 * The Set set must be a subset of self->arguments UNION other->arguments.
 * 
 * @param self the QBAFramework that is going to be reversed
 * @param other the QBAFramework that it is going to be reversed into.
 * @param set the set of QBAFArgument that are going to be reversed
 * @return PyObject* new QBAFramework, NULL if an error occurred
 */
static PyObject *
_QBAFramework_reversal(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject* set)
{
    PyObject *arguments_union = PySet_Union(self->arguments, other->arguments);
    if (arguments_union == NULL) {
        return NULL;
    }

    // Check that set.issubset(self.arguments.union(other.arguments)) 
    int issubset = PySet_IsSubset(set, arguments_union);
    if (issubset < 0) {
        Py_DECREF(arguments_union);
        return NULL;
    }
    if (!issubset) {
        Py_DECREF(arguments_union);
        PyErr_SetString(PyExc_ValueError,
                        "argument set must be a subset of the union of the arguments of both frameworks");
        return NULL;
    }
    Py_DECREF(arguments_union);

    // Copy this framework
    QBAFrameworkObject *reversal = (QBAFrameworkObject*)QBAFramework_copy(self, NULL);
    if (reversal == NULL) {
        return NULL;
    }

    // Modify arguments
    PyObject *self_arguments_union_set = PySet_Union(self->arguments, set);
    if (self_arguments_union_set == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    PyObject *set_difference_other_arguments = PySet_Difference(set, other->arguments);
    if (set_difference_other_arguments == NULL) {
        Py_DECREF(reversal);
        Py_DECREF(self_arguments_union_set);
        return NULL;
    }
    Py_CLEAR(reversal->arguments);
    reversal->arguments = PySet_Difference(self_arguments_union_set, set_difference_other_arguments);
    Py_DECREF(self_arguments_union_set);
    Py_DECREF(set_difference_other_arguments);
    if (reversal->arguments == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }

    // Modify attack relations
    PyObject *set_iterator = PyObject_GetIter(set);
    PyObject *arg;  // item
    if (set_iterator == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    PyObject *iterator, *attacked, *patients;
    while ((arg = PyIter_Next(set_iterator))) {    // PyIter_Next returns a new reference
        // for attacked in self.__attack_relations.patients(arg): att.remove_relation(arg, attacked)
        patients = _QBAFARelations_patients((QBAFARelationsObject*)self->attack_relations, arg); // new reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients);
        if (iterator == NULL) {
            Py_DECREF(patients); Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        while ((attacked = PyIter_Next(iterator))) {
            if (_QBAFARelations_remove((QBAFARelationsObject*)reversal->attack_relations, arg, attacked) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(attacked); Py_DECREF(iterator); Py_DECREF(patients);
                return NULL;
            }
            Py_DECREF(attacked);
        }
        Py_DECREF(patients);
        Py_DECREF(iterator);

        // for attacked in other.__attack_relations.patients(arg): att.add_relation(arg, attacked)
        patients = _QBAFARelations_patients((QBAFARelationsObject*)other->attack_relations, arg); // new reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients);
        if (iterator == NULL) {
            Py_DECREF(patients); Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        while ((attacked = PyIter_Next(iterator))) {
            if (_QBAFARelations_add((QBAFARelationsObject*)reversal->attack_relations, arg, attacked) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(attacked); Py_DECREF(iterator); Py_DECREF(patients);
                return NULL;
            }
            Py_DECREF(attacked);
        }
        Py_DECREF(patients);
        Py_DECREF(iterator);
        
        // Decref arg
        Py_DECREF(arg);
    }
    Py_DECREF(set_iterator);

    // Modify support relations
    set_iterator = PyObject_GetIter(set);
    if (set_iterator == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    PyObject *supported;
    while ((arg = PyIter_Next(set_iterator))) {    // PyIter_Next returns a new reference
        // for supported in self.__support_relations.patients(arg): att.remove_relation(arg, supported)
        patients = _QBAFARelations_patients((QBAFARelationsObject*)self->support_relations, arg); // new reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients);
        if (iterator == NULL) {
            Py_DECREF(patients); Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        while ((supported = PyIter_Next(iterator))) {
            if (_QBAFARelations_remove((QBAFARelationsObject*)reversal->support_relations, arg, supported) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(supported); Py_DECREF(iterator); Py_DECREF(patients);
                return NULL;
            }
            Py_DECREF(supported);
        }
        Py_DECREF(patients);
        Py_DECREF(iterator);

        // for supported in other.__support_relations.patients(arg): att.add_relation(arg, supported)
        patients = _QBAFARelations_patients((QBAFARelationsObject*)other->support_relations, arg); // new reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients);
        if (iterator == NULL) {
            Py_DECREF(patients); Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        while ((supported = PyIter_Next(iterator))) {
            if (_QBAFARelations_add((QBAFARelationsObject*)reversal->support_relations, arg, supported) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(supported); Py_DECREF(iterator); Py_DECREF(patients);
                return NULL;
            }
            Py_DECREF(supported);
        }
        Py_DECREF(patients);
        Py_DECREF(iterator);
        
        // Decref arg
        Py_DECREF(arg);
    }
    Py_DECREF(set_iterator);

    // Modify initial weights
    Py_CLEAR(reversal->initial_weights);
    reversal->initial_weights = PyDict_New();
    if (reversal->initial_weights == NULL) {
        return NULL;
    }
    iterator = PyObject_GetIter(reversal->arguments);
    if (iterator == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    PyObject *other_arguments_intersection_set = PySet_Intersection(other->arguments, set);
    if (other_arguments_intersection_set == NULL) {
        Py_DECREF(reversal); Py_DECREF(iterator);
        return NULL;
    }
    PyObject *initial_weight;
    while ((arg = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(other_arguments_intersection_set, arg);
        if (contains < 0) {
            Py_DECREF(reversal); Py_DECREF(iterator);
            Py_DECREF(arg); Py_DECREF(other_arguments_intersection_set);
            return NULL;
        }
        if (contains) {
            initial_weight = PyDict_GetItem(other->initial_weights, arg);
        } else {
            initial_weight = PyDict_GetItem(self->initial_weights, arg);
        }
        if (initial_weight == NULL) {
            Py_DECREF(reversal); Py_DECREF(iterator);
            Py_DECREF(arg); Py_DECREF(other_arguments_intersection_set);
            return NULL;
        }
        if (PyDict_SetItem(reversal->initial_weights, arg, initial_weight) < 0) {
            Py_DECREF(reversal); Py_DECREF(iterator);
            Py_DECREF(arg); Py_DECREF(other_arguments_intersection_set);
            return NULL;
        }
    }
    Py_DECREF(iterator);
    Py_DECREF(other_arguments_intersection_set);

    // Remove calculated final weights
    Py_CLEAR(reversal->final_weights);
    reversal->final_weights = PyDict_New();
    reversal->modified = TRUE;

    // Return
    return (PyObject*) reversal;
}

/**
 * @brief Return the reversal framework of self to other w.r.t. set, NULL if an error is encountered.
 * The Set set must be a subset of self->arguments UNION other->arguments.
 * 
 * @param self the QBAFramework that is going to be reversed
 * @param args the argument values (other: QBAFramework, set: PySet of QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new QBAFramework, NULL if an error occurred
 */
static PyObject *
QBAFramework_reversal(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", "set", NULL};
    PyObject *other, *set;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &other, &set))
        return NULL;
    
    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "other must be an instance of QBAFramework");
        return NULL;
    }

    if (PySet_Check(set)) {
        Py_INCREF(set);
    } else if (PyList_Check(set)) {
        set = PySet_New(set); // new reference
        if (set == NULL) {
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "argument set must be an instance of set or list");
        return NULL;
    }

    PyObject *reversal = _QBAFramework_reversal(self, (QBAFrameworkObject*)other, set);

    Py_DECREF(set);

    return reversal;
}

/**
 * @brief Return True if a set of arguments set is Sufficient Strength Inconsistency (SSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * -1 if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param set a PySet of QBAFArgument
 * @param arg1 a QBAFArgument
 * @param arg2 a QBAFArgument
 * @return int 1 if it is a SSI Explanation, 0 if it is not, -1 if an error has occurred
 */
static int
_QBAFramework_isSSIExplanation(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *set, PyObject *arg1, PyObject *arg2)
{
    int are_strength_consistent = _QBAFramework_are_strength_consistent(self, other, arg1, arg2);
    if (are_strength_consistent < 0) {
        return -1;
    }
    if (are_strength_consistent) {
        return PySet_GET_SIZE(set) == 0;
    }

    PyObject *self_arguments_union_other_arguments = PySet_Union(self->arguments, other->arguments);
    if (self_arguments_union_other_arguments == NULL) {
        return -1;
    }
    PyObject *self_arguments_union_other_arguments_difference_set = PySet_Difference(self_arguments_union_other_arguments, set);
    Py_DECREF(self_arguments_union_other_arguments);
    if (self_arguments_union_other_arguments_difference_set == NULL) {
        return -1;
    }

    PyObject *reversal = _QBAFramework_reversal(self, other, self_arguments_union_other_arguments_difference_set);
    Py_DECREF(self_arguments_union_other_arguments_difference_set);
    if (reversal == NULL) {
        return -1;
    }

    are_strength_consistent = _QBAFramework_are_strength_consistent(other, (QBAFrameworkObject*)reversal, arg1, arg2);
    Py_DECREF(reversal);
    if (are_strength_consistent < 0) {
        return -1;
    }
    return !are_strength_consistent;
}

/**
 * @brief Return True if a set of arguments set is Counterfactual Strength Inconsistency (SSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * -1 if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param set a PySet of QBAFArgument
 * @param arg1 a QBAFArgument
 * @param arg2 a QBAFArgument
 * @return int 1 if it is a CSI Explanation, 0 if it is not, -1 if an error has occurred
 */
static int
_QBAFramework_isCSIExplanation(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *set, PyObject *arg1, PyObject *arg2)
{
    PyObject *reversal = _QBAFramework_reversal(self, other, set);
    if (reversal == NULL) {
        return -1;
    }

    int are_strength_consistent = _QBAFramework_are_strength_consistent(other, (QBAFrameworkObject*)reversal, arg1, arg2);
    Py_DECREF(reversal);
    if (are_strength_consistent < 0) {
        return -1;
    }
    if (!are_strength_consistent) {
        return FALSE;
    }

    return _QBAFramework_isSSIExplanation(self, other, set, arg1, arg2);
}

/**
 * @brief Return True if a set of arguments set is Sufficient Strength Inconsistency (SSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * NULL if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (other: QBAFramework, set: PySet of QBAFArgument, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds the argument names
 * @return int PyTrue if it is a SSI Explanation, PyFalse if it is not, NULL if an error has occurred
 */
static PyObject *
QBAFramework_isSSIExplanation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", "set", "arg1", "arg2", NULL};
    PyObject *other, *set, *arg1, *arg2;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO|", kwlist,
                                     &other, &set, &arg1, &arg2))
        return NULL;
    
    // Check other is a QBAFramework
    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "other must be an instance of QBAFramework");
        return NULL;
    }

    // Check set is a PySet or a PyList. If a PyList, create a Pyset.
    if (PySet_Check(set)) {
        Py_INCREF(set);
    } else if (PyList_Check(set)) {
        set = PySet_New(set); // new reference
        if (set == NULL) {
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "argument set must be an instance of set or list");
        return NULL;
    }

    // Check all items of PySet are in self->arguments UNION other->arguments
    PyObject *self_arguments_union_other_arguments = PySet_Union(self->arguments, ((QBAFrameworkObject*)other)->arguments);
    if (self_arguments_union_other_arguments == NULL) {
        Py_DECREF(set);
        return NULL;
    }
    PyObject *iterator = PyObject_GetIter(set);
    PyObject *item;
    int contains;
    if (iterator == NULL) {
        Py_DECREF(set); Py_DECREF(self_arguments_union_other_arguments);
        return NULL;
    }
    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        contains = PySet_Contains(self_arguments_union_other_arguments, item);
        if (contains < 0) {
            Py_DECREF(set); Py_DECREF(self_arguments_union_other_arguments);
            Py_DECREF(item); Py_DECREF(iterator);
            return NULL;
        }
        if (!contains) {
            Py_DECREF(set); Py_DECREF(self_arguments_union_other_arguments);
            Py_DECREF(item); Py_DECREF(iterator);
            PyErr_SetString(PyExc_ValueError, "every item of set must be contained in self.arguments UNION other.arguments");
            return NULL;
        }
        Py_DECREF(item);
    }
    Py_DECREF(iterator);
    Py_DECREF(self_arguments_union_other_arguments);

    // Check arg1 is in self->arguments intersection other->arguments
    PyObject *self_arguments_intersection_other_arguments = PySet_Intersection(self->arguments, ((QBAFrameworkObject*)other)->arguments);
    contains = PySet_Contains(self_arguments_intersection_other_arguments, arg1);
    if (contains < 0) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        return NULL;
    }
    if (!contains) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        PyErr_SetString(PyExc_ValueError, "arg1 must be contained in self.arguments INTERSECTION other.arguments");
        return NULL;
    }
    contains = PySet_Contains(self_arguments_intersection_other_arguments, arg2);
    if (contains < 0) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        return NULL;
    }
    if (!contains) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        PyErr_SetString(PyExc_ValueError, "arg2 must be contained in self.arguments INTERSECTION other.arguments");
        return NULL;
    }
    Py_DECREF(self_arguments_intersection_other_arguments);

    int isSSIExplanation = _QBAFramework_isSSIExplanation(self, (QBAFrameworkObject*)other, set, arg1, arg2);
    Py_DECREF(set);
    if (isSSIExplanation < 0) {
        return NULL;
    }

    if (isSSIExplanation)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

/**
 * @brief Return True if a set of arguments set is Counterfactual Strength Inconsistency (SSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * NULL if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (other: QBAFramework, set: PySet of QBAFArgument, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds the argument names
 * @return int PyTrue if it is a SSI Explanation, PyFalse if it is not, NULL if an error has occurred
 */
static PyObject *
QBAFramework_isCSIExplanation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", "set", "arg1", "arg2", NULL};
    PyObject *other, *set, *arg1, *arg2;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO|", kwlist,
                                     &other, &set, &arg1, &arg2))
        return NULL;
    
    // Check other is a QBAFramework
    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "other must be an instance of QBAFramework");
        return NULL;
    }

    // Check set is a PySet or a PyList. If a PyList, create a Pyset.
    if (PySet_Check(set)) {
        Py_INCREF(set);
    } else if (PyList_Check(set)) {
        set = PySet_New(set); // new reference
        if (set == NULL) {
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "argument set must be an instance of set or list");
        return NULL;
    }

    // Check all items of PySet are in self->arguments UNION other->arguments
    PyObject *self_arguments_union_other_arguments = PySet_Union(self->arguments, ((QBAFrameworkObject*)other)->arguments);
    if (self_arguments_union_other_arguments == NULL) {
        Py_DECREF(set);
        return NULL;
    }
    PyObject *iterator = PyObject_GetIter(set);
    PyObject *item;
    int contains;
    if (iterator == NULL) {
        Py_DECREF(set); Py_DECREF(self_arguments_union_other_arguments);
        return NULL;
    }
    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        contains = PySet_Contains(self_arguments_union_other_arguments, item);
        if (contains < 0) {
            Py_DECREF(set); Py_DECREF(self_arguments_union_other_arguments);
            Py_DECREF(item); Py_DECREF(iterator);
            return NULL;
        }
        if (!contains) {
            Py_DECREF(set); Py_DECREF(self_arguments_union_other_arguments);
            Py_DECREF(item); Py_DECREF(iterator);
            PyErr_SetString(PyExc_ValueError, "every item of set must be contained in self.arguments UNION other.arguments");
            return NULL;
        }
        Py_DECREF(item);
    }
    Py_DECREF(iterator);
    Py_DECREF(self_arguments_union_other_arguments);

    // Check arg1 is in self->arguments intersection other->arguments
    PyObject *self_arguments_intersection_other_arguments = PySet_Intersection(self->arguments, ((QBAFrameworkObject*)other)->arguments);
    contains = PySet_Contains(self_arguments_intersection_other_arguments, arg1);
    if (contains < 0) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        return NULL;
    }
    if (!contains) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        PyErr_SetString(PyExc_ValueError, "arg1 must be contained in self.arguments INTERSECTION other.arguments");
        return NULL;
    }
    contains = PySet_Contains(self_arguments_intersection_other_arguments, arg2);
    if (contains < 0) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        return NULL;
    }
    if (!contains) {
        Py_DECREF(set); Py_DECREF(self_arguments_intersection_other_arguments);
        PyErr_SetString(PyExc_ValueError, "arg2 must be contained in self.arguments INTERSECTION other.arguments");
        return NULL;
    }
    Py_DECREF(self_arguments_intersection_other_arguments);

    int isCSIExplanation = _QBAFramework_isCSIExplanation(self, (QBAFrameworkObject*)other, set, arg1, arg2);
    Py_DECREF(set);
    if (isCSIExplanation < 0) {
        return NULL;
    }

    if (isCSIExplanation)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

/**
 * @brief Return a list with the arguments that are attacking/supporting Argument argument directly or indirectly,
 * NULL if an error has occurred.
 * 
 * @param self an instance of QBAFramework
 * @param argument an instance of QBAFArgument
 * @param not_visited a PySet of QBAFArgument that have not been visited yet (this set is modified in this function)
 * @param visiting a PySet of QBAFArgument that are being visited within this function
 * @return PyObject* a new PyList of QBAFArgument objects 
 */
static PyObject *
_QBAFramework_influential_arguments(QBAFrameworkObject *self, PyObject *argument, PyObject *not_visited, PyObject *visiting)
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

    Py_INCREF(argument);
    if (PySet_Add(visiting, argument) < 0) {    // We add the argument to visiting
        Py_DECREF(argument);
        return NULL;
    }
    PyObject *attack_agents, *support_agents;
    PyObject *agents, *result, *previous_result, *influential_arguments;

    attack_agents = _QBAFARelations_agents((QBAFARelationsObject*)self->attack_relations, argument);
    if (attack_agents == NULL) {
        return NULL;
    }
    support_agents = _QBAFARelations_agents((QBAFARelationsObject*)self->support_relations, argument);
    if (support_agents == NULL) {
        Py_DECREF(attack_agents);
    }

    agents = PyList_Concat(attack_agents, support_agents);
    Py_DECREF(attack_agents);
    Py_DECREF(support_agents);
    if (agents == NULL) {
        return NULL;
    }

    // result = [argument]
    result = PyList_New(1);
    if (result == NULL) {
        Py_DECREF(agents);
        return NULL;
    }
    Py_INCREF(argument);
    PyList_SET_ITEM(result, 0, argument);

    PyObject *iterator = PyObject_GetIter(agents);
    PyObject *item;

    if (iterator == NULL) {
        Py_DECREF(agents);
        Py_DECREF(result);
        return NULL;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        contains = PySet_Contains(not_visited, item);
        if (contains < 0) {
            Py_DECREF(agents); Py_DECREF(result);
            Py_DECREF(iterator); Py_DECREF(item);
            return NULL;
        }

        if (contains) {
            previous_result = result;
            influential_arguments = _QBAFramework_influential_arguments(self, item, not_visited, visiting);
            if (influential_arguments == NULL) {
                Py_DECREF(agents); Py_DECREF(previous_result);
                Py_DECREF(iterator); Py_DECREF(item);
                return NULL;
            }

            result = PyList_Concat(previous_result, influential_arguments);
            Py_DECREF(previous_result);
            Py_DECREF(influential_arguments);
            if (result == NULL) {
                Py_DECREF(agents);
                Py_DECREF(iterator); Py_DECREF(item);
                return NULL;
            }
        }

        Py_DECREF(item);
    }

    Py_DECREF(iterator);
    Py_DECREF(agents);

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
 * @brief Return a set with the arguments that are attacking/supporting Argument arg1 or Argument arg2
 * directly or indirectly, NULL if an error has been encountered.
 * 
 * @param self an instance of QBAFramework
 * @param arg1 an instance of QBAFArgument
 * @param arg2 an instance of QBAFArgument
 * @return PyObject* a new PySet of QBAFArgument, NULL if an error occurred
 */
static inline PyObject *
_QBAFramework_influential_arguments_set(QBAFrameworkObject *self, PyObject *arg1, PyObject *arg2)
{
    PyObject *not_visited = PySet_New(self->arguments); // New reference (copy)
    if (not_visited == NULL) {
        return NULL;
    }

    PyObject *visiting = PySet_New(NULL);
    if (visiting == NULL) {
        Py_DECREF(not_visited);
        return NULL;
    }

    PyObject *influential_arg1 = _QBAFramework_influential_arguments(self, arg1, not_visited, visiting);
    if (influential_arg1 == NULL) {
        Py_DECREF(not_visited); Py_DECREF(visiting);
        return NULL;
    }

    PyObject *influential_arg2 = _QBAFramework_influential_arguments(self, arg2, not_visited, visiting);
    if (influential_arg2 == NULL) {
        Py_DECREF(not_visited); Py_DECREF(visiting);
        Py_DECREF(influential_arg1);
        return NULL;
    }

    Py_DECREF(not_visited);
    Py_DECREF(visiting);

    PyObject *influential_arguments = PyList_Concat(influential_arg1, influential_arg2);
    if (influential_arguments == NULL) {
        Py_DECREF(influential_arg1); Py_DECREF(influential_arg2);
        return NULL;
    }

    Py_DECREF(influential_arg1);
    Py_DECREF(influential_arg2);

    PyObject *set = PySet_New(influential_arguments);
    if (set == NULL) {
        Py_DECREF(influential_arguments);
        return NULL;
    }

    Py_DECREF(influential_arguments);

    return set;
}

/**
 * @brief Return True if the Argument argument is candidate for a CSI explanation, False if not,
 * -1 if an error has occurred
 * An Argument is candidate if it is not contained by one of the frameworks or
 * it has a different initial weight or it has a different final weight between the frameworks or
 * it has different relations as attacker/supporter (as attacked/supported not checked).
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param argument a QBAFArgument
 * @return int 1 if candidate, 0 if not candidate, -1 if an error occurred
 */
static inline int
_QBAFramework_candidate_argument(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *argument)
{
    // if not (argument in self.arguments and argument in other.arguments): return True
    int self_contains = PySet_Contains(self->arguments, argument);
    if (self_contains < 0)
        return -1;
    int other_contains = PySet_Contains(other->arguments, argument);
    if (other_contains < 0)
        return -1;
    if (!self_contains || !other_contains) {
        return TRUE;
    }

    // if self.initial_weights[argument] != other.initial_weights[argument]: return True
    PyObject *self_initial = PyDict_GetItemWithError(self->initial_weights, argument); // Borrowed reference
    if (self_initial == NULL)
        return -1;
    PyObject *other_initial = PyDict_GetItemWithError(other->initial_weights, argument); // Borrowed reference
    if (other_initial == NULL)
        return -1;
    int equals = PyObject_RichCompareBool(self_initial, other_initial, Py_EQ);
    if (equals < 0)
        return -1;
    if (!equals) {
        return TRUE;
    }

    // if self.final_weight(argument) != other.final_weight(argument): return True
    PyObject *self_final = _QBAFramework_final_weight(self, argument);
    if (self_final == NULL)
        return -1;
    PyObject *other_final = _QBAFramework_final_weight(other, argument);
    if (other_final == NULL)
        return -1;
    equals = PyObject_RichCompareBool(self_final, other_final, Py_EQ);
    if (equals < 0)
        return -1;
    if (!equals) {
        return TRUE;
    }

    // if self.attack_relations.patients(argument) != other.attack_relations.patients(argument): return True
    int equal_attack_patients = _QBAFARelations_equal_patients(
        (QBAFARelationsObject*)self->attack_relations, (QBAFARelationsObject*)other->attack_relations, argument);
    if (equal_attack_patients < 0)
        return -1;
    if (!equal_attack_patients) {
        return TRUE;
    }

    // if self.support_relations.patients(argument) != other.support_relations.patients(argument): return True
    int equal_support_patients = _QBAFARelations_equal_patients(
        (QBAFARelationsObject*)self->support_relations, (QBAFARelationsObject*)other->support_relations, argument);
    if (equal_support_patients < 0)
        return -1;
    if (!equal_support_patients) {
        return TRUE;
    }

    // return False
    return FALSE;
}

/**
 * @brief Return a list of all the sets of arguments that are minimal SSI Explanations of arg1 and arg2
 * w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), NULL if an error was encountered.
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param arg1 a QBAFArgument
 * @param arg2 a QBAFArgument
 * @return PyObject* new PyList, NULL if an error occurred
 */
static inline PyObject *
_QBAFramework_minimal_SSIExplanations(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *arg1, PyObject *arg2)
{
    // If empty set is a SSI Explanation return it
    PyObject *empty_set = PySet_New(NULL);
    if (empty_set == NULL) {
        return NULL;
    }
    
    int isSSIExplanation = _QBAFramework_isSSIExplanation(self, other, empty_set, arg1, arg2);
    if (isSSIExplanation < 0) {
        Py_DECREF(empty_set);
        return NULL;
    }
    if (isSSIExplanation) {
        PyObject *list = PyList_New(1);
        if (list == NULL) {
            Py_DECREF(empty_set);
            return NULL;
        }
        PyList_SET_ITEM(list, 0, empty_set);
        return list;
    }

    Py_DECREF(empty_set);

    // Obtain the influential arguments (arguments that attack/support arg1 or arg2, directly or indirectly)
    PyObject *self_influential_arguments = _QBAFramework_influential_arguments_set(self, arg1, arg2);
    if (self_influential_arguments == NULL) {
        return NULL;
    }

    PyObject *other_influential_arguments = _QBAFramework_influential_arguments_set(other, arg1, arg2);
    if (other_influential_arguments == NULL) {
        Py_DECREF(self_influential_arguments);
        return NULL;
    }

    PyObject *influential_arguments = PySet_Union(self_influential_arguments, other_influential_arguments);
    if (influential_arguments == NULL) {
        Py_DECREF(self_influential_arguments); Py_DECREF(other_influential_arguments);
        return NULL;
    }

    Py_DECREF(self_influential_arguments);
    Py_DECREF(other_influential_arguments);

    // Filter the candidate arguments (arguments that are 'different' in self and other)
    PyObject *candidate_arguments = PySet_New(NULL);
    if (candidate_arguments == NULL) {
        Py_DECREF(influential_arguments);
        return NULL;
    }

    PyObject *iterator = PyObject_GetIter(influential_arguments);
    PyObject *argument;
    int candidate;

    if (iterator == NULL) {
        Py_DECREF(influential_arguments); Py_DECREF(candidate_arguments);
        return NULL;
    }

    while ((argument = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        candidate = _QBAFramework_candidate_argument(self, other, argument);
        if (candidate < 0) {
            Py_DECREF(influential_arguments); Py_DECREF(candidate_arguments);
            Py_DECREF(argument); Py_DECREF(iterator);
            return NULL;
        }
        
        if (candidate) {
            if (PySet_Add(candidate_arguments, argument) < 0) {
                Py_DECREF(influential_arguments); Py_DECREF(candidate_arguments);
                Py_DECREF(argument); Py_DECREF(iterator);
                return NULL;
            }
        } else {
            Py_DECREF(argument);
        }
    }

    Py_DECREF(iterator);
    Py_DECREF(influential_arguments);

    // Find SSI Explanations trying with size from 1 to length of candidate_arguments
    Py_ssize_t max_size = PySet_GET_SIZE(candidate_arguments);
    PyObject *subsets, *explanations, *set;
    int contains_subset;

    explanations = PyList_New(0);
    if (explanations == NULL) {
        Py_DECREF(candidate_arguments);
        return NULL;
    }

    for (Py_ssize_t size = 1; size <= max_size; size++) {
        subsets = PySet_SubSets(candidate_arguments, size);
        if (subsets == NULL) {
            Py_DECREF(candidate_arguments); Py_DECREF(explanations);
            return NULL;
        }

        iterator = PyObject_GetIter(subsets);
        if (iterator == NULL) {
            Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
            return NULL;
        }

        while ((set = PyIter_Next(iterator))) {
            contains_subset = PyList_ContainsSubset(explanations, set);
            if (contains_subset < 0) {
                Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
                Py_DECREF(set); Py_DECREF(iterator);
                return NULL;
            }
            
            if (!contains_subset) { // If set is not a superset of any explanation
                isSSIExplanation = _QBAFramework_isSSIExplanation(self, other, set, arg1, arg2);
                if (isSSIExplanation < 0) {
                    Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
                    Py_DECREF(set); Py_DECREF(iterator);
                    return NULL;
                }

                if (isSSIExplanation) {
                    if (PyList_Append(explanations, set) < 0) {
                        Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
                        Py_DECREF(set); Py_DECREF(iterator);
                        return NULL;
                    }
                } else {
                    Py_DECREF(set);
                }
            }
        }

        Py_DECREF(iterator);
        Py_DECREF(subsets);

    }

    Py_DECREF(candidate_arguments);

    return explanations;
}

/**
 * @brief Return a list of all the sets of arguments that are minimal SSI Explanations of arg1 and arg2
 * w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), NULL if an error was encountered.
 * 
 * @param self an instance of QBAFramework
 * @param args a tuple with arguments (other: QBAFramework, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds name of the arguments args
 * @return PyObject* new PyList, NULL if an error occurred
 */
static PyObject *
QBAFramework_minimal_SSIExplanations(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", "arg1", "arg2", NULL};
    PyObject *other, *arg1, *arg2;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|", kwlist,
                                     &other, &arg1, &arg2))
        return NULL;

    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "other must be an instance of QBAFramework");
        return NULL;
    }

    return _QBAFramework_minimal_SSIExplanations(self, (QBAFrameworkObject*)other, arg1, arg2);
}

/**
 * @brief Return a list of all the sets of arguments that are minimal CSI Explanations of arg1 and arg2
 * w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), NULL if an error was encountered.
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param arg1 a QBAFArgument
 * @param arg2 a QBAFArgument
 * @return PyObject* new PyList, NULL if an error occurred
 */
static inline PyObject *
_QBAFramework_minimal_CSIExplanations(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *arg1, PyObject *arg2)
{
    // If empty set is a CSI Explanation return it
    PyObject *empty_set = PySet_New(NULL);
    if (empty_set == NULL) {
        return NULL;
    }
    
    int isCSIExplanation = _QBAFramework_isCSIExplanation(self, other, empty_set, arg1, arg2);
    if (isCSIExplanation < 0) {
        Py_DECREF(empty_set);
        return NULL;
    }
    if (isCSIExplanation) {
        PyObject *list = PyList_New(1);
        if (list == NULL) {
            Py_DECREF(empty_set);
            return NULL;
        }
        PyList_SET_ITEM(list, 0, empty_set);
        return list;
    }

    Py_DECREF(empty_set);

    // Obtain the influential arguments (arguments that attack/support arg1 or arg2, directly or indirectly)
    PyObject *self_influential_arguments = _QBAFramework_influential_arguments_set(self, arg1, arg2);
    if (self_influential_arguments == NULL) {
        return NULL;
    }

    PyObject *other_influential_arguments = _QBAFramework_influential_arguments_set(other, arg1, arg2);
    if (other_influential_arguments == NULL) {
        Py_DECREF(self_influential_arguments);
        return NULL;
    }

    PyObject *influential_arguments = PySet_Union(self_influential_arguments, other_influential_arguments);
    if (influential_arguments == NULL) {
        Py_DECREF(self_influential_arguments); Py_DECREF(other_influential_arguments);
        return NULL;
    }

    Py_DECREF(self_influential_arguments);
    Py_DECREF(other_influential_arguments);

    // Filter the candidate arguments (arguments that are 'different' in self and other)
    PyObject *candidate_arguments = PySet_New(NULL);
    if (candidate_arguments == NULL) {
        Py_DECREF(influential_arguments);
        return NULL;
    }

    PyObject *iterator = PyObject_GetIter(influential_arguments);
    PyObject *argument;
    int candidate;

    if (iterator == NULL) {
        Py_DECREF(influential_arguments); Py_DECREF(candidate_arguments);
        return NULL;
    }

    while ((argument = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        candidate = _QBAFramework_candidate_argument(self, other, argument);
        if (candidate < 0) {
            Py_DECREF(influential_arguments); Py_DECREF(candidate_arguments);
            Py_DECREF(argument); Py_DECREF(iterator);
            return NULL;
        }
        
        if (candidate) {
            if (PySet_Add(candidate_arguments, argument) < 0) {
                Py_DECREF(influential_arguments); Py_DECREF(candidate_arguments);
                Py_DECREF(argument); Py_DECREF(iterator);
                return NULL;
            }
        } else {
            Py_DECREF(argument);
        }
    }

    Py_DECREF(iterator);
    Py_DECREF(influential_arguments);

    // Find CSI Explanations trying with size from 1 to length of candidate_arguments
    Py_ssize_t max_size = PySet_GET_SIZE(candidate_arguments);
    PyObject *subsets, *explanations, *set;
    int contains_subset;

    explanations = PyList_New(0);
    if (explanations == NULL) {
        Py_DECREF(candidate_arguments);
        return NULL;
    }

    for (Py_ssize_t size = 1; size <= max_size; size++) {
        subsets = PySet_SubSets(candidate_arguments, size);
        if (subsets == NULL) {
            Py_DECREF(candidate_arguments); Py_DECREF(explanations);
            return NULL;
        }

        iterator = PyObject_GetIter(subsets);
        if (iterator == NULL) {
            Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
            return NULL;
        }

        while ((set = PyIter_Next(iterator))) {
            contains_subset = PyList_ContainsSubset(explanations, set);
            if (contains_subset < 0) {
                Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
                Py_DECREF(set); Py_DECREF(iterator);
                return NULL;
            }
            
            if (!contains_subset) { // If set is not a superset of any explanation
                isCSIExplanation = _QBAFramework_isCSIExplanation(self, other, set, arg1, arg2);
                if (isCSIExplanation < 0) {
                    Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
                    Py_DECREF(set); Py_DECREF(iterator);
                    return NULL;
                }

                if (isCSIExplanation) {
                    if (PyList_Append(explanations, set) < 0) {
                        Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(candidate_arguments);
                        Py_DECREF(set); Py_DECREF(iterator);
                        return NULL;
                    }
                } else {
                    Py_DECREF(set);
                }
            }
        }

        Py_DECREF(iterator);
        Py_DECREF(subsets);

    }

    Py_DECREF(candidate_arguments);

    return explanations;
}

/**
 * @brief Return a list of all the sets of arguments that are minimal CSI Explanations of arg1 and arg2
 * w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), NULL if an error was encountered.
 * 
 * @param self an instance of QBAFramework
 * @param args a tuple with arguments (other: QBAFramework, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds name of the arguments args
 * @return PyObject* new PyList, NULL if an error occurred
 */
static PyObject *
QBAFramework_minimal_CSIExplanations(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"other", "arg1", "arg2", NULL};
    PyObject *other, *arg1, *arg2;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|", kwlist,
                                     &other, &arg1, &arg2))
        return NULL;

    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "other must be an instance of QBAFramework");
        return NULL;
    }

    return _QBAFramework_minimal_CSIExplanations(self, (QBAFrameworkObject*)other, arg1, arg2);
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
    {"final_weights", (getter) QBAFramework_getfinal_weights, NULL,
     "Return a copy of the final weights.", NULL},
    {"disjoint_relations", (getter) QBAFramework_getdisjoint_relations, (setter) QBAFramework_setdisjoint_relations,
     "Return True if the attack/support relations must be disjoint, False if they do not have to.",
     "Setter of the attribute disjoint_relations."},
    {NULL}  /* Sentinel */
};

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
    {"final_weight", (PyCFunctionWithKeywords) QBAFramework_final_weight, METH_VARARGS | METH_KEYWORDS,
    "Return the final weight of the Argument argument."
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
    {"are_strength_consistent", (PyCFunctionWithKeywords) QBAFramework_are_strength_consistent, METH_VARARGS | METH_KEYWORDS,
    "Return True if a pair of arguments are strength consistent between two frameworks, False otherwise."
    },
    {"reversal", (PyCFunctionWithKeywords) QBAFramework_reversal, METH_VARARGS | METH_KEYWORDS,
    "Return the reversal framework of self to other w.r.t. set."
    },
    {"isSSIExplanation", (PyCFunctionWithKeywords) QBAFramework_isSSIExplanation, METH_VARARGS | METH_KEYWORDS,
    "Return True if a set of arguments set is Sufficient Strength Inconsistency (SSI) Explanation, False if not."
    },
    {"isCSIExplanation", (PyCFunctionWithKeywords) QBAFramework_isCSIExplanation, METH_VARARGS | METH_KEYWORDS,
    "Return True if a set of arguments set is Counterfactual Strength Inconsistency (CSI) Explanation, False if not."
    },
    {"minimal_SSIExplanations", (PyCFunctionWithKeywords) QBAFramework_minimal_SSIExplanations, METH_VARARGS | METH_KEYWORDS,
    "Return a list of a set of arguments that are minimal (all have the same size) SSI Explanations\nof arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF)."
    },
    {"minimal_CSIExplanations", (PyCFunctionWithKeywords) QBAFramework_minimal_CSIExplanations, METH_VARARGS | METH_KEYWORDS,
    "Return a list of a set of arguments that are minimal (all have the same size) CSI Explanations\nof arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF)."
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