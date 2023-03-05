/**
 * @file framework.c
 * @author Jose Ruiz Alarcon
 * @brief Definition of the PyTypeObject QBAFramework.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <float.h>
#include <string.h>

#include "framework.h"
#include "relations.h"
#include "qbaf_utils.h"
#include "qbaf_functions.h"

#ifndef stricmp
#include <ctype.h>
int stricmp(const char *s1, const char *s2) {
    while (tolower((unsigned char) *s1) == tolower((unsigned char) *s2)) {
        if (*s1 == '\0')
            return 0;
        s1++; s2++;
    }

    return (int) tolower((unsigned char) *s1) - (int) tolower((unsigned char) *s2);
}
#endif

#define TRUE 1
#define FALSE 0

#define Py_RETURN_BOOL(i) if (i) Py_RETURN_TRUE; else Py_RETURN_FALSE;
#define PyObject_IsNoneOrNULL(obj) (obj == Py_None || obj == NULL)
#define streq(str1, str2) (stricmp(str1, str2) == 0)

static const char *STR_BASIC_MODEL = "basic_model";
static const char *STR_QUADRATICENERGY_MODEL = "QuadraticEnergy_model";
static const char *STR_SQUAREDDFQUAD_MODEL = "SquaredDFQuAD_model";
static const char *STR_EULERBASEDTOP_MODEL = "EulerBasedTop_model";
static const char *STR_EULERBASED_MODEL = "EulerBased_model";
static const char *STR_DFQUAD_MODEL = "DFQuAD_model";

/**
 * @brief Struct that defines the Object Type Framework in a QBAF.
 * 
 */
typedef struct {
    PyObject_HEAD
    PyObject *arguments;            /* a set of QBAFArgument */
    PyObject *initial_strengths;      /* a dictionary (argument: QBAFArgument, initial_strength: double) */
    PyObject *attack_relations;     /* an instance of QBAFARelations */
    PyObject *support_relations;    /* an instance of QBAFARelations */
    PyObject *final_strengths;        /* a dictionary (argument: QBAFArgument, final_strength: double) */
    int       modified;             /* 0 if the framework has not been modified after calculating the final strengths. Otherwise, 1 */
    int       disjoint_relations;   /* 1 if the attack/support relations must be disjoint, 0 if they do not have to */
    char     *semantics;            /* name of the semantic model */
    double  (*influence_function)(double, double);   /* influence function that is going to be used to calcualte the final strengths */
    double  (*aggregation_function)(double, double); /* aggregation function that is going to be used to calcualte the final strengths */
    double    min_strength;           /* min value for the initial strengths */
    double    max_strength;           /* max value for the initial strengths */
    PyObject *influence_function_callable;   /* influence function given from python */
    PyObject *aggregation_function_callable; /* aggregation function given from python */
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
    Py_VISIT(self->initial_strengths);
    Py_VISIT(self->attack_relations);
    Py_VISIT(self->support_relations);
    Py_VISIT(self->final_strengths);
    Py_VISIT(self->influence_function_callable);
    Py_VISIT(self->aggregation_function_callable);
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
    Py_CLEAR(self->initial_strengths);
    Py_CLEAR(self->attack_relations);
    Py_CLEAR(self->support_relations);
    Py_CLEAR(self->final_strengths);
    Py_CLEAR(self->influence_function_callable);
    Py_CLEAR(self->aggregation_function_callable);
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
        self->initial_strengths = Py_None;
        Py_INCREF(Py_None);
        self->attack_relations = Py_None;
        Py_INCREF(Py_None);
        self->support_relations = Py_None;
        Py_INCREF(Py_None);
        self->final_strengths = Py_None;
        self->modified = TRUE;
        self->disjoint_relations = TRUE;
        self->semantics = STR_BASIC_MODEL;
        self->influence_function = simple_influence;
        self->aggregation_function = sum;
        self->min_strength = -DBL_MAX;
        self->max_strength = DBL_MAX;
        self->influence_function_callable = NULL;
        self->aggregation_function = NULL;
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
            PyErr_SetString(PyExc_TypeError, "all items of initial_strengths must be of a numeric type (int, float)");
            Py_DECREF(item);
            Py_DECREF(iterator);
            Py_DECREF(new);
            return NULL;
        }

        if (PyLong_Check(item)) {
            // Transform the PyLong to PyFloat
            double strength = PyLong_AsDouble(item);
            Py_DECREF(item);
            if (strength == -1.0 && PyErr_Occurred()) {
                Py_DECREF(iterator);
                Py_DECREF(new);
                return NULL;
            }

            item = PyFloat_FromDouble(strength);  // New reference
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
 * @brief Return true if the double value of pyfloat is within the range (min, max),
 * false if is not, and -1 if an error has occurred.
 * 
 * @param pyfloat a PyFloat
 * @param max the maximum value of the range
 * @param min the minimum value of the range
 * @return int 1 if within range, 0 if not, -1 if an error occurred
 */
static inline int
strength_in_minmax(PyObject *pyfloat, double min, double max)
{
    double strength = PyFloat_AsDouble(pyfloat);
    if (strength == -1.0 && PyErr_Occurred()) {
        return -1;
    }

    if (strength < min || strength > max) {
        return FALSE;
    }

    return TRUE;
}

static inline int
_QBAFramework_initial_strengths_in_minmax(QBAFrameworkObject *self)
{
    PyObject *initial_strength_list = PyDict_Values(self->initial_strengths);
    if (initial_strength_list == NULL) {
        return -1;
    }

    PyObject *iterator = PyObject_GetIter(initial_strength_list);
    PyObject *initial_strength;
    int in_minmax;

    if (iterator == NULL) {
        Py_DECREF(initial_strength_list);
        return -1;
    }

    while ((initial_strength = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        in_minmax = strength_in_minmax(initial_strength, self->min_strength, self->max_strength);
        Py_DECREF(initial_strength);

        if (in_minmax < 0) {
            Py_DECREF(iterator); Py_DECREF(initial_strength_list);
            return -1;
        }

        if (!in_minmax) {
            Py_DECREF(iterator); Py_DECREF(initial_strength_list);
            return FALSE;
        }
    }

    Py_DECREF(iterator);
    Py_DECREF(initial_strength_list);

    return TRUE;
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
    static char *kwlist[] = {"arguments", "initial_strengths", "attack_relations", "support_relations",
                            "disjoint_relations", "semantics", "aggregation_function", "influence_function",
                            "min_strength", "max_strength", NULL};
    PyObject *arguments, *initial_strengths, *attack_relations, *support_relations, *tmp;
    int disjoint_relations = TRUE;
    char *semantics = NULL; // (e.g. "basic_model") If None it will be NULL, otherwise it is a pointer to char that is only accesible in this function.
    PyObject *aggregation_function = NULL, *influence_function = NULL;
    double min_strength = -DBL_MAX, max_strength = DBL_MAX;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO|pzOOdd", kwlist,
                                     &arguments, &initial_strengths, &attack_relations, &support_relations,
                                     &disjoint_relations, &semantics, &aggregation_function, &influence_function,
                                     &min_strength, &max_strength))
        return -1;

    if (!PyList_Check(arguments)) {
        PyErr_SetString(PyExc_TypeError, "arguments must be of type list");
        return -1;
    }

    if (!PyList_Check(initial_strengths)) {
        PyErr_SetString(PyExc_TypeError, "initial_strengths must be of type list");
        return -1;
    }

    if (PyList_GET_SIZE(arguments) != PyList_GET_SIZE(initial_strengths)) {
        PyErr_SetString(PyExc_ValueError, "the lengths of arguments and initial_strengths must be equal");
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

    // Check that all init strengths are numerical values
    initial_strengths = PyListFloat_FromPyListNumeric(initial_strengths);   // New reference
    if (initial_strengths == NULL) {
        return -1;
    }

    // Initialize initial_strengths
    tmp = self->initial_strengths;
    self->initial_strengths = PyDict_FromLists(arguments, initial_strengths);
    Py_DECREF(initial_strengths);
    if (self->initial_strengths == NULL) {
        /* propagate error*/
        self->initial_strengths = tmp;
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

    if (semantics == NULL && PyObject_IsNoneOrNULL(aggregation_function) && PyObject_IsNoneOrNULL(influence_function)) {
        semantics = STR_BASIC_MODEL;
    }

    if (semantics != NULL && (min_strength != -DBL_MAX || max_strength != DBL_MAX)) {
        PyErr_SetString(PyExc_ValueError,
                        "cannot modify min_strength or max_strength without implementing your own aggregation function and influence function");
        return -1;
    }

    // Implement aggregation_function, influence_function
    if (!PyObject_IsNoneOrNULL(aggregation_function) || !PyObject_IsNoneOrNULL(influence_function)) {
        if (semantics != NULL) {
            PyErr_SetString(PyExc_ValueError,
                            "cannot modify the aggregation_function and influence_function of the semantics");
            return -1;
        }

        if (!PyCallable_Check(aggregation_function) || !PyCallable_Check(influence_function)) {
            PyErr_SetString(PyExc_ValueError,
            "aggregation_function and influence_function must be callable");
            return -1;
        }

        self->semantics = NULL;
        self->influence_function = NULL;
        self->aggregation_function = NULL;

        Py_XDECREF(self->influence_function_callable);
        Py_INCREF(influence_function);
        self->influence_function_callable = influence_function;

        Py_XDECREF(self->aggregation_function_callable);
        Py_INCREF(aggregation_function);
        self->aggregation_function_callable = aggregation_function;

        self->min_strength = min_strength;
        self->max_strength = max_strength;

    }

    // Assign the influence_function and aggregation_function based on the value of semantics
    if (semantics != NULL) {

        if (streq(semantics, STR_BASIC_MODEL)) {
            self->semantics = STR_BASIC_MODEL;
            self->aggregation_function = sum;
            self->influence_function = simple_influence;
            self->min_strength = -DBL_MAX;
            self->max_strength = DBL_MAX;
        }
        else if (streq(semantics, STR_QUADRATICENERGY_MODEL)) {
            self->semantics = STR_QUADRATICENERGY_MODEL;
            self->aggregation_function = sum;
            self->influence_function = max_2_1; // 2-Max(1)
            self->min_strength = -DBL_MAX;
            self->max_strength = DBL_MAX;
        }
        else if (streq(semantics, STR_SQUAREDDFQUAD_MODEL)) {
            self->semantics = STR_SQUAREDDFQUAD_MODEL;
            self->aggregation_function = product;
            self->influence_function = max_1_1; // 1-Max(1)
            self->min_strength = -DBL_MAX;
            self->max_strength = DBL_MAX;
        }
        else if (streq(semantics, STR_EULERBASEDTOP_MODEL)) {
            self->semantics = STR_EULERBASEDTOP_MODEL;
            self->aggregation_function = top;
            self->influence_function = euler_based;
            self->min_strength = -DBL_MAX;
            self->max_strength = DBL_MAX;
        }
        else if (streq(semantics, STR_EULERBASED_MODEL)) {
            self->semantics = STR_EULERBASED_MODEL;
            self->aggregation_function = sum;
            self->influence_function = euler_based;
            self->min_strength = -DBL_MAX;
            self->max_strength = DBL_MAX;
        }
        else if (streq(semantics, STR_DFQUAD_MODEL)) {
            self->semantics = STR_DFQUAD_MODEL;
            self->aggregation_function = product;
            self->influence_function = linear_1; // Linear(1)
            self->min_strength = -1;
            self->max_strength = 1;
        }
        else {
            PyErr_SetString(PyExc_ValueError, "incorrect value of semantics");
            return -1;
        }

    }

    // Check all the initial strengths are in range (min_strength, max_strength)
    int initial_strengths_in_minmax = _QBAFramework_initial_strengths_in_minmax(self);
    if (initial_strengths_in_minmax < 0) {
        return -1;
    }
    if (!initial_strengths_in_minmax) {
        char msg[100];
        sprintf(msg, "every initial_strength must be within range (%.2f, %.2f)", self->min_strength, self->max_strength);
        PyErr_SetString(PyExc_ValueError, msg);
        return -1;
    }

    return 0;
}

/**
 * @brief Return the result of aggregating the strengths w1 and w2 in the Framework self,
 * -1.0 in an error has occurred.
 * 
 * @param self a QBAFramework
 * @param w1 a double
 * @param w2 another double
 * @return double result of the aggregation of w1 and w2, -1.0 if an error occurred
 */
static inline
double _QBAFramework_aggregation_function(QBAFrameworkObject *self, double w1, double w2)
{
    if (self->aggregation_function != NULL)
        return self->aggregation_function(w1, w2);
    
    if (self->aggregation_function_callable != NULL) {
        PyObject *pyfloat = PyObject_CallFunction(self->aggregation_function_callable, "dd", w1, w2);
        if (pyfloat == NULL)
            return -1;
        double aggregation = PyFloat_AsDouble(pyfloat);
        Py_DECREF(pyfloat);
        return aggregation;
    }

    PyErr_BadArgument();
    return -1.0;
}

/**
 * @brief Return the result of the influence function of the Framework self,
 * -1.0 in an error has occurred.
 * 
 * @param self a QBAFramework
 * @param w the initial strength
 * @param s the result of applying the aggregation function to all attackers and supporters
 * @return double the result, -1.0 if an error occurred
 */
static inline
double _QBAFramework_influence_function(QBAFrameworkObject *self, double w, double s)
{
    if (self->influence_function != NULL)
        return self->influence_function(w, s);
    
    if (self->influence_function_callable != NULL) {
        PyObject *pyfloat = PyObject_CallFunction(self->influence_function_callable, "dd", w, s);
        if (pyfloat == NULL)
            return -1;
        double influence = PyFloat_AsDouble(pyfloat);
        Py_DECREF(pyfloat);
        return influence;
    }

    PyErr_BadArgument();
    return -1.0;
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
 * @brief Getter of the attribute initial_strengths.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* copy of a dict of (argument: QBAFArgument, initial_strength: float)
 */
static PyObject *
QBAFramework_getinitial_strengths(QBAFrameworkObject *self, void *closure)
{
    return PyDict_Copy(self->initial_strengths);
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
    Py_RETURN_BOOL(self->disjoint_relations);
}

/**
 * @brief Getter of the attribute semantics.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* new PyUnicode with the string semantics, new PyNone if semantics is NULL
 */
static PyObject *
QBAFramework_getsemantics(QBAFrameworkObject *self, void *closure)
{
    if (self->semantics != NULL)
        return PyUnicode_FromString(self->semantics);
    Py_RETURN_NONE;
}

/**
 * @brief Getter of the attribute min_strength.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* new PyFloat with the min_strength
 */
static PyObject *
QBAFramework_getmin_strength(QBAFrameworkObject *self, void *closure)
{
    return PyFloat_FromDouble(self->min_strength);
}

/**
 * @brief Getter of the attribute max_strength.
 * 
 * @param self the QBAFramework object
 * @param closure 
 * @return PyObject* new PyFloat with the max_strength
 */
static PyObject *
QBAFramework_getmax_strength(QBAFrameworkObject *self, void *closure)
{
    return PyFloat_FromDouble(self->max_strength);
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
 * @brief Modify the initial strength of the Argument argument.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument, initial_strength: float)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_modify_initial_strengths(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", "initial_strength", NULL};
    PyObject *argument, *initial_strength;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|", kwlist,
                                     &argument, &initial_strength))
        return NULL;

    if (!PyFloat_Check(initial_strength) && !PyLong_Check(initial_strength)) {
        PyErr_SetString(PyExc_TypeError, "initial_strength must be of a numeric type");
        return NULL;
    }

    if (PyLong_Check(initial_strength)) {
        // Transform the PyLong to PyFloat
        double strength = PyLong_AsDouble(initial_strength);
        if (strength == -1.0 && PyErr_Occurred()) {
            return NULL;
        }

        initial_strength = PyFloat_FromDouble(strength);    // New reference
        if (initial_strength == NULL) {
            PyErr_SetString(PyExc_ValueError, "initial_strength could not be transformed to float");
            return NULL;
        }
    } else {  
        Py_INCREF(initial_strength);
    }

    // Check initial_strength is within range
    int in_minmax = strength_in_minmax(initial_strength, self->min_strength, self->max_strength);
    if (in_minmax < 0) {
        Py_DECREF(initial_strength);
        return NULL;
    }
    if (!in_minmax) {
        char msg[100];
        sprintf(msg, "initial_strength must be within range (%.2f, %.2f)", self->min_strength, self->max_strength);
        PyErr_SetString(PyExc_ValueError, msg);
        Py_DECREF(initial_strength);
        return NULL;
    }

    Py_INCREF(argument);
    if (PyDict_SetItem(self->initial_strengths, argument, initial_strength) < 0) {
        Py_DECREF(argument);
        Py_DECREF(initial_strength);
        return NULL;
    }

    Py_DECREF(initial_strength);

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Return the initial strength of the Argument argument, NULL in case of error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyFloat
 */
static PyObject *
QBAFramework_initial_strength(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", NULL};
    PyObject *argument;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &argument))
        return NULL;

    int contains = PyDict_Contains(self->initial_strengths, argument);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument must be contained in the QBAFramework");
        return NULL;
    }

    PyObject *initial_strength = PyDict_GetItem(self->initial_strengths, argument);
    Py_INCREF(initial_strength);
    return initial_strength;
}

