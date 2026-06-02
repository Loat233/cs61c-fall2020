#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/

/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fieds of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix(matrix **mat, int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid dimensions");
        return -1;
    }

    *mat = (matrix*) malloc(sizeof(matrix));
    if (*mat == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate matrix");
        return -1;
    }

    double* flat_data = (double*) calloc(rows * cols, sizeof(double));
    if (flat_data == NULL) {
        free(*mat);
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate matrix");
        return -1;
    }

    (*mat)->data = (double**) calloc(rows, sizeof(double*));
    if ((*mat)->data == NULL) {
        free(flat_data);
        free(*mat);
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate matrix");
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        (*mat)->data[i] = flat_data + (i * cols);
    }

    (*mat)->rows = rows;
    (*mat)->cols = cols;
    (*mat)->is_1d = rows == 1 || cols == 1 ? 1 : 0;
    (*mat)->ref_cnt = 1;
    (*mat)->parent = NULL;
    return 0;
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset,
                        int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        PyErr_SetString(PyExc_ValueError, "Invalid dimensions");
        return -1;
    }

    *mat = (matrix*) malloc(sizeof(matrix));
    if (*mat == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate matrix");
        return -1;
    }

    (*mat)->data = (double**) malloc(rows * sizeof(double*));
    if ((*mat)->data == NULL) {
        free(*mat);
        PyErr_SetString(PyExc_RuntimeError, "Failed to allocate matrix");
        return -1;
    }

    for (int i = 0; i < rows; i++) {
        (*mat)->data[i] = from->data[row_offset + i] + col_offset;
    }

    (*mat)->rows = rows;
    (*mat)->cols = cols;
    (*mat)->is_1d = rows == 1 || cols == 1 ? 1 : 0;

    if (from->parent != NULL) {
        (*mat)->parent = from->parent;
    } else {
        (*mat)->parent = from;
    }

    (*mat)->parent->ref_cnt += 1;
    (*mat)->ref_cnt = 1;
    return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    if (mat == NULL) {
        return;
    }

    if (mat->parent != NULL) {
        mat->parent->ref_cnt -= 1;

        if (mat->parent->ref_cnt == 0) {
            free(mat->parent->data[0]);
            free(mat->parent->data);
            free(mat->parent);
        }
        free(mat->data);
        free(mat);
    } else {
        mat->ref_cnt -= 1;
        if (mat->ref_cnt == 0) {
            free(mat->data[0]);
            free(mat->data);
            free(mat);
        }
    }
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double get(matrix *mat, int row, int col) {
    return mat->data[row][col];
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void set(matrix *mat, int row, int col, double val) {
    mat->data[row][col] = val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
    if (mat->parent == NULL) {
        int total = mat->cols * mat->rows;
        double* flat_ptr = mat->data[0];
        __m256d val_vec = _mm256_set1_pd(val);

        #pragma omp parallel for default(none) shared(total, flat_ptr, val_vec)
        for (int i = 0; i < total / 4 * 4; i += 4) {
            _mm256_storeu_pd(flat_ptr + i, val_vec);
        }

        for (int i = total / 4 * 4; i < total; i++) {
            flat_ptr[i] = val;
        }
    }
    else {
        #pragma omp parallel for default(none) shared(mat, val)
        for (int i = 0; i < mat->rows; i++) {
            for (int j = 0; j < mat->cols; j++) {
                mat->data[i][j] = val;
            }
        }
    }
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        PyErr_SetString(PyExc_ValueError, "Matrix dimensions must match for addition");
        return -1;
    }

    if (mat1->parent == NULL && mat2->parent == NULL && result->parent == NULL) {
        int total = mat1->rows * mat1->cols;
        double* flat_res = result->data[0];
        double* flat_mat1 = mat1->data[0];
        double* flat_mat2 = mat2->data[0];

        #pragma omp parallel for default(none) shared(total, flat_res, flat_mat1, flat_mat2)
        for (int i = 0; i < total / 4 * 4; i += 4) {
            __m256d mat1_val = _mm256_loadu_pd(flat_mat1 + i);
            __m256d mat2_val = _mm256_loadu_pd(flat_mat2 + i);

            __m256d result_val = _mm256_add_pd(mat1_val, mat2_val);
            _mm256_storeu_pd(flat_res + i, result_val);
        }

        for (int i = total / 4 * 4; i < total; i++) {
            flat_res[i] = flat_mat1[i] + flat_mat2[i];
        }
    }
    else {
        #pragma omp parallel for default(none) shared(result, mat1, mat2)
        for (int x = 0; x < mat1->rows; x++) {
            for (int y = 0; y < mat1->cols; y++) {
                result->data[x][y] = mat1->data[x][y] + mat2->data[x][y];
            }
        }
    }
    return 0;
}

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        PyErr_SetString(PyExc_ValueError, "Matrix dimensions must match for subtraction");
        return -1;
    }

    if (mat1->parent == NULL && mat2->parent == NULL && result->parent == NULL) {
        int total = mat1->rows * mat1->cols;
        double* flat_res = result->data[0];
        double* flat_mat1 = mat1->data[0];
        double* flat_mat2 = mat2->data[0];

        #pragma omp parallel for default(none) shared(total, flat_res, flat_mat1, flat_mat2)
        for (int i = 0; i < total / 4 * 4; i += 4) {
            __m256d mat1_val = _mm256_loadu_pd(flat_mat1 + i);
            __m256d mat2_val = _mm256_loadu_pd(flat_mat2 + i);

            __m256d result_val = _mm256_sub_pd(mat1_val, mat2_val);
            _mm256_storeu_pd(flat_res + i, result_val);
        }

        for (int i = total / 4 * 4; i < total; i++) {
            flat_res[i] = flat_mat1[i] - flat_mat2[i];
        }
    }
    else {
        #pragma omp parallel for default(none) shared(result, mat1, mat2)
        for (int x = 0; x < mat1->rows; x++) {
            for (int y = 0; y < mat1->cols; y++) {
                result->data[x][y] = mat1->data[x][y] - mat2->data[x][y];
            }
        }
    }
    return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1->cols != mat2->rows) {
        PyErr_SetString(PyExc_ValueError, "mat1 cols must equal mat2 rows for multiplication");
        return -1;
    }
    fill_matrix(result, 0.0);


    if (mat1->parent == NULL && mat2->parent == NULL && result->parent == NULL) {
        #pragma omp parallel for default(none) shared(mat1, mat2, result)
        for (int i = 0; i < mat1->rows; i++) {
            for (int k = 0; k < mat1->cols; k++) {
                double mat1_ik = mat1->data[i][k];
                __m256d vec_mat1_ik = _mm256_set1_pd(mat1_ik);
                int j;
                for (j = 0; j < mat2->cols / 4 * 4; j += 4) {
                    __m256d vec_mat2 = _mm256_loadu_pd(mat2->data[k] + j);
                    __m256d vec_res = _mm256_loadu_pd(result->data[i] + j);

                    vec_res = _mm256_fmadd_pd(vec_mat1_ik, vec_mat2, vec_res);
                    _mm256_storeu_pd(result->data[i] + j, vec_res);
                }

                for (; j < mat2->cols; j++) {
                    result->data[i][j] += mat1_ik * mat2->data[k][j];
                }
            }
        }
    }
    else {
        #pragma omp parallel for default(none) shared(mat1, mat2, result)
        for (int i = 0; i < mat1->rows; i++) {
            for (int k = 0; k < mat1->cols; k++) {
                double mat1_ik = mat1->data[i][k];
                for (int j = 0; j < mat2->cols; j++) {
                    result->data[i][j] += mat1_ik * mat2->data[k][j];
                }
            }
        }
    }
    return 0;
}

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise multiplication.
 */
