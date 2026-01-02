#include "include/abstract_analyzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

void print_help(char **argv) {
    fprintf(stderr, "Usage: %s COMMAND [ARGS]\n\n", argv[0]);

    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  cfg             Control Flow Graph utils.\n");
    fprintf(stderr, "  analyze         Static analysis.\n\n");

    fprintf(stderr, "Run '%s COMMAND' for more information on a specific command.\n", argv[0]);
}

void print_help_cfg(char **argv) {
    fprintf(stderr, "Usage: %s cfg SOURCE\n\n", argv[0]);
    fprintf(stderr, "Dump the Control Flow Graph of the SOURCE (Graphviz format).\n\n");

    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  SOURCE          Path to the source file (While language).\n");
}

void print_help_analyze(char **argv) {
    fprintf(stderr, "Usage: %s analyze DOMAIN [ARGS]\n\n", argv[0]);
    fprintf(stderr, "Run the static analysis on the SOURCE using the selected abstract domain.\n\n");

    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  DOMAIN          Abstract domain (available domains are listed below).\n\n");

    fprintf(stderr, "Domains:\n");
    fprintf(stderr, "  pinterval       Parametric Interval domain.\n\n");

    fprintf(stderr, "Run '%s analyze DOMAIN' for more information on a specific domain.\n", argv[0]);
}

void print_help_analyze_pinterval(char **argv) {
    fprintf(stderr, "Usage: %s analyze p_interval SOURCE [OPTIONS]\n\n", argv[0]);
    fprintf(stderr, "Given m,n ∈ (Z union {-INF, +INF}),\n");
    fprintf(stderr, "the abstract domain of parametric interval Int(m,n) is defined as the union of this sets:\n");
    fprintf(stderr, "  { BOTTOM, TOP }\n");
    fprintf(stderr, "  { [k,k] | k ∈ Z }\n");
    fprintf(stderr, "  { [a, b] | a < b, [a, b] ⊆ [m, n] }\n");
    fprintf(stderr, "  { (-INF, k] | k ∈ [m, n] }\n");
    fprintf(stderr, "  { [k, +INF) | k ∈ [m, n] }\n\n");

    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --m INT          Lower bound of the domain (default: -INF).\n");
    fprintf(stderr, "  --n INT          Upper bound of the domain (default: +INF).\n");
    fprintf(stderr, "  --wdelay N       Number of steps to wait before applying widening (default: disabled).\n");
    fprintf(stderr, "  --dsteps N       Number of descending steps (narrowing) (default: 0).\n");
    fprintf(stderr, "  --init FILE      Initial abstract state configuration for the entry point,\n");
    fprintf(stderr, "                   each abstract domain has its own representation (default: TOP).\n\n");

    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  SOURCE          Path to the source file (While language).\n\n");

    fprintf(stderr, "Notes on (m,n):\n");
    fprintf(stderr, "  (-INF,+INF)     Standard interval abstract domain.\n");
    fprintf(stderr, "  m > n           Constant propagation abstract domain.\n");
}

bool parse_int64(const char *arg, void *n) {
    int64_t *n_int = (int64_t *)n;
    char *endptr;
    *n_int = strtoll(arg, &endptr, 10);
    return *endptr == '\0';
}

bool parse_size(const char *arg, void *n) {
    size_t *n_size = (size_t *)n;
    while (isspace(*arg)) {
        arg++;
    }
    if (*arg == '-') return false;
    char *endptr;
    *n_size = strtoull(arg, &endptr, 10);
    return *endptr == '\0';
}

bool parse_string(const char *arg, void *c) {
    const char **string = (const char **)c;
    *string = arg;
    return true;
}

typedef bool (*parse_opt_val)(const char *arg, void *n);
bool get_opt(void *opt_val, const char *opt, bool *opt_found, parse_opt_val parse, int i, int argc, char **argv) {
    if (strcmp(opt, argv[i]) == 0) {
        if (*opt_found) {
            fprintf(stderr, "Parsing error: (%s) redundant option.\n", opt);
            exit(1);
        }
        if (i + 1 >= argc) {
            fprintf(stderr, "Parsing error: (%s) missing value.\n", opt);
            exit(1);
        }
        if (!parse(argv[i+1], opt_val)) {
            fprintf(stderr, "Parsing error: (%s) incorrect value type.\n", opt);
            exit(1);
        }
        *opt_found = true;
        return true;
    }
    return false;
}

