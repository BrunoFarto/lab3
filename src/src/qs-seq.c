#include "qs.h"

size_t partition(void *b, long l, long r, size_t w, int (*c)(const void *, const void *)) {
    long i = l-1;
    for (size_t j = l;  j < r;  j++) {
        if (c((char*)b+j*w, (char*)b+r*w) == -1) {
            i++;
            swap ((char*)b+i*w, (char*)b+j*w, w);
        }
    }
    i++;
    swap ((char*)b+i*w, (char*)b+r*w, w);
    return i;
}

void qsort_seq(void *b, long l, long r, size_t w, int (*c)(const void *, const void *)) {
    if (l < r) {
        size_t q = partition(b, l, r, w, c);
        qsort_seq(b, l, q-1, w, c);
        qsort_seq(b, q+1, r, w, c);
    }
}


void qsort(void *base, size_t nel, size_t width, int (*compar)(const void *, const void *)) {
    qsort_seq(base, 0, nel-1, width, compar);
}

