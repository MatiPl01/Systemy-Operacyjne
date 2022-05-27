#include "librandom.h"


double randdouble(struct drand48_data *context) {
    double result;
    drand48_r(context, &result);
    return result;
}

int randint(struct drand48_data *context, int a, int b) {
    return (int) (a + (b - a) * randdouble(context));
}

void randsleep(struct drand48_data *context, int a, int b) {
    sleep(randint(context, a, b));
}
