#ifndef LIBRANDOM_H
#define LIBRANDOM_H

#include <stdlib.h>
#include <unistd.h>

double randdouble(struct drand48_data *context);
int randint(struct drand48_data *context, int a, int b);
void randsleep(struct drand48_data *context, int a, int b);

#endif // LIBRANDOM_H
