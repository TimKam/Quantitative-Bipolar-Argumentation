#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "argument.h"

static PyModuleDef QBAFmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "qbaf",
    .m_doc = "Module that creates a QBAFArgument type.",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_qbaf(void)
{

    PyTypeObject *QBAFArgumentType = get_QBAFArgumentType();

    PyObject *m;
    if (PyType_Ready(QBAFArgumentType) < 0)
        return NULL;

    m = PyModule_Create(&QBAFmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(QBAFArgumentType);
    if (PyModule_AddObject(m, "QBAFArgument", (PyObject *) QBAFArgumentType) < 0) {
        Py_DECREF(QBAFArgumentType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}