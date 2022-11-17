/**
 * @file argument.c
 * @author Jose Ruiz Alarcon
 * @brief Definition of the PyTypeObject QBAFArgument.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "argument.h"

/**
 * @brief Struct that defines the Object Type Argument in a QBAF.
 * 
 */
typedef struct {
    PyObject_HEAD
    PyObject *name;         /* name of the argument and identifier */
    PyObject *description;  /* description of the argument */
} QBAFArgumentObject;

/**
 * @brief This function is used by the garbage collector to detect reference cycles.
 * 
 * @param self a object of type QBAFArgument
 * @param visit 
 * @param arg 
 * @return int 0 if the function was successful. Otherwise, -1.
 */
static int
QBAFArgument_traverse(QBAFArgumentObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->name);
    Py_VISIT(self->description);
    return 0;
}

/**
 * @brief Drop the instance’s references to those of its members that may be Python objects, and set its pointers to those members to NULL.
 * 
 * @param self a object of type QBAFArgument
 * @return int 0 if the function was successful. Otherwise, -1.
 */
static int
QBAFArgument_clear(QBAFArgumentObject *self)
{
    Py_CLEAR(self->name);
    Py_CLEAR(self->description);
    return 0;
}

/**
 * @brief Destructor function that is called to free memory of a object that will no longer be used.
 * 
 * @param self a object of type QBAFArgument
 */
static void
QBAFArgument_dealloc(QBAFArgumentObject *self)
{
    PyObject_GC_UnTrack(self);
    QBAFArgument_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * @brief Constructor of a QBAFArgument.
 * 
 * @param type the Object type
 * @param args the argument values that might be used by the constructor
 * @param kwds the names of the argument values
 * @return PyObject* The created object of type QBAFArgument
 */
static PyObject *
QBAFArgument_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    QBAFArgumentObject *self;
    self = (QBAFArgumentObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->name = PyUnicode_FromString("");
        if (self->name == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->description = PyUnicode_FromString("");
        if (self->description == NULL) {
            Py_DECREF(self->name); // TODO: Check if this is correct
            Py_DECREF(self);
            return NULL;
        }
    }
    return (PyObject *) self;
}

/**
 * @brief Initializer of a QBAFArgument instance. It is called right after the constructor by the python interpreter.
 * 
 * @param self the Object 
 * @param args the argument values that might be used by the constructor
 * @param kwds the names of the argument values
 * @return int 0 if it was executed with no errors. Otherwise, -1.
 */
static int
QBAFArgument_init(QBAFArgumentObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", "description", NULL};
    PyObject *name = NULL, *description = NULL, *tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "U|U", kwlist,
                                     &name, &description))
        return -1;

    if (name) { // It will always be true
        tmp = self->name;
        Py_INCREF(name);
        self->name = name;
        Py_DECREF(tmp);
    }
    if (description) {
        tmp = self->description;
        Py_INCREF(description);
        self->description = description;
        Py_DECREF(tmp);
    }
    return 0;
}

/**
 * @brief A list with the attributes of the class QBAFArgument
 * 
 */
static PyMemberDef QBAFArgument_members[] = {
    {NULL}  /* Sentinel */
};

/**
 * @brief Getter of the attribute name.
 * 
 * @param self the QBAFArgument object
 * @param closure 
 * @return PyObject* the name of the QBAFArgument
 */
static PyObject *
QBAFArgument_getname(QBAFArgumentObject *self, void *closure)
{
    Py_INCREF(self->name);
    return self->name;
}

/**
 * @brief Getter of the attribute description
 * 
 * @param self the QBAFArgument object
 * @param closure 
 * @return PyObject* the description of the QBAFArgument
 */
static PyObject *
QBAFArgument_getdescription(QBAFArgumentObject *self, void *closure)
{
    Py_INCREF(self->description);
    return self->description;
}

/**
 * @brief Setter of the attribute description
 * 
 * @param self the QBAFArgument object
 * @param value the new description
 * @param closure 
 * @return int 0 if it was executed with no errors. Otherwise, -1.
 */
