#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

/**
 * @brief Struct that defines the Object Type Argument in a QBAF
 * 
 */
typedef struct {
    PyObject_HEAD
    PyObject *name;         /* name of the argument and identifier */
    PyObject *description;  /* description of the argument */
    double initial_weight;  /* initial weight */
    double final_weight;    /* final weight */
} QBAFArgumentObject;

// TODO
/**
 * @brief 
 * 
 * @param self 
 * @param visit 
 * @param arg 
 * @return int 
 */
static int
QBAFArgument_traverse(QBAFArgumentObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->name);
    Py_VISIT(self->description);
    return 0;
}

// TODO
/**
 * @brief 
 * 
 * @param self 
 * @return int 
 */
static int
QBAFArgument_clear(QBAFArgumentObject *self)
{
    Py_CLEAR(self->name);
    Py_CLEAR(self->description);
    return 0;
}

// TODO
/**
 * @brief 
 * 
 * @param self 
 */
static void
QBAFArgument_dealloc(QBAFArgumentObject *self)
{
    PyObject_GC_UnTrack(self);
    QBAFArgument_clear(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

// TODO
/**
 * @brief 
 * 
 * @param type 
 * @param args 
 * @param kwds 
 * @return PyObject* 
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
            Py_DECREF(self);
            return NULL;
        }
        self->initial_weight = 0;
        self->final_weight = 0;
    }
    return (PyObject *) self;
}

// TODO
/**
 * @brief 
 * 
 * @param self 
 * @param args 
 * @param kwds 
 * @return int 
 */
static int
QBAFArgument_init(QBAFArgumentObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", "description", "initial_weight", NULL};
    PyObject *name = NULL, *description = NULL, *tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|UUd", kwlist,
                                     &name, &description,
                                     &self->initial_weight))
        return -1;

    if (name) {
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

static PyMemberDef QBAFArgument_members[] = {
    {"initial_weight", T_DOUBLE, offsetof(QBAFArgumentObject, initial_weight), 0,
     "initial weight"},
    {"final_weight", T_DOUBLE, offsetof(QBAFArgumentObject, final_weight), 0,
     "final weight"},
    {NULL}  /* Sentinel */
};

static PyObject *
QBAFArgument_getname(QBAFArgumentObject *self, void *closure)
{
    Py_INCREF(self->name);
    return self->name;
}

static PyObject *
QBAFArgument_getdescription(QBAFArgumentObject *self, void *closure)
{
    Py_INCREF(self->description);
    return self->description;
}

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

static PyGetSetDef QBAFArgument_getsetters[] = {
    {"name", (getter) QBAFArgument_getname, NULL,
     "name and identifier", NULL},
    {"description", (getter) QBAFArgument_getdescription, (setter) QBAFArgument_setdescription,
     "argument description", NULL},
    {NULL}  /* Sentinel */
};

static PyObject *
QBAFArgument_str(QBAFArgumentObject *self, PyObject *Py_UNUSED(ignored))
{
    return PyUnicode_FromFormat("QBAFArgument(%S)", self->name);
}

static PyMethodDef QBAFArgument_methods[] = {
    {"str", (PyCFunction) QBAFArgument_str, METH_NOARGS,
     "Return the string format of the object: QBAFArgument(<name>)"
    },
    {NULL}  /* Sentinel */
};

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
    .tp_str = QBAFArgument_str,                 // __str__ function
};

PyTypeObject *get_QBAFArgumentType() {
    return &QBAFArgumentType;
}