/**
 * @file argument.h
 * @author Jose Ruiz Alarcon
 * @brief  Module that defines a function to obtain information about the QBAFArgument class.
 */

#ifndef _QBAF_ARGUMENT_H_
#define _QBAF_ARGUMENT_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/**
 * @brief Get the QBAFArgumentType object that defines the class QBAFArgument
 * 
 * @return PyTypeObject* a pointer to the QBAFArgument class definition
 */
PyTypeObject *get_QBAFArgumentType(void);

#endif