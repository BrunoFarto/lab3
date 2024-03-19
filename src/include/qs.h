#ifndef QS_H
#define QS_H

#include <stdlib.h>

void swap (void *a, void *b, size_t width);

void qsort(void *base, size_t nel, size_t width,
           int (*compar)(const void *, const void *));
         
#endif /* QS_H */