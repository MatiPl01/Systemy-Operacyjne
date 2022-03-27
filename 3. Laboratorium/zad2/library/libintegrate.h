#ifndef LIBINTEGRATE_H
#define LIBINTEGRATE_H

typedef long double ld;

ld integrate(ld (*f)(ld), ld a, ld b, ld step);
ld integrate_async(ld (*f)(ld), ld a, ld b, ld step, unsigned no_processes);

#endif // LIBINTEGRATE_H
