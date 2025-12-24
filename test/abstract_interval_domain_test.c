#include "../src/domain/abstract_interval_domain.c"
#include <assert.h>
#include <stdio.h>

void interval_create_test(void) {
    /* m,n integers */
    int64_t m = -10;
    int64_t n = 10;
    abstract_int_set_params(m, n);
    Interval i = {0};

    i = interval_create(3, 2);
    assert(i.type == INTERVAL_BOTTOM);

    i = interval_create(INTERVAL_MIN_INF, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(2, 2);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 2);

    i = interval_create(-11, -11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == -11);

    i = interval_create(11, 11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 11);

    i = interval_create(INTERVAL_MIN_INF, 9);
    assert(i.type == INTERVAL_STD);

    i = interval_create(9, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(-6, 7);
    assert(i.type == INTERVAL_STD);

    i = interval_create(-20, 7);
    assert(i.type == INTERVAL_STD && i.a == INTERVAL_MIN_INF && i.b == INTERVAL_PLUS_INF);

    /* m,n infinite */
    m = INTERVAL_MIN_INF;
    n = INTERVAL_PLUS_INF;
    abstract_int_set_params(m, n);

    i = interval_create(3, 2);
    assert(i.type == INTERVAL_BOTTOM);

    i = interval_create(INTERVAL_MIN_INF, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(2, 2);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 2);

    i = interval_create(-11, -11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == -11);

    i = interval_create(11, 11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 11);

    i = interval_create(INTERVAL_MIN_INF, 9);
    assert(i.type == INTERVAL_STD);

    i = interval_create(9, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(-6, 7);
    assert(i.type == INTERVAL_STD);
}

void interval_leq_test(void) {
    /* STD intervals */
    Interval i1 = {
        .type = INTERVAL_STD,
        .a = 10,
        .b = 12,
    };

    Interval i2 = {
        .type = INTERVAL_STD,
        .a = 10,
        .b = 20,
    };

    assert(interval_leq(i1, i2) == true);

    /* i1 and i2 BOTTOM */
    i1.type = INTERVAL_BOTTOM;
    i2.type = INTERVAL_BOTTOM;
    assert(interval_leq(i1, i2));

    /* i1 BOTTOM */
    i1.type = INTERVAL_BOTTOM;
    i2 = (Interval) {
        .type = INTERVAL_STD,
        .a = 10,
        .b = 20,
    };
    assert(interval_leq(i1, i2) == true);

    /* i2 BOTTOM */
    i1 = (Interval) {
        .type = INTERVAL_STD,
        .a = 10,
        .b = 20,
    };
    i2.type = INTERVAL_BOTTOM;
    assert(interval_leq(i1, i2) == false);
}

int main(void) {
    interval_leq_test();
    printf("[OK]: interval_leq_test pass.\n");
    interval_create_test();
    printf("[OK]: interval_create_test pass.\n");
}