void handle_cfg_cmd(int argc, char **argv) {
    /* Help handling */
    if (argc == 2) {
        print_help_cfg(argv);
    }
    else if (argc == 3) {
        const char *src_path = argv[2];
        While_Analyzer_Opt opt = {0};
        While_Analyzer *wa = while_analyzer_init(src_path, &opt);
        printf("https://dreampuf.github.io/GraphvizOnline/?engine=dot#\n");
        while_analyzer_cfg_dump(wa, stdout);
        while_analyzer_free(wa);
        printf("\n(You can also use Graphviz locally with '$ dot -Tpng -o graph.png')\n");
    }
    else {
        print_help_cfg(argv);
    }
}

void handle_analyze_cmd(int argc, char **argv) {
    /* Help handling */
    if (argc == 2) {
        print_help_analyze(argv);
    }
    else if (argc == 3) {
        if (strcmp(argv[2], "pinterval") == 0) {
            print_help_analyze_pinterval(argv);
        }
        else {
            print_help_analyze(argv);
        }
    }
    /* Parametric interval handling */
    else if (strcmp(argv[2], "pinterval") == 0) {
        While_Analyzer_Opt opt = {
            .type = WHILE_ANALYZER_PARAMETRIC_INTERVAL,
            .as = {
                .parametric_interval = {
                    .m = INT64_MIN,
                    .n = INT64_MAX,
                },
            },
        };

        While_Analyzer_Exec_Opt exec_opt = {
            .widening_delay = SIZE_MAX,
            .descending_steps = 0,
            .init_state_path = NULL,
        };

        const char *src_path = argv[3];

        bool m_found = false;
        bool n_found = false;
        bool wdelay_found = false;
        bool dsteps_found = false;
        bool init_found = false;
        
        /* Check options */
        for (int i = 4; i < argc; i+=2) {

            if (get_opt(&opt.as.parametric_interval.m, "--m", &m_found, parse_int64, i, argc, argv)) {
                continue;
            }
            if (get_opt(&opt.as.parametric_interval.n, "--n", &n_found, parse_int64, i, argc, argv)) {
                continue;
            }
            if (get_opt(&exec_opt.widening_delay, "--wdelay", &wdelay_found, parse_size, i, argc, argv)) {
                continue;
            }
            if (get_opt(&exec_opt.descending_steps, "--dsteps", &dsteps_found, parse_size, i, argc, argv)) {
                continue;
            }
            if (get_opt(&exec_opt.init_state_path, "--init", &init_found, parse_string, i, argc, argv)) {
                continue;
            }

            fprintf(stderr, "Parsing error: (%s) invalid option.\n", argv[i]);
            exit(1);
        }

        /* Analysis */
        printf("\n/========================\\\n");
        printf("|    Analysis options    |\n");
        printf("|========================|\n");
        printf("  domain : pinterval\n");
        if (opt.as.parametric_interval.m == INT64_MIN) {
            printf("  m      : -INF\n");
        } else {
            printf("  m      : %" PRId64 "\n", opt.as.parametric_interval.m);
        }
        if (opt.as.parametric_interval.n == INT64_MAX) {
            printf("  n      : +INF\n");
        } else {
            printf("  n      : %" PRId64 "\n", opt.as.parametric_interval.n);
        }
        if (exec_opt.widening_delay == SIZE_MAX) {
            printf("  wdelay : (disabled)\n");
        } else {
            printf("  wdelay : %zu\n", exec_opt.widening_delay);
        }
        printf("  dsteps : %zu\n", exec_opt.descending_steps);
        if (exec_opt.init_state_path == NULL) {
            printf("  init   : (TOP)\n");
        } else {
            printf("  init   : %s\n", exec_opt.init_state_path);
        }
        printf("\\========================/\n\n");

        While_Analyzer *wa = while_analyzer_init(src_path, &opt);
        while_analyzer_exec(wa, &exec_opt);
        while_analyzer_states_dump(wa, stdout);
        while_analyzer_free(wa);
    }
    else {
        print_help_analyze(argv);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help(argv);
    }
    else  if (strcmp(argv[1], "cfg") == 0) {
        handle_cfg_cmd(argc, argv);
    }
    else if (strcmp(argv[1], "analyze") == 0) {
        handle_analyze_cmd(argc, argv);
    }
    else {
        print_help(argv);
    }
}