int pow_matrix(matrix *result, matrix *mat, int pow) {
    if (mat->rows != mat->cols) {
        PyErr_SetString(PyExc_ValueError, "Matrix must be square for exponentiation");
        return -1;
    }
    if (pow < 0) {
        PyErr_SetString(PyExc_ValueError, "Power cannot be negative");
        return -1;
    }

    fill_matrix(result, 0.0);
    #pragma omp parallel for default(none) shared(result)
    for (int i = 0; i < result->rows; i++) {
        result->data[i][i] = 1.0;
    }

    if (pow == 0) {
        return 0;
    }

    matrix *temp = NULL;
    matrix *base_mat = NULL;
    if (allocate_matrix(&temp, mat->rows, mat->cols) != 0) {
        return -1;
    }
    if (allocate_matrix(&base_mat, mat->rows, mat->cols) != 0) {
        deallocate_matrix(temp);
        return -1;
    }

    #pragma omp parallel for default(none) shared(base_mat, mat)
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            base_mat->data[i][j] = mat->data[i][j];
        }
    }

    int current_pow = pow;
    while (current_pow > 0) {
        if (current_pow % 2 == 1) {
            mul_matrix(temp, result, base_mat);

            #pragma omp parallel for default(none) shared(result, temp)
            for (int i = 0; i < result->rows; i++) {
                for (int j = 0; j < result->cols; j++) {
                    result->data[i][j] = temp->data[i][j];
                }
            }
        }
        current_pow /= 2;
        if (current_pow == 0) {
            break;
        }

        // base_mat = base_mat * base_mat
        mul_matrix(temp, base_mat, base_mat);

        #pragma omp parallel for default(none) shared(base_mat, temp)
        for (int i = 0; i < base_mat->rows; i++) {
            for (int j = 0; j < base_mat->cols; j++) {
                base_mat->data[i][j] = temp->data[i][j];
            }
        }
    }

    deallocate_matrix(temp);
    deallocate_matrix(base_mat);

    return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    if (mat->parent == NULL && result->parent == NULL) {
        int total = mat->cols * mat->rows;
        double* flat_res = result->data[0];
        double* flat_mat = mat->data[0];

        __m256d vec_zero = _mm256_setzero_pd();

        #pragma omp parallel for default(none) shared(flat_mat, flat_res, vec_zero, total)
        for (int i = 0; i < total / 4 * 4; i += 4) {
            __m256d vec_mat = _mm256_loadu_pd(flat_mat + i);
            __m256d vec_res = _mm256_sub_pd(vec_zero, vec_mat);

            _mm256_storeu_pd(flat_res + i, vec_res);
        }

        for (int i = total / 4 * 4; i < total; i++) {
            flat_res[i] = -flat_mat[i];
        }
    }
    else {
        #pragma omp parallel for default(none) shared(mat, result)
        for (int x = 0; x < mat->rows; x++) {
            for (int y = 0; y < mat->cols; y++) {
                result->data[x][y] = -mat->data[x][y];
            }
        }
    }
    return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    if (mat->parent == NULL && result->parent == NULL) {
        int total = mat->cols * mat->rows;
        double* flat_res = result->data[0];
        double* flat_mat = mat->data[0];

        union {
            uint64_t i;
            double d;
        } mask_union;
        mask_union.i = 0x7FFFFFFFFFFFFFFF;
        __m256d mask = _mm256_set1_pd(mask_union.d);

        #pragma omp parallel for default(none) shared(flat_mat, flat_res, mask, total)
        for (int i = 0; i < total / 4 * 4; i += 4){
            __m256d vec_mat = _mm256_loadu_pd(flat_mat + i);
            __m256d vec_res = _mm256_and_pd(vec_mat, mask);

            _mm256_storeu_pd(flat_res + i, vec_res);
        }

        for (int i = total / 4 * 4; i < total; i++) {
            flat_res[i] = fabs(flat_mat[i]);
        }
    }
    else {
        #pragma omp parallel for default(none) shared(mat, result)
        for (int x = 0; x < mat->rows; x++) {
            for (int y = 0; y < mat->cols; y++) {
                result->data[x][y] = fabs(mat->data[x][y]);
            }
        }
    }
    return 0;
}

