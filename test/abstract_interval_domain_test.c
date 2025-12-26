#include "../src/domain/abstract_interval_domain.c"
#include <assert.h>
#include <stdio.h>

void interval_create_test(void) {
    /* m,n integers */
    int64_t m = -10;
    int64_t n = 10;
    Abstract_Interval_Ctx *ctx = abstract_interval_ctx_init(m, n, NULL, 0);
    Interval i = {0};

    i = interval_create(ctx, 3, 2);
    assert(i.type == INTERVAL_BOTTOM);

    i = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, 2, 2);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 2);

    i = interval_create(ctx, -11, -11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == -11);

    i = interval_create(ctx, 100, 100);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 100);

    i = interval_create(ctx, INTERVAL_MIN_INF, 9);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, 9, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, -6, 7);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, -20, 7);
    assert(i.type == INTERVAL_STD && i.a == INTERVAL_MIN_INF && i.b == INTERVAL_PLUS_INF);

    abstract_interval_ctx_free(ctx);

    /* m,n infinite */
    m = INTERVAL_MIN_INF;
    n = INTERVAL_PLUS_INF;
    ctx = abstract_interval_ctx_init(m, n, NULL, 0);

    i = interval_create(ctx, 3, 2);
    assert(i.type == INTERVAL_BOTTOM);

    i = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, 2, 2);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 2);

    i = interval_create(ctx, -11, -11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == -11);

    i = interval_create(ctx, 11, 11);
    assert(i.type == INTERVAL_STD && i.a == i.b && i.a == 11);

    i = interval_create(ctx, INTERVAL_MIN_INF, 9);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, 9, INTERVAL_PLUS_INF);
    assert(i.type == INTERVAL_STD);

    i = interval_create(ctx, -6, 7);
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

void interval_union_test(void) {
    /* m,n integers */
    int64_t m = -10;
    int64_t n = 10;
    Abstract_Interval_Ctx *ctx = abstract_interval_ctx_init(m, n, NULL, 0);
    Interval i1 = {0};
    Interval i2 = {0};
    Interval i_union = {0};

    i1 = interval_create(ctx, 1, -1); /* Bottom */
    i2 = interval_create(ctx, 1, -1); /* Bottom */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_BOTTOM);

    i1 = interval_create(ctx, 1, -1); /* Bottom */
    i2 = interval_create(ctx, 0, 2);  /* [0, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i2.a && i_union.b == i2.b);

    i1 = interval_create(ctx, 0, 2);  /* [0, 2] */
    i2 = interval_create(ctx, 1, -1); /* Bottom */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i1.b);

    i1 = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF); /* Top */
    i2 = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF); /* Top */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i1.b);

    i1 = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF); /* Top */
    i2 = interval_create(ctx, 1, -1); /* Bottom */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i1.b);

    i1 = interval_create(ctx, 1, -1); /* Bottom */
    i2 = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF); /* Top */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i2.a && i_union.b == i2.b);

    i1 = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF); /* Top */
    i2 = interval_create(ctx, 0, 2);  /* [0, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i1.b);

    i1 = interval_create(ctx, 0, 2);  /* [0, 2] */
    i2 = interval_create(ctx, INTERVAL_MIN_INF, INTERVAL_PLUS_INF); /* Top */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i2.a && i_union.b == i2.b);

    i1 = interval_create(ctx, -2, 0);  /* [-2, 0] */
    i2 = interval_create(ctx, 1, 2);  /* [1, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i2.b);

    i1 = interval_create(ctx, -1, 0);  /* [-2, 0] */
    i2 = interval_create(ctx, 1, 2);  /* [1, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i2.b);

    i1 = interval_create(ctx, 1, 2);  /* [1, 2] */
    i2 = interval_create(ctx, -2, 0);  /* [-2, 0] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i2.a && i_union.b == i1.b);

    i1 = interval_create(ctx, INTERVAL_MIN_INF, 5); /* (-INF, 5] */
    i2 = interval_create(ctx, 1, 2); /* [1, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i1.b);

    i1 = interval_create(ctx, INTERVAL_MIN_INF, 5); /* (-INF, 5] */
    i2 = interval_create(ctx, 1, 8); /* [1, 8] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i2.b);

    i1 = interval_create(ctx, -1, INTERVAL_PLUS_INF); /* [-1, +INF) */
    i2 = interval_create(ctx, 1, 2); /* [1, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i1.a && i_union.b == i1.b);

    i1 = interval_create(ctx, -1, INTERVAL_PLUS_INF); /* [-1, +INF) */
    i2 = interval_create(ctx, -5, 8); /* [-5, 8] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i2.a && i_union.b == i1.b);

    i1 = interval_create(ctx, 100, 100);  /* [100, 100] */
    i2 = interval_create(ctx, 1, 2);      /* [1, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == i2.a && i_union.b == INTERVAL_PLUS_INF);

    i1 = interval_create(ctx, -100, -100);  /* [-100, -100] */
    i2 = interval_create(ctx, 1, 2);        /* [1, 2] */
    i_union = interval_union(ctx, i1, i2);
    assert(i_union.type == INTERVAL_STD && i_union.a == INTERVAL_MIN_INF && i_union.b == i2.b);

/* TODO: Test
    { (-INF, k] | k ∈ [m, n] }
    { [k, +INF) | k ∈ [m, n] } */
}

int main(void) {
    interval_leq_test();
    printf("[OK]: interval_leq_test pass.\n");
    interval_create_test();
    printf("[OK]: interval_create_test pass.\n");
    interval_union_test();
    printf("[OK]: interval_union_test, test pass.\n");
}