/**
 * @brief Add an Argument to the Framework. If it exists already it does nothing.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument, initial_strength: float)
 * @param kwds the argument names
 * @return PyObject* new Py_None, NULL in case of error
 */
static PyObject *
QBAFramework_add_argument(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", "initial_strength", NULL};
    PyObject *argument, *initial_strength = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist,
                                     &argument, &initial_strength))
        return NULL;

    double default_initial_strength = 0.0;

    if (initial_strength == NULL) {

        initial_strength = PyFloat_FromDouble(default_initial_strength);    // New reference
        if (initial_strength == NULL)
            return NULL;

    } else {

        if (!PyFloat_Check(initial_strength) && !PyLong_Check(initial_strength)) {
            PyErr_SetString(PyExc_TypeError, "initial_strength must be of a numeric type");
            return NULL;
        }

        if (PyLong_Check(initial_strength)) {
            // Transform the PyLong to PyFloat
            double strength = PyLong_AsDouble(initial_strength);
            if (strength == -1.0 && PyErr_Occurred()) {
                return NULL;
            }

            initial_strength = PyFloat_FromDouble(strength);    // New reference
            if (initial_strength == NULL) {
                PyErr_SetString(PyExc_ValueError, "initial_strength could not be transformed to float");
                return NULL;
            }
        } else {  
            Py_INCREF(initial_strength);
        }

    }

    int contains = PySet_Contains(self->arguments, argument);
    if (contains < 0) {
        Py_DECREF(initial_strength);
        return NULL;
    }
    if (contains) { // If the argument already exists it does nothing
        Py_RETURN_NONE;
    }

    // Check initial_strength is within range
    int in_minmax = strength_in_minmax(initial_strength, self->min_strength, self->max_strength);
    if (in_minmax < 0) {
        Py_DECREF(initial_strength);
        return NULL;
    }
    if (!in_minmax) {
        char msg[100];
        sprintf(msg, "initial_strength must be within range (%.2f, %.2f)", self->min_strength, self->max_strength);
        PyErr_SetString(PyExc_ValueError, msg);
        Py_DECREF(initial_strength);
        return NULL;
    }

    // the new argument, initial_strength is added
    Py_INCREF(argument);
    if (PyDict_SetItem(self->initial_strengths, argument, initial_strength) < 0) {
        Py_DECREF(argument);
        Py_DECREF(initial_strength);
        return NULL;
    }

    Py_DECREF(initial_strength);

    Py_INCREF(argument);
    if (PySet_Add(self->arguments, argument) < 0) {
        Py_DECREF(argument);
        return NULL;
    }

    self->modified = TRUE;

    Py_RETURN_NONE;
}