static int
QBAFArgument_setdescription(QBAFArgumentObject *self, PyObject *value, void *closure)
{
    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "Cannot delete the description attribute");
        return -1;
    }
    if (!PyUnicode_Check(value)) {
        PyErr_SetString(PyExc_TypeError,
                        "The description attribute value must be a string");
        return -1;
    }
    Py_INCREF(value);
    Py_CLEAR(self->description);
    self->description = value;
    return 0;
}

/**
 * @brief A list with the setters and getters of the class QBAFArgument
 * 
 */
static PyGetSetDef QBAFArgument_getsetters[] = {
    {"name", (getter) QBAFArgument_getname, NULL,
     "name and identifier", NULL},
    {"description", (getter) QBAFArgument_getdescription, (setter) QBAFArgument_setdescription,
     "argument description", NULL},
    {NULL}  /* Sentinel */
};

/**
 * @brief Return the comparison of the names of two QBAFArgument instances.
 * 
 * @param self a QBAFArgument object
 * @param other another QBAFArgument object
 * @param op the operation type (Py_LT, Py_LE ,Py_EQ ,Py_NE ,Py_GT or Py_GE)
 * @return PyObject* a object with the result of the comparison
 */
static PyObject *
QBAFArgument_richcompare(QBAFArgumentObject *self, PyObject *other, int op)
{
    if (other == NULL) {
        PyErr_SetString(PyExc_TypeError, "Compare instance of 'QBAFArgument' with NULL not supported");
        return NULL;
    }
    if (!PyObject_TypeCheck(other, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "Compare instance of 'QBAFArgument' with instance of a different type not supported");
        return NULL;
    }
    return PyObject_RichCompare(self->name, ((QBAFArgumentObject *)other)->name, op);
}

/**
 * @brief Return the hash of the name of a QBAFArgument instance.
 * 
 * @param self the QBAFArgument object
 * @return PyObject* a object with the hash
 */
static PyObject *
QBAFArgument_hashfunc(QBAFArgumentObject *self)
{
    return PyObject_Hash(self->name);
}

/**
 * @brief Return the string format of a QBAFArgument object
 * 
 * @param self the QBAFArgument object
 * @param Py_UNUSED 
 * @return PyObject* the string representing the object
 */
static PyObject *
QBAFArgument_str(QBAFArgumentObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromFormat("Arg('%S')", self->name);
}

/**
 * @brief Return the string format of a QBAFArgument object
 * 
 * @param self the QBAFArgument object
 * @param Py_UNUSED 
 * @return PyObject* the string representing the object
 */
static PyObject *
QBAFArgument_repr(QBAFArgumentObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromFormat("'%S'", self->name);
}

/**
 * @brief List of functions of the class QBAFArgument
 * 
 */
static PyMethodDef QBAFArgument_methods[] = {
    {NULL}  /* Sentinel */
};

/**
 * @brief Python definition for the class QBAFArgument
 * 
 */
static PyTypeObject QBAFArgumentType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "qbaf.QBAFArgument",
    .tp_doc = PyDoc_STR("QBAFArgument objects"),
    .tp_basicsize = sizeof(QBAFArgumentObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_new = QBAFArgument_new,
    .tp_init = (initproc) QBAFArgument_init,
    .tp_dealloc = (destructor) QBAFArgument_dealloc,
    .tp_traverse = (traverseproc) QBAFArgument_traverse,
    .tp_clear = (inquiry) QBAFArgument_clear,
    .tp_members = QBAFArgument_members,
    .tp_methods = QBAFArgument_methods,
    .tp_getset = QBAFArgument_getsetters,
    .tp_str = (reprfunc) QBAFArgument_str,                      // __str__
    .tp_repr = (reprfunc) QBAFArgument_repr,                    // __repr__
    .tp_richcompare = (richcmpfunc) QBAFArgument_richcompare,   // __lt__, __le__, __eq__, __ne__, __gt__, __ge__
    .tp_hash = (hashfunc) QBAFArgument_hashfunc,                // __hash__
};

/**
 * @brief Get the QBAFArgumentType object created above that defines the class QBAFArgument
 * 
 * @return PyTypeObject* a pointer to the QBAFArgument class definition
 */
PyTypeObject *get_QBAFArgumentType() {
    return &QBAFArgumentType;
}