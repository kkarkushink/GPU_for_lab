// float, т.к. встроенная видеокарта не тянет double (cl_khr_fp64)
// один запуск на столбец col, строки ниже диагонали независимы
// count может быть меньше глобального размера - лишние просто выходят
__kernel void eliminate_rows(__global float *a, int n, int col, int count)
{
    int local_row = get_global_id(0);
    if (local_row >= count)
        return;

    int row = col + 1 + local_row;
    float pivot = a[col * n + col];
    float factor = a[row * n + col] / pivot;

    for (int k = col; k < n; k++) {
        a[row * n + k] -= factor * a[col * n + k];
    }
}