/**
 * @brief Remove an Argument from the Framework. If it does not exist already it does nothing.
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

    if (PyDict_DelItem(self->initial_strengths, argument) < 0) {
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
 * @brief Return the arguments that are attacked by the argument attacker,
 * NULL (with the corresponding exception) if an error has occurred.
 * 
 * @param self 
 * @param args the argument values (attacker: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyList of QBAFArgument, NULL if an error occurred
 */
static PyObject *
QBAFramework_attackedBy(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"attacker", NULL};
    PyObject *attacker;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &attacker))
        return NULL;

    int contains = PySet_Contains(self->arguments, attacker);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "attacker must be an argument of the framework");
        return NULL;
    }

    return _QBAFARelations_patients(self->attack_relations, attacker);
}

/**
 * @brief Return the arguments that are attacking the argument attacked,
 * NULL (with the corresponding exception) if an error has occurred.
 * 
 * @param self 
 * @param args the argument values (attacked: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyList of QBAFArgument, NULL if an error occurred
 */
static PyObject *
QBAFramework_attackersOf(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"attacked", NULL};
    PyObject *attacked;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &attacked))
        return NULL;

    int contains = PySet_Contains(self->arguments, attacked);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "attacked must be an argument of the framework");
        return NULL;
    }

    return _QBAFARelations_agents(self->attack_relations, attacked);
}

/**
 * @brief Return the arguments that are supported by the argument supporter,
 * NULL (with the corresponding exception) if an error has occurred.
 * 
 * @param self 
 * @param args the argument values (supporter: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyList of QBAFArgument, NULL if an error occurred
 */
static PyObject *
QBAFramework_supportedBy(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"supporter", NULL};
    PyObject *supporter;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &supporter))
        return NULL;

    int contains = PySet_Contains(self->arguments, supporter);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "supporter must be an argument of the framework");
        return NULL;
    }

    return _QBAFARelations_patients(self->support_relations, supporter);
}

/**
 * @brief Return the arguments that are supporting the argument supported,
 * NULL (with the corresponding exception) if an error has occurred.
 * 
 * @param self 
 * @param args the argument values (supported: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyList of QBAFArgument, NULL if an error occurred
 */
static PyObject *
QBAFramework_supportersOf(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"supported", NULL};
    PyObject *supported;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &supported))
        return NULL;

    int contains = PySet_Contains(self->arguments, supported);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "supported must be an argument of the framework");
        return NULL;
    }

    return _QBAFARelations_agents(self->support_relations, supported);
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

    Py_DECREF(copy->initial_strengths);
    copy->initial_strengths = PyDict_Copy(self->initial_strengths);
    if (copy->initial_strengths == NULL) {
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

    if (!self->modified) {
        Py_DECREF(copy->final_strengths);
        copy->final_strengths = PyDict_Copy(self->final_strengths);
        if (copy->final_strengths == NULL) {
            Py_DECREF(copy);
            return NULL;
        }
    }

    copy->modified = self->modified;
    copy->disjoint_relations = self->disjoint_relations;

    copy->semantics = self->semantics;
    copy->aggregation_function = self->aggregation_function;
    copy->influence_function = self->influence_function;
    copy->min_strength = self->min_strength;
    copy->max_strength = self->max_strength;

    Py_XINCREF(self->aggregation_function_callable);
    copy->aggregation_function_callable = self->aggregation_function_callable;
    Py_XINCREF(self->influence_function_callable);
    copy->influence_function_callable = self->influence_function_callable;
    
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
 * @brief Return the final strength of a specific argument, -1.0 if an error occurred.
 * This function calls itself recursively. So, it only works with acyclic arguments.
 * It stores all the calculated final strengths in self.__final_strengths.
 * 
 * @param self an instance of QBAFramework
 * @param argument the QBAFArgument
 * @return PyObject* borrowed PyFloat reference, -1.0 if an error has occurrred
 */
static double
_QBAFramework_calculate_final_strength(QBAFrameworkObject *self, PyObject *argument)
{
    int contains = PyDict_Contains(self->final_strengths, argument);
    if (contains < 0) {
        return -1.0;
    }
    if (contains) {
        return PyFloat_AsDouble(PyDict_GetItem(self->final_strengths, argument));
    }

    double initial_strength = PyFloat_AsDouble(PyDict_GetItem(self->initial_strengths, argument));
    double attackers_aggregation = 0;
    double supporters_aggregation = 0;

    PyObject *attackers = _QBAFARelations_agents((QBAFARelationsObject*)self->attack_relations, argument);
    PyObject *iterator = PyObject_GetIter(attackers);
    PyObject *item;

    if (iterator == NULL) {
        Py_XDECREF(attackers);
        return -1.0;
    }

    // if len(attackers) > 0: attackers_aggregation = attackers[0]
    item = PyIter_Next(iterator);
    if (item != NULL) {
        attackers_aggregation = _QBAFramework_calculate_final_strength(self, item);
        Py_DECREF(item);
        if (attackers_aggregation == -1.0 && PyErr_Occurred()) {
            Py_DECREF(attackers); Py_DECREF(iterator);
            return -1.0;
        }
    }

    // for item in attackers[1:]:
    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double item_final_strength = _QBAFramework_calculate_final_strength(self, item);
        Py_DECREF(item);
        if (item_final_strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(attackers); Py_DECREF(iterator);
            return -1.0;
        }
        attackers_aggregation = _QBAFramework_aggregation_function(self, attackers_aggregation, item_final_strength);
        if (attackers_aggregation == -1.0 && PyErr_Occurred()) {
            Py_DECREF(attackers); Py_DECREF(iterator);
            return -1.0;
        }
    }

    Py_DECREF(attackers);
    Py_DECREF(iterator);

    PyObject *supporters = _QBAFARelations_agents((QBAFARelationsObject*)self->support_relations, argument);
    iterator = PyObject_GetIter(supporters);

    if (iterator == NULL) {
        Py_XDECREF(supporters);
        return -1.0;
    }

    // if len(supporters) > 0: supporters_aggregation = supporters[0]
    item = PyIter_Next(iterator);
    if (item != NULL) {
        supporters_aggregation = _QBAFramework_calculate_final_strength(self, item);
        Py_DECREF(item);
        if (supporters_aggregation == -1.0 && PyErr_Occurred()) {
            Py_DECREF(supporters); Py_DECREF(iterator);
            return -1.0;
        }
    }

    // for item in supporters[1:]:
    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        double item_final_strength = _QBAFramework_calculate_final_strength(self, item);
        Py_DECREF(item);
        if (item_final_strength == -1.0 && PyErr_Occurred()) {
            Py_DECREF(supporters); Py_DECREF(iterator);
            return -1.0;
        }
        supporters_aggregation = _QBAFramework_aggregation_function(self, supporters_aggregation, item_final_strength);
        if (supporters_aggregation == -1.0 && PyErr_Occurred()) {
            Py_DECREF(supporters); Py_DECREF(iterator);
            return -1.0;
        }
    }

    Py_DECREF(supporters);
    Py_DECREF(iterator);

    double aggregation = supporters_aggregation - attackers_aggregation;

    double final_strength = _QBAFramework_influence_function(self, initial_strength, aggregation);
    if (final_strength == -1.0 && PyErr_Occurred()) {
        return -1.0;
    }

    // final_strengths[argument] = final_strength
    PyObject *pyfinal_strength = PyFloat_FromDouble(final_strength);    // New reference
    Py_INCREF(argument);
    if (PyDict_SetItem(self->final_strengths, argument, pyfinal_strength) < 0) {
        Py_DECREF(argument); Py_XDECREF(pyfinal_strength);
        return -1.0;
    }
    Py_XDECREF(pyfinal_strength);

    return final_strength;
}

/**
 * @brief Calculate the final strengths of all the arguments of the Framework.
 * It stores all the calculated final strengths in self.__final_strengths.
 * 
 * @param self the QBAFramework
 * @return int 0 if succesful, -1 if an error occurred
 */
static int
_QBAFRamework_calculate_final_strengths(QBAFrameworkObject *self)
{
    int isacyclic = _QBAFramework_isacyclic(self);
    if (isacyclic < 0) {
        return -1;
    }
    if (!isacyclic) {
        PyErr_SetString(PyExc_NotImplementedError,
                        "calculate final strengths of non-acyclic framework not implemented");
        return -1;
    }

    Py_CLEAR(self->final_strengths);
    self->final_strengths = PyDict_New();

    PyObject *iterator = PyObject_GetIter(self->arguments);
    PyObject *item;
    if (iterator == NULL) {
        return -1;
    }

    while ((item = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        if (_QBAFramework_calculate_final_strength(self, item) == -1.0 && PyErr_Occurred()) {
            Py_DECREF(item); Py_DECREF(iterator);
            return -1;
        }
        Py_DECREF(item);
    }

    Py_DECREF(iterator);

    return 0;
}

/**
 * @brief Return the final strengths of arguments of the Framework, NULL if an error occurred.
 * If the framework has been modified from the last time they were calculated
 * they are calculated again. Otherwise, it returns the already calculated final strengths.
 * 
 * @param self the QBAFramework
 * @param closure 
 * @return PyObject* a new PyDict, NULL if an error occurred
 */
static PyObject *
QBAFramework_getfinal_strengths(QBAFrameworkObject *self, void *closure)
{
    if (self->modified) {   // Calculate final strengths if the framework has been modified
        if (_QBAFRamework_calculate_final_strengths(self) < 0) {
            return NULL;
        }
        self->modified = FALSE;
    }

    return PyDict_Copy(self->final_strengths);
}

/**
 * @brief Return the final strength of the Argument argument, NULL in case of error.
 * 
 * @param self an instance of QBAFramework
 * @param argument the QBAFArgument
 * @return PyObject* borrowed PyFloat
 */
static PyObject *
_QBAFramework_final_strength(QBAFrameworkObject *self, PyObject *argument)
{
    if (self->modified) {   // Calculate final strengths if the framework has been modified
        if (_QBAFRamework_calculate_final_strengths(self) < 0) {
            return NULL;
        }
        self->modified = FALSE;
    }

    int contains = PyDict_Contains(self->final_strengths, argument);
    if (contains < 0) {
        return NULL;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError,
                        "argument must be contained in the QBAFramework");
        return NULL;
    }

    return PyDict_GetItem(self->final_strengths, argument);
}

