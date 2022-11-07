/**
 * @file qbaf_module.c
 * @author Jose Ruiz Alarcon
 * @brief Module to crate a QBAFramework with the python C api.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "qbaf_module.h"

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
    // A pointer to the class description of QBAFArgument
    PyTypeObject *QBAFArgumentType = get_QBAFArgumentType();

    // A pointer to the class description of QBAFARelations
    PyTypeObject *QBAFARelationsType = get_QBAFARelationsType();

    // A pointer to the class description of QBAFramework
    PyTypeObject *QBAFrameworkType = get_QBAFrameworkType();

    if (PyType_Ready(QBAFArgumentType) < 0)
        return NULL;

    if (PyType_Ready(QBAFARelationsType) < 0)
        return NULL;

    if (PyType_Ready(QBAFrameworkType) < 0)
        return NULL;

    PyObject *m = PyModule_Create(&QBAFmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(QBAFArgumentType);
    if (PyModule_AddObject(m, "QBAFArgument", (PyObject *) QBAFArgumentType) < 0) {
        Py_DECREF(QBAFArgumentType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(QBAFARelationsType);
    if (PyModule_AddObject(m, "QBAFARelations", (PyObject *) QBAFARelationsType) < 0) {
        Py_DECREF(QBAFARelationsType);
        Py_DECREF(QBAFArgumentType);
        Py_DECREF(m);
        return NULL;
    }
    
    Py_INCREF(QBAFrameworkType);
    if (PyModule_AddObject(m, "QBAFramework", (PyObject *) QBAFrameworkType) < 0) {
        Py_DECREF(QBAFrameworkType);
        Py_DECREF(QBAFARelationsType);
        Py_DECREF(QBAFArgumentType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}