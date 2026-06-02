#include "numc.h"
#include <structmember.h>

PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Right operand must be a numc.Matrix");
        return NULL;
    }

    Matrix61c* other = (Matrix61c*) args;

    if (self->mat->rows != other->mat->rows || self->mat->cols != other->mat->cols) {
        PyErr_SetString(PyExc_ValueError, "Matrices dimensions must match for addition");
        return NULL;
    }

    matrix* result_mat = NULL;
    if (allocate_matrix(&result_mat, self->mat->rows, self->mat->cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate memory");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    if (result == NULL) {
        return NULL;
    }

    result->mat = result_mat;
    result->shape = get_shape(self->mat->rows, self->mat->cols);

    if (add_matrix(result->mat, self->mat, other->mat) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to add matrices");
        return NULL;
    }

    return (PyObject*) result;
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Right operand must be a numc.Matrix");
        return NULL;
    }

    Matrix61c* other = (Matrix61c*) args;

    if (self->mat->rows != other->mat->rows || self->mat->cols != other->mat->cols) {
        PyErr_SetString(PyExc_ValueError, "Matrices dimensions must match for subtraction");
        return NULL;
    }

    matrix* result_mat = NULL;
    if (allocate_matrix(&result_mat, self->mat->rows, self->mat->cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate memory");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    if (result == NULL) {
        return NULL;
    }

    result->mat = result_mat;
    result->shape = get_shape(self->mat->rows, self->mat->cols);

    if (sub_matrix(result->mat, self->mat, other->mat) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to subtract matrices");
        return NULL;
    }

    return (PyObject*) result;
}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Right operand must be a numc.Matrix");
        return NULL;
    }

    Matrix61c* other = (Matrix61c*) args;

    if (self->mat->cols != other->mat->rows) {
        PyErr_SetString(PyExc_ValueError, "1st matrix's columns must match 2nd matrix's rows for multiplication");
        return NULL;
    }

    matrix* result_mat = NULL;
    if (allocate_matrix(&result_mat, self->mat->rows, other->mat->cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate memory");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    if (result == NULL) {
        return NULL;
    }

    result->mat = result_mat;
    result->shape = get_shape(self->mat->rows, other->mat->cols);

    if (mul_matrix(result->mat, self->mat, other->mat) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to multiple matrices");
        return NULL;
    }

    return (PyObject*) result;
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    matrix* res_mat = NULL;
    if (allocate_matrix(&res_mat, self->mat->rows, self->mat->cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate memory");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    if (result == NULL) {
        return NULL;
    }
    result->mat = res_mat;
    result->shape = get_shape(self->mat->rows, self->mat->cols);

    if (neg_matrix(result->mat, self->mat) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to negate matrices");
        return NULL;
    }

    return (PyObject*) result;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    matrix* res_mat = NULL;
    if (allocate_matrix(&res_mat, self->mat->rows, self->mat->cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate memory");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    if (result == NULL) {
        return NULL;
    }
    result->mat = res_mat;
    result->shape = get_shape(self->mat->rows, self->mat->cols);

    if (abs_matrix(result->mat, self->mat) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to Take the absolute value of matrices");
        return NULL;
    }

    return (PyObject*) result;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    if (!PyLong_Check(pow)) {
        PyErr_SetString(PyExc_TypeError, "Power must be an integer");
        return NULL;
    }

    int int_pow = (int) PyLong_AsLong(pow);

    if (int_pow < 0) {
        PyErr_SetString(PyExc_ValueError, "Power cannot be negative");
        return NULL;
    }

    if (self->mat->cols != self->mat->rows) {
        PyErr_SetString(PyExc_ValueError, "Matrix must be square to be raised to a power");
        return NULL;
    }

    matrix* result_mat = NULL;
    if (allocate_matrix(&result_mat, self->mat->rows, self->mat->cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate memory");
        return NULL;
    }

    Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
    if (result == NULL) {
        return NULL;
    }

    result->mat = result_mat;
    result->shape = get_shape(self->mat->rows, self->mat->cols);

    if (pow_matrix(result->mat, self->mat, int_pow) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to calculate matrix power");
        return NULL;
    }

    return (PyObject*) result;
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    .nb_add = (binaryfunc) Matrix61c_add,
    .nb_subtract = (binaryfunc) Matrix61c_sub,
    .nb_multiply = (binaryfunc) Matrix61c_multiply,
    .nb_negative = (unaryfunc) Matrix61c_neg,
    .nb_absolute = (unaryfunc) Matrix61c_abs,
    .nb_power = (ternaryfunc) Matrix61c_pow
};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    int i, j;
    double val;

    if (!PyArg_ParseTuple(args, "iid", &i, &j, &val)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments. Expected 2 ints and 1 float.");
        return NULL;
    }

    if (i < 0 || i >= self->mat->rows || j < 0 || j >= self->mat->cols) {
        PyErr_SetString(PyExc_IndexError, "Index out of range");
        return NULL;
    }

    set(self->mat, i, j, val);

    Py_RETURN_NONE;
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    int i, j;

    if (!PyArg_ParseTuple(args, "ii", &i, &j)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments. Expected 2 ints.");
        return NULL;
    }

    if (i < 0 || i >= self->mat->rows || j < 0 || j >= self->mat->cols) {
        PyErr_SetString(PyExc_IndexError, "Index out of range");
        return NULL;
    }

    return PyFloat_FromDouble(get(self->mat, i, j));
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    {"set", (PyCFunction)Matrix61c_set_value, METH_VARARGS, "Set a value at a specific row and column."},
    {"get", (PyCFunction)Matrix61c_get_value, METH_VARARGS, "Get a value at a specific row and column."},
    {NULL, NULL, 0, NULL}
};

/* INDEXING */

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    // int
    if (PyLong_Check(key)) {
        int index = (int) PyLong_AsLong(key);
        // 1d mat
        if (self->mat->is_1d) {
            double result;
            // 1 row mat
            if (self->mat->rows == 1) {
                if (index < 0 || index >= self->mat->cols) {
                    PyErr_SetString(PyExc_IndexError, "Index out of range");
                    return NULL;
                }
                result = get(self->mat, 0, index);
            }
            // 1 col mat
            else {
                if (index < 0 || index >= self->mat->rows) {
                    PyErr_SetString(PyExc_IndexError, "Index out of range");
                    return NULL;
                }
                result = get(self->mat, index, 0);
            }
            return PyFloat_FromDouble(result);
        }
        // 2d mat
        else {
            if (index < 0 || index >= self->mat->rows) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return NULL;
            }

            Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
            if (result == NULL) {
                return NULL;
            }

            if (allocate_matrix_ref(&result->mat, self->mat, index, 0, 1, self->mat->cols) != 0) {
                PyErr_SetString(PyExc_RuntimeError, "Failed to allocate matrix ref");
                return NULL;
            }

            result->shape = get_shape(1, self->mat->cols);

            return (PyObject*) result;
        }
    }

    // slice
    if (PySlice_Check(key)) {
        Py_ssize_t start, stop, step, slice_length;

        // 1d mat
        if (self->mat->is_1d) {
            // 1 row mat
            if (self->mat->rows == 1) {
                if (PySlice_GetIndicesEx(key, self->mat->cols, &start, &stop, &step, &slice_length) < 0) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return NULL;
                }
                if (step != 1 || slice_length < 1) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return NULL;
                }

                Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
                if (result == NULL) {
                    return NULL;
                }

                allocate_matrix_ref(&result->mat, self->mat, 0, (int)start, 1, (int)slice_length);

                result->shape = get_shape(1, (int)slice_length);

                return (PyObject*) result;
            }
            // 1 col mat
            else {
                if (PySlice_GetIndicesEx(key, self->mat->rows, &start, &stop, &step, &slice_length) < 0) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return NULL;
                }
                if (step != 1 || slice_length < 1) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return NULL;
                }

                Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
                if (result == NULL) {
                    return NULL;
                }

                allocate_matrix_ref(&result->mat, self->mat, (int)start, 0, (int)slice_length, 1);

                result->shape = get_shape((int)slice_length, 1);

                return (PyObject*) result;
            }
        }
        // 2d mat
        else {
            if (PySlice_GetIndicesEx(key, self->mat->rows, &start, &stop, &step, &slice_length) < 0) {
                PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                return NULL;
            }
            if (step != 1 || slice_length < 1) {
                PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                return NULL;
            }

            Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
            if (result == NULL) {
                return NULL;
            }

            allocate_matrix_ref(&result->mat, self->mat, (int)start, 0, (int)slice_length, self->mat->cols);

            result->shape = get_shape((int)slice_length, self->mat->cols);

            return (PyObject*) result;
        }
    }

    // tuple
    if (PyTuple_Check(key)) {
        if (self->mat->is_1d) {
            PyErr_SetString(PyExc_TypeError, "1D matrices only support single slice!");
            return NULL;
        }

        if (PyTuple_Size(key) != 2) {
            PyErr_SetString(PyExc_TypeError, "Tuple must have size 2");
            return NULL;
        }

        PyObject* row_key = PyTuple_GetItem(key, 0);
        PyObject* col_key = PyTuple_GetItem(key, 1);

        Py_ssize_t r_start, r_stop, r_step, r_slice_length;
        Py_ssize_t c_start, c_stop, c_step, c_slice_length;

        // int_row
        if (PyLong_Check(row_key)) {
            r_start = (int) PyLong_AsLong(row_key);
            r_slice_length = 1;

            if (r_start < 0 || r_start >= self->mat->rows) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return NULL;
            }
        }
        // slice_row
        else if (PySlice_Check(row_key)) {
            if (PySlice_GetIndicesEx(row_key, self->mat->rows, &r_start, &r_stop, &r_step, &r_slice_length) < 0) {
                PyErr_SetString(PyExc_ValueError, "Invalid row key");
                return NULL;
            }

            if (r_step != 1 || r_slice_length < 1) {
                PyErr_SetString(PyExc_ValueError, "Invalid row key");
                return NULL;
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "Invalid row key");
            return NULL;
        }

        // int_col
        if (PyLong_Check(col_key)) {
            c_start = (int) PyLong_AsLong(col_key);
            c_slice_length = 1;

            if (c_start < 0 || c_start >= self->mat->cols) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return NULL;
            }
        }
        // slice_col
        else if (PySlice_Check(col_key)) {
            if (PySlice_GetIndicesEx(col_key, self->mat->cols, &c_start, &c_stop, &c_step, &c_slice_length) < 0) {
                PyErr_SetString(PyExc_ValueError, "Invalid col key");
                return NULL;
            }

            if (c_step != 1 || c_slice_length < 1) {
                PyErr_SetString(PyExc_ValueError, "Invalid col key");
                return NULL;
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "Invalid col key");
            return NULL;
        }

        //  get result
        if (PyLong_Check(row_key) && PyLong_Check(col_key)) {
            double result = get(self->mat, (int)r_start, (int)c_start);
            return PyFloat_FromDouble(result);
        }
        else {
            Matrix61c* result = (Matrix61c*) Matrix61c_new(&Matrix61cType, NULL, NULL);
            if (result == NULL) {
                return NULL;
            }

            allocate_matrix_ref(&result->mat, self->mat, (int)r_start, (int)c_start, (int)r_slice_length, (int)c_slice_length);

            result->shape = get_shape((int)r_slice_length, (int)c_slice_length);

            return (PyObject*) result;
        }
    }
    else {
        PyErr_SetString(PyExc_TypeError, "Invalid key type");
        return NULL;
    }
}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject *key, PyObject *v) {
    Py_ssize_t r_start, r_stop, r_step, r_slice_length;
    Py_ssize_t c_start, c_stop, c_step, c_slice_length;
    int sub_is2d = 0;
    // int
    if (PyLong_Check(key)) {
        int index = (int) PyLong_AsLong(key);
        // 1d mat
        if (self->mat->is_1d) {
            // 1 row mat
            if (self->mat->rows == 1) {
                if (index < 0 || index >= self->mat->cols) {
                    PyErr_SetString(PyExc_IndexError, "Index out of range");
                    return -1;
                }
                r_start = 0;
                c_start = index;
            }
            // 1 col mat
            else {
                if (index < 0 || index >= self->mat->rows) {
                    PyErr_SetString(PyExc_IndexError, "Index out of range");
                    return -1;
                }
                r_start = index;
                c_start = 0;
            }

            if (!PyFloat_Check(v) && !PyLong_Check(v)) {
                PyErr_SetString(PyExc_TypeError, "v must be a float or int");
                return -1;
            }

            double val = PyFloat_Check(v) ? PyFloat_AsDouble(v) : (double) PyLong_AsLong(v);
            set(self->mat, (int)r_start, (int)c_start, val);
            return 0;
        }
        // 2d mat
        else {
            if (index < 0 || index >= self->mat->rows) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return -1;
            }
            r_start = index;
            c_start = 0;

            if (!PyList_Check(v)) {
                PyErr_SetString(PyExc_TypeError, "v must be a list");
                return -1;
            }

            if (PyList_Size(v) != self->mat->cols) {
                PyErr_SetString(PyExc_ValueError, "List length does not match slice length");
                return -1;
            }

            for (int i = 0; i < self->mat->cols; i++) {
                PyObject* item = PyList_GetItem(v, i);
                if (!PyFloat_Check(item) && !PyLong_Check(item)) {
                    PyErr_SetString(PyExc_ValueError, "List items must be numbers");
                    return -1;
                }
                double val = PyFloat_Check(item) ? PyFloat_AsDouble(item) : (double) PyLong_AsLong(item);
                set(self->mat, (int)r_start, (int)c_start + i, val);
            }
            return 0;
        }
    }

    // slice
    else if (PySlice_Check(key)) {
        // 1d mat
        if (self->mat->is_1d) {
            // 1 row mat
            if (self->mat->rows == 1) {
                if (PySlice_GetIndicesEx(key, self->mat->cols, &c_start, &c_stop, &c_step, &c_slice_length) < 0) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return -1;
                }
                if (c_step != 1 || c_slice_length < 1) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return -1;
                }
                r_start = 0;

                if (!PyList_Check(v)) {
                    PyErr_SetString(PyExc_TypeError, "v must be a list");
                    return -1;
                }

                if (PyList_Size(v) != c_slice_length) {
                    PyErr_SetString(PyExc_ValueError, "List length does not match slice length");
                    return -1;
                }

                for (int i = 0; i < c_slice_length; i++) {
                    PyObject* item = PyList_GetItem(v, i);
                    if (!PyFloat_Check(item) && !PyLong_Check(item)) {
                        PyErr_SetString(PyExc_ValueError, "List items must be numbers");
                        return -1;
                    }
                    double val = PyFloat_Check(item) ? PyFloat_AsDouble(item) : (double) PyLong_AsLong(item);
                    set(self->mat, (int)r_start, (int)c_start + i, val);
                }
                return 0;
            }
            // 1 col mat
            else {
                if (PySlice_GetIndicesEx(key, self->mat->rows, &r_start, &r_stop, &r_step, &r_slice_length) < 0) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return -1;
                }
                if (r_step != 1 || r_slice_length < 1) {
                    PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                    return -1;
                }
                c_start = 0;

                if (!PyList_Check(v)) {
                    PyErr_SetString(PyExc_TypeError, "v must be a list");
                    return -1;
                }

                if (PyList_Size(v) != r_slice_length) {
                    PyErr_SetString(PyExc_ValueError, "List length does not match slice length");
                    return -1;
                }

                for (int i = 0; i < r_slice_length; i++) {
                    PyObject* item = PyList_GetItem(v, i);
                    if (!PyFloat_Check(item) && !PyLong_Check(item)) {
                        PyErr_SetString(PyExc_ValueError, "List items must be numbers");
                        return -1;
                    }
                    double val = PyFloat_Check(item) ? PyFloat_AsDouble(item) : (double) PyLong_AsLong(item);
                    set(self->mat, (int)r_start + i, (int)c_start, val);
                }
                return 0;
            }
        }
        else {
            if (PySlice_GetIndicesEx(key, self->mat->rows, &r_start, &r_stop, &r_step, &r_slice_length) < 0) {
                PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                return -1;
            }
            if (r_step != 1 || r_slice_length < 1) {
                PyErr_SetString(PyExc_ValueError, "Slice info not valid!");
                return -1;
            }
            sub_is2d = 1;
            c_start = 0;
            c_slice_length = self->mat->cols;
        }
    }

    // tuple
    else if (PyTuple_Check(key)) {
        if (self->mat->is_1d) {
            PyErr_SetString(PyExc_TypeError, "1D matrices only support single slice!");
            return -1;
        }

        if (PyTuple_Size(key) != 2) {
            PyErr_SetString(PyExc_TypeError, "Tuple must have size 2");
            return -1;
        }

        PyObject* row_key = PyTuple_GetItem(key, 0);
        PyObject* col_key = PyTuple_GetItem(key, 1);

        // int_row
        if (PyLong_Check(row_key)) {
            r_start = (int) PyLong_AsLong(row_key);
            r_slice_length = 1;

            if (r_start < 0 || r_start >= self->mat->rows) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return -1;
            }
        }
        // slice_row
        else if (PySlice_Check(row_key)) {
            if (PySlice_GetIndicesEx(row_key, self->mat->rows, &r_start, &r_stop, &r_step, &r_slice_length) < 0) {
                PyErr_SetString(PyExc_ValueError, "Invalid row key");
                return -1;
            }

            if (r_step != 1 || r_slice_length < 1) {
                PyErr_SetString(PyExc_ValueError, "Invalid row key");
                return -1;
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "Invalid row key");
            return -1;
        }

        // int_col
        if (PyLong_Check(col_key)) {
            c_start = (int) PyLong_AsLong(col_key);
            c_slice_length = 1;

            if (c_start < 0 || c_start >= self->mat->cols) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return -1;
            }
        }
        // slice_col
        else if (PySlice_Check(col_key)) {
            if (PySlice_GetIndicesEx(col_key, self->mat->cols, &c_start, &c_stop, &c_step, &c_slice_length) < 0) {
                PyErr_SetString(PyExc_ValueError, "Invalid col key");
                return -1;
            }

            if (c_step != 1 || c_slice_length < 1) {
                PyErr_SetString(PyExc_ValueError, "Invalid col key");
                return -1;
            }
        }
        else {
            PyErr_SetString(PyExc_TypeError, "Invalid col key");
            return -1;
        }

        if (PyLong_Check(row_key) && PyLong_Check(col_key)) {
            if (!PyFloat_Check(v) && !PyLong_Check(v)) {
                PyErr_SetString(PyExc_TypeError, "v must be a float or int");
                return -1;
            }

            double val = PyFloat_Check(v) ? PyFloat_AsDouble(v) : (double) PyLong_AsLong(v);
            set(self->mat, (int)r_start, (int)c_start, val);
            return 0;
        }
        else {
            sub_is2d = 1;
        }
    }

    // set 2d_mat
    if (sub_is2d) {
        if (!PyList_Check(v)) {
            PyErr_SetString(PyExc_TypeError, "v must be a list");
            return -1;
        }

        if (PyList_Size(v) != r_slice_length) {
            PyErr_SetString(PyExc_ValueError, "Row length does not match slice length");
            return -1;
        }

        for (int i = 0; i < r_slice_length; i++) {
            PyObject* row_list = PyList_GetItem(v, i);

            if (!PyList_Check(row_list)) {
                PyErr_SetString(PyExc_TypeError, "v must be a 2D list");
                return -1;
            }

            if (PyList_Size(row_list) != c_slice_length) {
                PyErr_SetString(PyExc_ValueError, "Column length does not match slice length");
                return -1;
            }

            for (int j = 0; j < c_slice_length; j++) {
                PyObject* item = PyList_GetItem(row_list, j);
                if (!PyFloat_Check(item) && !PyLong_Check(item)) {
                    PyErr_SetString(PyExc_ValueError, "List items must be numbers");
                    return -1;
                }
                double val = PyFloat_Check(item) ? PyFloat_AsDouble(item) : (double) PyLong_AsLong(item);

                // 写入矩阵
                set(self->mat, (int)r_start + i, (int)c_start + j, val);
            }
        }
        return 0;
    }

    // Invalid key
    PyErr_SetString(PyExc_TypeError, "Invalid key type");
    return -1;
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}