/**
 * @brief Return the final strength of the Argument argument, NULL in case of error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (argument: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* new PyFloat
 */
static PyObject *
QBAFramework_final_strength(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"argument", NULL};
    PyObject *argument;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|", kwlist,
                                     &argument))
        return NULL;

    PyObject *final_strength = _QBAFramework_final_strength(self, argument); // Borrowed reference
    Py_XINCREF(final_strength);
    return final_strength;
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
    if (self->modified) {   // Calculate final strengths if the framework has been modified
        if (_QBAFRamework_calculate_final_strengths(self) < 0) {
            return -1;
        }
        self->modified = FALSE;
    }
    if (other->modified) {   // Calculate final strengths if the framework has been modified
        if (_QBAFRamework_calculate_final_strengths(other) < 0) {
            return -1;
        }
        other->modified = FALSE;
    }

    // Check that the arguments are contained in both frameworks
    int contains = PyDict_Contains(self->final_strengths, arg1);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg1 must be an argument of this QBAFramework");
        return -1;
    }
    contains = PyDict_Contains(self->final_strengths, arg2);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg2 must be an argument of this QBAFramework");
        return -1;
    }
    contains = PyDict_Contains(other->final_strengths, arg1);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg1 must be an argument of the QBAFramework other");
        return -1;
    }
    contains = PyDict_Contains(other->final_strengths, arg2);
    if (contains < 0) {
        return -1;
    }
    if (!contains) {
        PyErr_SetString(PyExc_ValueError, "arg2 must be an argument of the QBAFramework other");
        return -1;
    }

    double self_final_strength_arg1 = PyFloat_AS_DOUBLE(PyDict_GetItem(self->final_strengths, arg1));
    double self_final_strength_arg2 = PyFloat_AS_DOUBLE(PyDict_GetItem(self->final_strengths, arg2));
    double other_final_strength_arg1 = PyFloat_AS_DOUBLE(PyDict_GetItem(other->final_strengths, arg1));
    double other_final_strength_arg2 = PyFloat_AS_DOUBLE(PyDict_GetItem(other->final_strengths, arg2));

    if (self_final_strength_arg1 < self_final_strength_arg2)
        return other_final_strength_arg1 < other_final_strength_arg2;
    if (self_final_strength_arg1 > self_final_strength_arg2)
        return other_final_strength_arg1 > other_final_strength_arg2;
    return other_final_strength_arg1 == other_final_strength_arg2;
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
 * @brief Return a new QBAFramework copying the settings of self (disjoint_relations and semantics).
 * 
 * @param self instance of QBAFramework
 * @return PyObject* new instance of QBAFramework, NULL if an error has occurred
 */
