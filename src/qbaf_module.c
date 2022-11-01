/**
 * @file qbaf_module.c
 * @author Jose Ruiz Alarcon
 * @brief Module to crate a QBAFramework with the python C api.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "argument.h"

/**
 * @brief Definition of the module QBAF
 * 
 */
static PyModuleDef QBAFmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "qbaf",
    .m_doc = "Module that creates a QBAFArgument type.",
    .m_size = -1,
};

/**
 * @brief Creates a module that implements the QBAFArgument type.
 * 
 * @return PyMODINIT_FUNC is just python idiom to create a module
 */
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