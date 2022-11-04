/**
 * @file qbaf_module.h
 * @author Jose Ruiz Alarcon
 * @brief  Module that defines functions to obtain the definitions of QBAF classes.
 */

#ifndef _QBAF_MODULE_H_
#define _QBAF_MODULE_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/**
 * @brief Get the QBAFArgumentType object that defines the class QBAFArgument
 * 
 * @return PyTypeObject* a pointer to the QBAFArgument class definition
 */
PyTypeObject *get_QBAFArgumentType(void);

/**
 * @brief Get the QBAFARelationsType object that defines the class QBAFARelations
 * 
 * @return PyTypeObject* a pointer to the QBAFARelations class definition
 */
PyTypeObject *get_QBAFARelationsType(void);

#endif