static inline PyObject *
_QBAFramework_copy_settings(QBAFrameworkObject *self)
{
    PyObject *kwds = NULL;
    PyObject *args = NULL;

    QBAFrameworkObject *copy = (QBAFrameworkObject*) QBAFramework_new(Py_TYPE(self), args, kwds);
    if (copy == NULL) {
        return NULL;
    }

    Py_DECREF(copy->arguments);
    copy->arguments = PySet_New(NULL);
    if (copy->arguments == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    Py_DECREF(copy->initial_strengths);
    copy->initial_strengths = PyDict_New();
    if (copy->initial_strengths == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    Py_DECREF(copy->attack_relations);
    copy->attack_relations = QBAFARelations_Create(NULL);
    if (copy->attack_relations == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    Py_DECREF(copy->support_relations);
    copy->support_relations = QBAFARelations_Create(NULL);
    if (copy->support_relations == NULL) {
        Py_DECREF(copy);
        return NULL;
    }

    copy->modified = TRUE;
    copy->disjoint_relations = self->disjoint_relations;

    copy->semantics = self->semantics;
    copy->aggregation_function = self->aggregation_function;
    copy->influence_function = self->influence_function;
    copy->min_strength = self->min_strength;
    copy->max_strength = self->max_strength;

    Py_XINCREF(self->aggregation_function_callable);
    copy->aggregation_function_callable = self->aggregation_function_callable;
    Py_XINCREF(self->influence_function_callable);
    copy->influence_function_callable = self->influence_function_callable;
    
    return (PyObject*)copy;
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

    // Copy this framework settings
    QBAFrameworkObject *reversal = (QBAFrameworkObject*)_QBAFramework_copy_settings(self);
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
    Py_DECREF(reversal->arguments);
    reversal->arguments = PySet_Difference(self_arguments_union_set, set_difference_other_arguments);
    Py_DECREF(self_arguments_union_set);
    Py_DECREF(set_difference_other_arguments);
    if (reversal->arguments == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }

    PyObject *set_iterator;
    PyObject *arg; // item
    PyObject *iterator, *attacked, *supported, *patients;
    PyObject *patients_intersection_other_arguments;
    PyObject *patients_intersection_reversal_arguments;

    // Modify attack relations
    Py_DECREF(reversal->attack_relations);
    reversal->attack_relations = QBAFARelations_copy((QBAFARelationsObject*)self->attack_relations, NULL);
    if (reversal->attack_relations == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    set_iterator = PyObject_GetIter(set);
    if (set_iterator == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    while ((arg = PyIter_Next(set_iterator))) {    // PyIter_Next returns a new reference
        // for attacked in self.__attack_relations.patients(arg).intersection(other): att.remove(arg, attacked)
        patients = _QBAFARelations_patients_set((QBAFARelationsObject*)self->attack_relations, arg); // borrowed reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        patients_intersection_other_arguments = PySet_Intersection(patients, other->arguments);
        if (patients_intersection_other_arguments == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients_intersection_other_arguments);
        if (iterator == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator); Py_DECREF(patients_intersection_other_arguments);
            return NULL;
        }
        while ((attacked = PyIter_Next(iterator))) {
            if (_QBAFARelations_remove((QBAFARelationsObject*)reversal->attack_relations, arg, attacked) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(attacked); Py_DECREF(iterator); Py_DECREF(patients_intersection_other_arguments);
                return NULL;
            }
            Py_DECREF(attacked);
        }
        Py_DECREF(patients_intersection_other_arguments);
        Py_DECREF(iterator);

        // for attacked in other.__attack_relations.patients(arg).intersection(args): att.add_relation(arg, attacked)
        patients = _QBAFARelations_patients_set((QBAFARelationsObject*)other->attack_relations, arg); // borrowed reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        patients_intersection_reversal_arguments = PySet_Intersection(patients, reversal->arguments);
        if (patients_intersection_reversal_arguments == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients_intersection_reversal_arguments);
        if (iterator == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator); Py_DECREF(patients_intersection_reversal_arguments);
            return NULL;
        }
        while ((attacked = PyIter_Next(iterator))) {
            if (_QBAFARelations_add((QBAFARelationsObject*)reversal->attack_relations, arg, attacked) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(attacked); Py_DECREF(iterator); Py_DECREF(patients_intersection_reversal_arguments);
                return NULL;
            }
            Py_DECREF(attacked);
        }
        Py_DECREF(patients_intersection_reversal_arguments);
        Py_DECREF(iterator);
        
        // Decref arg
        Py_DECREF(arg);
    }
    Py_DECREF(set_iterator);

    // Modify support relations
    Py_DECREF(reversal->support_relations);
    reversal->support_relations = QBAFARelations_copy((QBAFARelationsObject*)self->support_relations, NULL);
    if (reversal->support_relations == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    set_iterator = PyObject_GetIter(set);
    if (set_iterator == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    while ((arg = PyIter_Next(set_iterator))) {    // PyIter_Next returns a new reference
        // for supported in self.__support_relations.patients(arg).intersection(other): supp.remove(arg, supported)
        patients = _QBAFARelations_patients_set((QBAFARelationsObject*)self->support_relations, arg); // borrowed reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        patients_intersection_other_arguments = PySet_Intersection(patients, other->arguments);
        if (patients_intersection_other_arguments == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients_intersection_other_arguments);
        if (iterator == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator); Py_DECREF(patients_intersection_other_arguments);
            return NULL;
        }
        while ((supported = PyIter_Next(iterator))) {
            if (_QBAFARelations_remove((QBAFARelationsObject*)reversal->support_relations, arg, supported) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(supported); Py_DECREF(iterator); Py_DECREF(patients_intersection_other_arguments);
                return NULL;
            }
            Py_DECREF(supported);
        }
        Py_DECREF(patients_intersection_other_arguments);
        Py_DECREF(iterator);

        // for supported in other.__support_relations.patients(arg).intersection(args): supp.add_relation(arg, supported)
        patients = _QBAFARelations_patients_set((QBAFARelationsObject*)other->support_relations, arg); // borrowed reference
        if (patients == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        patients_intersection_reversal_arguments = PySet_Intersection(patients, reversal->arguments);
        if (patients_intersection_reversal_arguments == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
            return NULL;
        }
        iterator = PyObject_GetIter(patients_intersection_reversal_arguments);
        if (iterator == NULL) {
            Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator); Py_DECREF(patients_intersection_reversal_arguments);
            return NULL;
        }
        while ((supported = PyIter_Next(iterator))) {
            if (_QBAFARelations_add((QBAFARelationsObject*)reversal->support_relations, arg, supported) < 0) {
                Py_DECREF(reversal); Py_DECREF(arg); Py_DECREF(set_iterator);
                Py_DECREF(supported); Py_DECREF(iterator); Py_DECREF(patients_intersection_reversal_arguments);
                return NULL;
            }
            Py_DECREF(supported);
        }
        Py_DECREF(patients_intersection_reversal_arguments);
        Py_DECREF(iterator);
        
        // Decref arg
        Py_DECREF(arg);
    }
    Py_DECREF(set_iterator);

    // Remove the attack/support relations that are not in Args*xArgs*
    PyObject *self_union_other_arguments = PySet_Union(self->arguments, other->arguments);
    if (self_union_other_arguments == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    PyObject *self_union_other_difference_reversal_arguments = PySet_Difference(self_union_other_arguments, reversal->arguments);
    Py_DECREF(self_union_other_arguments);
    if (self_union_other_difference_reversal_arguments == NULL) {
        Py_DECREF(reversal);
        return NULL;
    }
    if (_QBAFARelations_remove_arguments((QBAFARelationsObject*)reversal->attack_relations, self_union_other_difference_reversal_arguments) < 0) {
        Py_DECREF(reversal); Py_DECREF(self_union_other_difference_reversal_arguments);
        return NULL;
    }
    if (_QBAFARelations_remove_arguments((QBAFARelationsObject*)reversal->support_relations, self_union_other_difference_reversal_arguments) < 0) {
        Py_DECREF(reversal); Py_DECREF(self_union_other_difference_reversal_arguments);
        return NULL;
    }
    Py_DECREF(self_union_other_difference_reversal_arguments);

    // Modify initial strengths
    if (reversal->initial_strengths == NULL) { // It should be an empty PyDict
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
    PyObject *initial_strength;
    while ((arg = PyIter_Next(iterator))) {    // PyIter_Next returns a new reference
        int contains = PySet_Contains(other_arguments_intersection_set, arg);
        if (contains < 0) {
            Py_DECREF(reversal); Py_DECREF(iterator);
            Py_DECREF(arg); Py_DECREF(other_arguments_intersection_set);
            return NULL;
        }
        if (contains) {
            initial_strength = PyDict_GetItem(other->initial_strengths, arg);
        } else {
            initial_strength = PyDict_GetItem(self->initial_strengths, arg);
        }
        if (initial_strength == NULL) {
            Py_DECREF(reversal); Py_DECREF(iterator);
            Py_DECREF(arg); Py_DECREF(other_arguments_intersection_set);
            return NULL;
        }
        if (PyDict_SetItem(reversal->initial_strengths, arg, initial_strength) < 0) {
            Py_DECREF(reversal); Py_DECREF(iterator);
            Py_DECREF(arg); Py_DECREF(other_arguments_intersection_set);
            return NULL;
        }
    }
    Py_DECREF(iterator);
    Py_DECREF(other_arguments_intersection_set);

    // Remove calculated final strengths
    Py_CLEAR(reversal->final_strengths);
    reversal->final_strengths = PyDict_New();
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
 * @brief Return True if a set of arguments set is Counterfactual Strength Inconsistency (CSI) Explanation
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
 * @brief Return True if a set of arguments set is Necessary Strength Inconsistency (NSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * -1 if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param set a PySet of QBAFArgument
 * @param arg1 a QBAFArgument
 * @param arg2 a QBAFArgument
 * @return int 1 if it is a NSI Explanation, 0 if it is not, -1 if an error has occurred
 */
static int
_QBAFramework_isNSIExplanation(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *set, PyObject *arg1, PyObject *arg2)
{
    // if not isSSIExplanation: return False
    int isSSIExplanation = _QBAFramework_isSSIExplanation(self, other, set, arg1, arg2);
    if (isSSIExplanation < 0)
        return -1;
    if (!isSSIExplanation)
        return FALSE;

    PyObject *self_arguments_union_other_arguments = PySet_Union(self->arguments, other->arguments);
    if (self_arguments_union_other_arguments == NULL) {
        return -1;
    }
    PyObject *self_arguments_union_other_arguments_difference_set = PySet_Difference(self_arguments_union_other_arguments, set);
    Py_DECREF(self_arguments_union_other_arguments);
    if (self_arguments_union_other_arguments_difference_set == NULL) {
        return -1;
    }

    // Find any SSI Explanation trying with size from 1 to length of self_arguments_union_other_arguments_difference_set
    Py_ssize_t max_size = PySet_GET_SIZE(self_arguments_union_other_arguments_difference_set);
    PyObject *iterator, *subsets, *currentset;

    for (Py_ssize_t size = 1; size <= max_size; size++) {
        subsets = PySet_SubSets(self_arguments_union_other_arguments_difference_set, size);
        if (subsets == NULL) {
            Py_DECREF(self_arguments_union_other_arguments_difference_set);
            return -1;
        }

        iterator = PyObject_GetIter(subsets);
        if (iterator == NULL) {
            Py_DECREF(subsets); Py_DECREF(self_arguments_union_other_arguments_difference_set);
            return -1;
        }

        while ((currentset = PyIter_Next(iterator))) {
            isSSIExplanation = _QBAFramework_isSSIExplanation(self, other, currentset, arg1, arg2);
            if (isSSIExplanation < 0) {
                Py_DECREF(subsets); Py_DECREF(self_arguments_union_other_arguments_difference_set);
                Py_DECREF(currentset); Py_DECREF(iterator);
                return -1;
            }

            // if any subset isSSIExplanation: return False
            if (isSSIExplanation) {
                Py_DECREF(subsets); Py_DECREF(self_arguments_union_other_arguments_difference_set);
                Py_DECREF(currentset); Py_DECREF(iterator);
                return FALSE;
            }

            Py_DECREF(currentset);
        }

        Py_DECREF(iterator);
        Py_DECREF(subsets);
    }

    Py_DECREF(self_arguments_union_other_arguments_difference_set);

    // return True
    return TRUE;
}

/**
 * @brief Return True if a set of arguments set is Sufficient Strength Inconsistency (SSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * NULL if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (other: QBAFramework, set: PySet of QBAFArgument, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* PyTrue if it is a SSI Explanation, PyFalse if it is not, NULL if an error has occurred
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
 * @brief Return True if a set of arguments set is Counterfactual Strength Inconsistency (CSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * NULL if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (other: QBAFramework, set: PySet of QBAFArgument, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* PyTrue if it is a CSI Explanation, PyFalse if it is not, NULL if an error has occurred
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
 * @brief Return True if a set of arguments set is Necessary Strength Inconsistency (NSI) Explanation
 * of arg1 and arg2 w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), False if not,
 * NULL if encountered an error.
 * 
 * @param self an instance of QBAFramework
 * @param args the argument values (other: QBAFramework, set: PySet of QBAFArgument, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds the argument names
 * @return PyObject* PyTrue if it is a NSI Explanation, PyFalse if it is not, NULL if an error has occurred
 */
static PyObject *
QBAFramework_isNSIExplanation(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
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

    int isNSIExplanation = _QBAFramework_isNSIExplanation(self, (QBAFrameworkObject*)other, set, arg1, arg2);
    Py_DECREF(set);
    if (isNSIExplanation < 0) {
        return NULL;
    }

    Py_RETURN_BOOL(isNSIExplanation);
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
 * it has a different initial strength or it has a different final strength between the frameworks or
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

    // if self.initial_strengths[argument] != other.initial_strengths[argument]: return True
    PyObject *self_initial = PyDict_GetItemWithError(self->initial_strengths, argument); // Borrowed reference
    if (self_initial == NULL)
        return -1;
    PyObject *other_initial = PyDict_GetItemWithError(other->initial_strengths, argument); // Borrowed reference
    if (other_initial == NULL)
        return -1;
    int equals = PyObject_RichCompareBool(self_initial, other_initial, Py_EQ);
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
_QBAFramework_minimalSSIExplanations(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *arg1, PyObject *arg2)
{
    // If strength consistent return a list with empty set
    int strength_consistent = _QBAFramework_are_strength_consistent(self, other, arg1, arg2);
    if (strength_consistent < 0) {
        return NULL;
    }
    if (strength_consistent) {
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
    int contains_subset, isSSIExplanation;

    explanations = PyList_New(0);
    if (explanations == NULL) {
        Py_DECREF(candidate_arguments);
        return NULL;
    }

    subsets = PySet_PowersetWithoutEmptySet(candidate_arguments);
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
                Py_INCREF(set);
            }
        }

        Py_DECREF(set);
    }

    Py_DECREF(iterator);
    Py_DECREF(subsets);

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
QBAFramework_minimalSSIExplanations(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
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

    return _QBAFramework_minimalSSIExplanations(self, (QBAFrameworkObject*)other, arg1, arg2);
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
_QBAFramework_minimalCSIExplanations(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *arg1, PyObject *arg2)
{
    // If strength consistent return a list with empty set
    int strength_consistent = _QBAFramework_are_strength_consistent(self, other, arg1, arg2);
    if (strength_consistent < 0) {
        return NULL;
    }
    if (strength_consistent) {
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
    int contains_subset, isCSIExplanation;

    explanations = PyList_New(0);
    if (explanations == NULL) {
        Py_DECREF(candidate_arguments);
        return NULL;
    }

    subsets = PySet_PowersetWithoutEmptySet(candidate_arguments);
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
                Py_INCREF(set);
            }
        }

        Py_DECREF(set);
    }

    Py_DECREF(iterator);
    Py_DECREF(subsets);


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
QBAFramework_minimalCSIExplanations(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
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

    return _QBAFramework_minimalCSIExplanations(self, (QBAFrameworkObject*)other, arg1, arg2);
}

/**
 * @brief Return a list of all the sets of arguments that are minimal NSI Explanations of arg1 and arg2
 * w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), NULL if an error was encountered.
 * 
 * @param self an instance of QBAFramework
 * @param other a different instance of QBAFramework
 * @param arg1 a QBAFArgument
 * @param arg2 a QBAFArgument
 * @return PyObject* new PyList, NULL if an error occurred
 */
static inline PyObject *
_QBAFramework_minimalNSIExplanations(QBAFrameworkObject *self, QBAFrameworkObject *other, PyObject *arg1, PyObject *arg2)
{
    // If strength consistent return a list with empty set
    int strength_consistent = _QBAFramework_are_strength_consistent(self, other, arg1, arg2);
    if (strength_consistent < 0) {
        return NULL;
    }
    if (strength_consistent) {
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

    PyObject *explanations = PyList_New(0);
    if (explanations == NULL) {
        return NULL;
    }

    PyObject *minimalSSIExplanations = _QBAFramework_minimalSSIExplanations(self, other, arg1, arg2);
    if (minimalSSIExplanations == NULL) {
        Py_DECREF(explanations);
        return NULL;
    }

    PyObject *minimalSSIExplanations_unionset = PyListOfPySet_Union(minimalSSIExplanations); // Union of all arguments in minimal SSI Explanations
    if (minimalSSIExplanations_unionset == NULL) {
        Py_DECREF(explanations); Py_DECREF(minimalSSIExplanations);
        return NULL;
    }

    PyObject *subsets = PySet_PowersetWithoutEmptySet(minimalSSIExplanations_unionset);
    Py_DECREF(minimalSSIExplanations_unionset);
    if (subsets == NULL) {
        Py_DECREF(minimalSSIExplanations); Py_DECREF(explanations);
        return NULL;
    }

    // Discard sets that have an empty intersection with at least one minimal SSI Explanation
    PyObject *filtered_subsets = PyList_New(0);
    if (filtered_subsets == NULL) {
        Py_DECREF(minimalSSIExplanations); Py_DECREF(explanations);
        Py_DECREF(subsets);
        return NULL;
    }

    PyObject *iterator = PyObject_GetIter(subsets);
    PyObject *set;
    if (iterator == NULL) {
        Py_DECREF(minimalSSIExplanations); Py_DECREF(explanations);
        Py_DECREF(subsets); Py_DECREF(filtered_subsets);
        return NULL;
    }

    while ((set = PyIter_Next(iterator))) {
        int contains_disjoint = PyList_ContainsDisjoint(minimalSSIExplanations, set);
        if (contains_disjoint < 0) {
            Py_DECREF(minimalSSIExplanations); Py_DECREF(explanations);
            Py_DECREF(subsets); Py_DECREF(filtered_subsets);
            Py_DECREF(iterator); Py_DECREF(set);
            return NULL;
        }
        
        if (!contains_disjoint) {
            if (PyList_Append(filtered_subsets, set) < 0) {
                Py_DECREF(minimalSSIExplanations); Py_DECREF(explanations);
                Py_DECREF(subsets); Py_DECREF(filtered_subsets);
                Py_DECREF(iterator); Py_DECREF(set);
                return NULL;
            }
            Py_INCREF(set);
        }

        Py_DECREF(set);
    }
    Py_DECREF(iterator);

    Py_DECREF(subsets);
    subsets = filtered_subsets;

    // Find NSI Explanations
    int contains_subset;
    int isSSIExplanation;
    int contains_subset_SSIExplanation;

    iterator = PyObject_GetIter(subsets);
    if (iterator == NULL) {
        Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
        return NULL;
    }

    while ((set = PyIter_Next(iterator))) {
        contains_subset = PyList_ContainsSubset(explanations, set);
        if (contains_subset < 0) {
            Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
            Py_DECREF(set); Py_DECREF(iterator);
            return NULL;
        }
        
        if (!contains_subset) { // If set is not a superset of any explanation
            isSSIExplanation = _QBAFramework_isSSIExplanation(self, other, set, arg1, arg2);
            if (isSSIExplanation < 0) {
                Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
                Py_DECREF(set); Py_DECREF(iterator);
                return NULL;
            }

            if (isSSIExplanation) { // If set is SSI Explanation

                PyObject *self_arguments_union_other_arguments = PySet_Union(self->arguments, other->arguments);
                if (self_arguments_union_other_arguments == NULL) {
                    Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
                    Py_DECREF(set); Py_DECREF(iterator);
                    return NULL;
                }
                PyObject *self_arguments_union_other_arguments_difference_set = PySet_Difference(self_arguments_union_other_arguments, set);
                Py_DECREF(self_arguments_union_other_arguments);
                if (self_arguments_union_other_arguments_difference_set == NULL) {
                    Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
                    Py_DECREF(set); Py_DECREF(iterator);
                    return NULL;
                }

                contains_subset_SSIExplanation = PyList_ContainsSubset(minimalSSIExplanations, self_arguments_union_other_arguments_difference_set);
                Py_DECREF(self_arguments_union_other_arguments_difference_set);
                if (contains_subset_SSIExplanation < 0) {
                    Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
                    Py_DECREF(set); Py_DECREF(iterator);
                    return NULL;
                }

                if (!contains_subset_SSIExplanation) { // If it does not exist a SSI Explanation that is subset of self->arguments.union(other->arguments).difference(set)
                    if (PyList_Append(explanations, set) < 0) {
                        Py_DECREF(explanations); Py_DECREF(subsets); Py_DECREF(minimalSSIExplanations);
                        Py_DECREF(set); Py_DECREF(iterator);
                        return NULL;
                    }
                    Py_INCREF(set);
                }
            }
        }

        Py_DECREF(set);
    }

    Py_DECREF(iterator);
    Py_DECREF(subsets);

    Py_DECREF(minimalSSIExplanations);

    return explanations;
}

/**
 * @brief Return a list of all the sets of arguments that are minimal NSI Explanations of arg1 and arg2
 * w.r.t. QBAFramework self (QBF') and QBAFramework other (QBF), NULL if an error was encountered.
 * 
 * @param self an instance of QBAFramework
 * @param args a tuple with arguments (other: QBAFramework, arg1: QBAFArgument, arg2: QBAFArgument)
 * @param kwds name of the arguments args
 * @return PyObject* new PyList, NULL if an error occurred
 */
static PyObject *
QBAFramework_minimalNSIExplanations(QBAFrameworkObject *self, PyObject *args, PyObject *kwds)
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

    return _QBAFramework_minimalNSIExplanations(self, (QBAFrameworkObject*)other, arg1, arg2);
}

/**
 * @brief Return the comparison result between two QBAFramework, NULL if an error has occurred.
 * 
 * @param self instance of QBAFramework
 * @param other different instance of QBAFramework
 * @param op operation type (only Py_EQ and Py_NE are implemented)
 * @return PyObject* new PyBool, NULL if an error occurred
 */
static PyObject *
QBAFramework_richcompare(QBAFrameworkObject *self, PyObject *other, int op)
{
    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "cannot compare instance of 'QBAFramework' with instance of a different type");
        return NULL;
    }

    if ((op != Py_EQ) && (op != Py_NE)) {
        PyErr_SetNone(PyExc_NotImplementedError);
        return NULL;
    }

    int equals = PyObject_RichCompareBool(self->arguments, ((QBAFrameworkObject *)other)->arguments, Py_EQ);
    if (equals < 0)
        return NULL;
    if (!equals)
        Py_RETURN_BOOL(op != Py_EQ);

    equals = PyObject_RichCompareBool(self->initial_strengths, ((QBAFrameworkObject *)other)->initial_strengths, Py_EQ);
    if (equals < 0)
        return NULL;
    if (!equals)
        Py_RETURN_BOOL(op != Py_EQ);

    equals = PyObject_RichCompareBool(self->attack_relations, ((QBAFrameworkObject *)other)->attack_relations, Py_EQ);
    if (equals < 0)
        return NULL;
    if (!equals)
        Py_RETURN_BOOL(op != Py_EQ);

    equals = PyObject_RichCompareBool(self->support_relations, ((QBAFrameworkObject *)other)->support_relations, Py_EQ);
    if (equals < 0)
        return NULL;
    if (!equals)
        Py_RETURN_BOOL(op != Py_EQ);

    Py_RETURN_BOOL(op == Py_EQ);
}

PyDoc_STRVAR(arguments_doc,
"Set of arguments of the Framework.\n"
"\n"
"Getter: Return a copy of the QBAFramework's set of arguments\n"
"\n"
"Type: set of QBAFArgument\n"
);

PyDoc_STRVAR(initial_strengths_doc,
"Initial strengths of the arguments of the Framework.\n"
"\n"
"Getter: Return a copy of the QBAFramework's initial strengths\n"
"\n"
"Type: dict of QBAFArgument: float\n"
);

PyDoc_STRVAR(attack_relations_doc,
"Attack relations of the Framework.\n"
"\n"
"Getter: Return the QBAFramework's attack relations\n"
"\n"
"Type: QBAFARelations\n"
);

PyDoc_STRVAR(support_relations_doc,
"Support relations of the Framework.\n"
"\n"
"Getter: Return the QBAFramework's support relations\n"
"\n"
"Type: QBAFARelations\n"
);

PyDoc_STRVAR(final_strengths_doc,
"Final strengths of the arguments of the Framework.\n"
"\n"
"Getter: Calculate and return the QBAFramework's final strengths.\n"
"    If the Framework has not been modified since last time they were calculated,\n"
"    a copy of the previously calculated final strengths is returned.\n"
"\n"
"Type: dict of QBAFArgument: float\n"
);

PyDoc_STRVAR(disjoint_relations_doc,
"True if the attack/support relations must be disjoint, False if they do not have to.\n"
"\n"
"Getter: Return if the attack/support relations must be disjoint or not\n"
"\n"
"Setter: Set if the attack/support relations must be disjoint or not\n"
"\n"
"Type: bool\n"
);

PyDoc_STRVAR(semantics_doc,
"The name of the semantics used to calculate the final strengths of the Framework.\n"
"If the semantics are custom (not predefined) then its value is None.\n"
"\n"
"Getter: Return the QBAFramework's semantics name. None if it is custom.\n"
"\n"
"Type: str\n"
);

PyDoc_STRVAR(min_strength_doc,
"The minimun value an initial strength can have in the Framework.\n"
"\n"
"Getter: Return the QBAFramework's minimum strength.\n"
"\n"
"Type: float\n"
);

PyDoc_STRVAR(max_strength_doc,
"The maximun value an initial strength can have in the Framework.\n"
"\n"
"Getter: Return the QBAFramework's maximum strength.\n"
"\n"
"Type: float\n"
);

/**
 * @brief A list with the setters and getters of the class QBAFramework
 * 
 */
static PyGetSetDef QBAFramework_getsetters[] = {
    {"arguments", (getter) QBAFramework_getarguments, NULL,
     arguments_doc, NULL},
    {"initial_strengths", (getter) QBAFramework_getinitial_strengths, NULL,
     initial_strengths_doc, NULL},
    {"attack_relations", (getter) QBAFramework_getattack_relations, NULL,
     attack_relations_doc, NULL},
    {"support_relations", (getter) QBAFramework_getsupport_relations, NULL,
     support_relations_doc, NULL},
    {"final_strengths", (getter) QBAFramework_getfinal_strengths, NULL,
     final_strengths_doc, NULL},
    {"disjoint_relations", (getter) QBAFramework_getdisjoint_relations, (setter) QBAFramework_setdisjoint_relations,
     disjoint_relations_doc, NULL},
    {"semantics", (getter) QBAFramework_getsemantics, NULL,
     semantics_doc, NULL},
    {"min_strength", (getter) QBAFramework_getmin_strength, NULL,
     min_strength_doc, NULL},
    {"max_strength", (getter) QBAFramework_getmax_strength, NULL,
     max_strength_doc, NULL},
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(modify_initial_strength_doc,
"modify_initial_strength(self, argument, initial_strength)\n"
"--\n"
"\n"
"Modify the initial strength of the argument.\n"
"\n"
"Args:\n"
"    argument (QBAFArgument): the argument to be modified\n"
"    initial_strength (float): the new value of initial strength\n"
);

PyDoc_STRVAR(initial_strength_doc,
"initial_strength(self, argument)\n"
"--\n"
"\n"
"Return the initial strength of the argument.\n"
"\n"
"Args:\n"
"    argument (QBAFARelations): the argument\n"
"\n"
"Returns:\n"
"    float: the initial strength\n"
);

PyDoc_STRVAR(final_strength_doc,
"final_strength(self, argument)\n"
"--\n"
"\n"
"Return the final strength of the argument.\n"
"If the framework has been modified from the last time the final strengths were calculated\n"
"they are calculated again. Otherwise, it returns the already calculated final strength.\n"
"\n"
"Args:\n"
"    argument (QBAFARelations): the argument\n"
"\n"
"Returns:\n"
"    float: the initial strength\n"
);

PyDoc_STRVAR(add_argument_doc,
"add_argument(self, argument, initial_strength=0.0)\n"
"--\n"
"\n"
"Add an argument to the Framework. If it already exists it does nothing.\n"
"\n"
"Args:\n"
"    argument (QBAFArgument): the argument\n"
"    initial_strength (float, optional): the initial strength of the argument. Defaults to 0.0.\n"
);

PyDoc_STRVAR(remove_argument_doc,
"remove_argument(self, argument)\n"
"--\n"
"\n"
"Remove the argument from the Framework. If it does not exist it does nothing.\n"
"\n"
"Args:\n"
"    argument (QBAFArgument): the argument\n"
);

PyDoc_STRVAR(add_attack_relation_doc,
"add_attack_relation(self, attacker, attacked)\n"
"--\n"
"\n"
"Add the Attack relation (attacker, attacked) to the Framework.\n"
"The relation's arguments must be contained in the Framework's arguments.\n"
"If the Attack relation already exists, this method does nothing.\n"
"\n"
"Args:\n"
"    attacker (QBAFArgument): the argument that is attacking\n"
"    attacked (QBAFArgument): the argument that is being attacked\n"
);

PyDoc_STRVAR(remove_attack_relation_doc,
"remove_attack_relation(self, attacker, attacked)\n"
"--\n"
"\n"
"Remove the Attack relation (attacker, attacked) from the Framework.\n"
"If the Attack relation does not exist, this method does nothing.\n"
"\n"
"Args:\n"
"    attacker (QBAFArgument): the argument that is attacking\n"
"    attacked (QBAFArgument): the argument that is being attacked\n"
);

PyDoc_STRVAR(add_support_relation_doc,
"add_support_relation(self, supporter, supported)\n"
"--\n"
"\n"
"Add the Support relation (supporter, supported) to the Framework.\n"
"The relation's arguments must be contained in the Framework's arguments.\n"
"If the Support relation already exists, this method does nothing.\n"
"\n"
"Args:\n"
"    supporter (QBAFArgument): the argument that is supporting\n"
"    supported (QBAFArgument): the argument that is being supported\n"
);

PyDoc_STRVAR(remove_support_relation_doc,
"remove_support_relation(self, supporter, supported)\n"
"--\n"
"\n"
"Remove the Support relation (supporter, supported) from the Framework.\n"
"If the Support relation does not exist, this method does nothing.\n"
"\n"
"Args:\n"
"    supporter (QBAFArgument): the argument that is supporting\n"
"    supported (QBAFArgument): the argument that is being supported\n"
);

PyDoc_STRVAR(contains_argument_doc,
"contains_argument(self, argument)\n"
"--\n"
"\n"
"Return True if the argument is contained in the Framework. False if it is not.\n"
"\n"
"Args:\n"
"    argument (QBAFARelations): the argument\n"
"\n"
"Returns:\n"
"    bool: True if contained, False if not contained\n"
);

PyDoc_STRVAR(contains_attack_relation_doc,
"contains_attack_relation(self, argument)\n"
"--\n"
"\n"
"Return True if the Attack relation (attacker, attacked) is contained\n"
"in the Framework. False if it is not.\n"
"\n"
"Args:\n"
"    attacker (QBAFArgument): the argument that is attacking\n"
"    attacked (QBAFArgument): the argument that is being attacked\n"
"\n"
"Returns:\n"
"    bool: True if contained, False if not contained\n"
);

PyDoc_STRVAR(contains_support_relation_doc,
"contains_support_relation(self, argument)\n"
"--\n"
"\n"
"Return True if the Support relation (supporter, supported) is contained\n"
"in the Framework. False if it is not.\n"
"\n"
"Args:\n"
"    supporter (QBAFArgument): the argument that is supporting\n"
"    supported (QBAFArgument): the argument that is being supported\n"
"\n"
"Returns:\n"
"    bool: True if contained, False if not contained\n"
);

PyDoc_STRVAR(attackedBy_doc,
"attackedBy(self, attacker)\n"
"--\n"
"\n"
"Return the arguments that are being attacked by the argument attacker.\n"
"\n"
"Args:\n"
"    attacker (QBAFArgument): the argument that is attacking\n"
"\n"
"Returns:\n"
"    list: the arguments that are being attacked\n"
);

PyDoc_STRVAR(attackersOf_doc,
"attackersOf(self, attacked)\n"
"--\n"
"\n"
"Return the arguments that are attacking the argument attacked.\n"
"\n"
"Args:\n"
"    attacked (QBAFArgument): the argument that is being attacked\n"
"\n"
"Returns:\n"
"    list: the arguments that are attacking\n"
);

PyDoc_STRVAR(supportedBy_doc,
"supportedBy(self, supporter)\n"
"--\n"
"\n"
"Return the arguments that are being supported by the argument supporter.\n"
"\n"
"Args:\n"
"    supporter (QBAFArgument): the argument that is supporting\n"
"\n"
"Returns:\n"
"    list: the arguments that are being supported\n"
);

PyDoc_STRVAR(supportersOf_doc,
"supportersOf(self, supported)\n"
"--\n"
"\n"
"Return the arguments that are supporting the argument supported.\n"
"\n"
"Args:\n"
"    supported (QBAFArgument): the argument that is being supported\n"
"\n"
"Returns:\n"
"    list: the arguments that are supporting\n"
);

PyDoc_STRVAR(__copy___doc,
"__copy__(self, /)\n"
"--\n"
"\n"
"Return a shallow copy of self.\n"
);

PyDoc_STRVAR(copy_doc,
"copy(self)\n"
"--\n"
"\n"
"Return a shallow copy of self.\n"
);

PyDoc_STRVAR(isacyclic_doc,
"isacyclic(self)\n"
"--\n"
"\n"
"Return True if the Attack/Support relations of the Framework have no cycles.\n"
"False otherwise.\n"
"\n"
"Returns:\n"
"    bool: True if acyclic, False if not acyclic\n"
);

PyDoc_STRVAR(are_strength_consistent_doc,
"are_strength_consistent(self, other, arg1, arg2)\n"
"--\n"
"\n"
"Return True if the argument arg1 and the argument arg2 are strength consistent\n"
"w.r.t the Framework self and the Framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    bool: True if strength consistent, False if strength inconsistent\n"
);

PyDoc_STRVAR(reversal_doc,
"reversal(self, other, set)\n"
"--\n"
"\n"
"Return the reversal Framework of the Framework self to the Framework other\n"
"w.r.t. the Set of arguments set.\n"
"All arguments in set must be contained in at least one of the Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    set (set): a set of arguments\n"
"\n"
"Returns:\n"
"    QBAFramework: a new Framework\n"
);

PyDoc_STRVAR(isSSIExplanation_doc,
"isSSIExplanation(self, other, set, arg1, arg2)\n"
"--\n"
"\n"
"Return True if the Set of arguments set is Sufficient Strength Inconsistency (SSI) Explanation\n"
"of the argument arg1 and the argument arg2 w.r.t. the framework self and the framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"All arguments in set must be contained in at least one of the Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    set (set): a set of arguments\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    bool: True if SSI Explanation, False if not SSI Explanation\n"
);

PyDoc_STRVAR(isCSIExplanation_doc,
"isCSIExplanation(self, other, set, arg1, arg2)\n"
"--\n"
"\n"
"Return True if the Set of arguments set is Counterfactual Strength Inconsistency (CSI) Explanation\n"
"of the argument arg1 and the argument arg2 w.r.t. the framework self and the framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"All arguments in set must be contained in at least one of the Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    set (set): a set of arguments\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    bool: True if CSI Explanation, False if not CSI Explanation\n"
);

PyDoc_STRVAR(isNSIExplanation_doc,
"isNSIExplanation(self, other, set, arg1, arg2)\n"
"--\n"
"\n"
"Return True if the Set of arguments set is Necessary Strength Inconsistency (NSI) Explanation\n"
"of the argument arg1 and the argument arg2 w.r.t. the framework self and the framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"All arguments in set must be contained in at least one of the Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    set (set): a set of arguments\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    bool: True if NSI Explanation, False if not NSI Explanation\n"
);

PyDoc_STRVAR(minimalSSIExplanations_doc,
"minimalSSIExplanations(self, other, arg1, arg2)\n"
"--\n"
"\n"
"Return all the sets of arguments that are a subset-minimal SSI Explanation\n"
"of the argument arg1 and the argument arg2 w.r.t. the Framework self and the Framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    list: list of set of arguments\n"
);

PyDoc_STRVAR(minimalCSIExplanations_doc,
"minimalCSIExplanations(self, other, arg1, arg2)\n"
"--\n"
"\n"
"Return all the sets of arguments that are a subset-minimal CSI Explanation\n"
"of the argument arg1 and the argument arg2 w.r.t. the Framework self and the Framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    list: list of set of arguments\n"
);

PyDoc_STRVAR(minimalNSIExplanations_doc,
"minimalNSIExplanations(self, other, arg1, arg2)\n"
"--\n"
"\n"
"Return all the sets of arguments that are a subset-minimal NSI Explanation\n"
"of the argument arg1 and the argument arg2 w.r.t. the Framework self and the Framework other.\n"
"Both arguments must be contained in both Frameworks.\n"
"\n"
"Args:\n"
"    other (QBAFramework): a Framework\n"
"    arg1 (QBAFArgument): first argument\n"
"    arg2 (QBAFArgument): second argument\n"
"\n"
"Returns:\n"
"    list: list of set of arguments\n"
);

/**
 * @brief List of functions of the class QBAFramework
 * 
 */
static PyMethodDef QBAFramework_methods[] = {
    {"modify_initial_strength", (PyCFunctionWithKeywords) QBAFramework_modify_initial_strengths, METH_VARARGS | METH_KEYWORDS,
    modify_initial_strength_doc
    },
    {"initial_strength", (PyCFunctionWithKeywords) QBAFramework_initial_strength, METH_VARARGS | METH_KEYWORDS,
    initial_strength_doc
    },
    {"final_strength", (PyCFunctionWithKeywords) QBAFramework_final_strength, METH_VARARGS | METH_KEYWORDS,
    final_strength_doc
    },
    {"add_argument", (PyCFunctionWithKeywords) QBAFramework_add_argument, METH_VARARGS | METH_KEYWORDS,
    add_argument_doc
    },
    {"remove_argument", (PyCFunctionWithKeywords) QBAFramework_remove_argument, METH_VARARGS | METH_KEYWORDS,
    remove_argument_doc
    },
    {"add_attack_relation", (PyCFunctionWithKeywords) QBAFramework_add_attack_relation, METH_VARARGS | METH_KEYWORDS,
    add_attack_relation_doc
    },
    {"add_support_relation", (PyCFunctionWithKeywords) QBAFramework_add_support_relation, METH_VARARGS | METH_KEYWORDS,
    add_support_relation_doc
    },
    {"remove_attack_relation", (PyCFunctionWithKeywords) QBAFramework_remove_attack_relation, METH_VARARGS | METH_KEYWORDS,
    remove_attack_relation_doc
    },
    {"remove_support_relation", (PyCFunctionWithKeywords) QBAFramework_remove_support_relation, METH_VARARGS | METH_KEYWORDS,
    remove_support_relation_doc
    },
    {"contains_argument", (PyCFunctionWithKeywords) QBAFramework_contains_argument, METH_VARARGS | METH_KEYWORDS,
    contains_argument_doc
    },
    {"contains_attack_relation", (PyCFunctionWithKeywords) QBAFramework_contains_attack_relation, METH_VARARGS | METH_KEYWORDS,
    contains_attack_relation_doc
    },
    {"contains_support_relation", (PyCFunctionWithKeywords) QBAFramework_contains_support_relation, METH_VARARGS | METH_KEYWORDS,
    contains_support_relation_doc
    },
    {"attackedBy", (PyCFunctionWithKeywords) QBAFramework_attackedBy, METH_VARARGS | METH_KEYWORDS,
    attackedBy_doc
    },
    {"attackersOf", (PyCFunctionWithKeywords) QBAFramework_attackersOf, METH_VARARGS | METH_KEYWORDS,
    attackersOf_doc
    },
    {"supportedBy", (PyCFunctionWithKeywords) QBAFramework_supportedBy, METH_VARARGS | METH_KEYWORDS,
    supportedBy_doc
    },
    {"supportersOf", (PyCFunctionWithKeywords) QBAFramework_supportersOf, METH_VARARGS | METH_KEYWORDS,
    supportersOf_doc
    },
    {"__copy__", (PyCFunction) QBAFramework_copy, METH_NOARGS,
    __copy___doc
    },
    {"copy", (PyCFunction) QBAFramework_copy, METH_NOARGS,
    copy_doc
    },
    {"isacyclic", (PyCFunction) QBAFramework_isacyclic, METH_NOARGS,
    isacyclic_doc
    },
    {"are_strength_consistent", (PyCFunctionWithKeywords) QBAFramework_are_strength_consistent, METH_VARARGS | METH_KEYWORDS,
    are_strength_consistent_doc
    },
    {"reversal", (PyCFunctionWithKeywords) QBAFramework_reversal, METH_VARARGS | METH_KEYWORDS,
    reversal_doc
    },
    {"isSSIExplanation", (PyCFunctionWithKeywords) QBAFramework_isSSIExplanation, METH_VARARGS | METH_KEYWORDS,
    isSSIExplanation_doc
    },
    {"isCSIExplanation", (PyCFunctionWithKeywords) QBAFramework_isCSIExplanation, METH_VARARGS | METH_KEYWORDS,
    isCSIExplanation_doc
    },
    {"isNSIExplanation", (PyCFunctionWithKeywords) QBAFramework_isNSIExplanation, METH_VARARGS | METH_KEYWORDS,
    isNSIExplanation_doc
    },
    {"minimalSSIExplanations", (PyCFunctionWithKeywords) QBAFramework_minimalSSIExplanations, METH_VARARGS | METH_KEYWORDS,
    minimalSSIExplanations_doc
    },
    {"minimalCSIExplanations", (PyCFunctionWithKeywords) QBAFramework_minimalCSIExplanations, METH_VARARGS | METH_KEYWORDS,
    minimalCSIExplanations_doc
    },
    {"minimalNSIExplanations", (PyCFunctionWithKeywords) QBAFramework_minimalNSIExplanations, METH_VARARGS | METH_KEYWORDS,
    minimalNSIExplanations_doc
    },
    {NULL}  /* Sentinel */
};

PyDoc_STRVAR(QBAFramework_doc,
"This class represents a Quantitative Bipolar Argumentation Framework (QBAF).\n"
"\n"
"A QBAF consists of a set of arguments, Attack relations between arguments,\n"
"Support relations between arguments and each argument has an initial strength.\n"
"\n"
"Each argument has a final strength which is calculated as the result of an influence\n"
"function that combines the initial strength and the aggregation result.\n"
"The aggregation result is obtained as the result of applying an aggregation function\n"
"to the supporters of the argument minus the result of applying the same aggregation\n"
"function to the attackers of the argument.\n"
"\n"
"Note that every time the type QBAFArgument is written, any type that is hashable can be used.\n"
"\n"
"The semantics of a QBAF are associated with the way the final strengths are calculated.\n"
"There are some predefined semantics (The default is 'basic_model'), but custom semantics\n"
"can be created by implementing your own aggregation function and influence function.\n"
"\n"
"Predefined semantics: 'basic_model', 'QuadraticEnergy_model', 'SquaredDFQuAD_model',\n"
"    'EulerBasedTop_model', 'EulerBased_model' and 'DFQuAD_model'.\n"
"\n"
"QBAFramework(arguments, initial_strengths, attack_relations, support_relations,\n"
"    disjoint_relations=True, semantics=None,\n"
"    aggregation_function=None, influence_function=None,\n"
"    min_strength=-1.7976931348623157e+308, max_strength=1.7976931348623157e+308)\n"     
"\n"
"Args:\n"
"    arguments (list): a list of QBAFArgument\n"
"    initial_strengths (list): a list of floats corresponding to each argument of arguments\n"
"    attack_relations (Union[set,list]): a collection of (attacker: QBAFArgument, attacked: QBAFArgument)\n"
"    support_relations (Union[set,list]): a collection of (supporter: QBAFArgument, supported: QBAFArgument)\n"
"    disjoint_relations (bool, optional): True if the Attack relations and the Support relations must be disjoint.\n"
"        Defaults to True.\n"
"    semantics (str, optional): Name of the predifined semantics to be used to calculate the final strengths.\n"
"        Defaults to None. If the aggregation function and the influence function are None it defaults to 'basic_model'.\n"
"    aggregation_function (Callable[[float, float], float], optional): Function to combine two final strengths.\n"
"        Defaults to None.\n"
"    influence_function (Callable[[float, float], float], optional): Function to combine the initial strength\n"
"        and the aggregation result. Defaults to None.\n"
"    min_strength (float, optional): The minimum value an initial strength can have. Defaults to -1.7976931348623157e+308.\n"
"        It can only be modified when the semantics are custom\n"
"    max_strength (float, optional): The maximum value an initial strength can have. Defaults to 1.7976931348623157e+308.\n"
"        It can only be modified when the semantics are custom\n"
);

/**
 * @brief Python definition for the class QBAFramework
 * 
 */
static PyTypeObject QBAFrameworkType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "qbaf.QBAFramework",
    .tp_doc = QBAFramework_doc,
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
    .tp_richcompare = (richcmpfunc) QBAFramework_richcompare,   // __eq__, __ne__
};

/**
 * @brief Get the QBAFrameworkType object created above that defines the class QBAFramework
 * 
 * @return PyTypeObject* a pointer to the QBAFramework class definition
 */
PyTypeObject *get_QBAFrameworkType() {
    return &QBAFrameworkType;
}