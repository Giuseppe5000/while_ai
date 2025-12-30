#include "include/abstract_analyzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

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

void print_help_analyze_p_interval(char **argv) {
    fprintf(stderr, "Usage: %s analyze p_interval SOURCE [OPTIONS]\n\n", argv[0]);
    fprintf(stderr, "TODO: Parametric interval exaplain.\n\n");

    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -m INT          Lower bound of the domain (default: -INF).\n");
    fprintf(stderr, "  -n INT          Upper bound of the domain (default: +INF).\n");
    fprintf(stderr, "  -wdelay N       Number of steps to wait before applying widening (default: disabled).\n");
    fprintf(stderr, "  -dsteps N       Number of descending steps (narrowing) (default: 0).\n\n");

    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  SOURCE          Path to the source file (While language).\n");
}

/* If 'opt' string is in argv[i] ... argv[argc-2] then returns the index of the next string in argv (the option) */
int get_opt(const char *opt, int i, int argc, char **argv) {
    while (i < argc) {
        if (strcmp(opt, argv[i]) == 0) {
            /* If there is the opt but not the value */
            if (i+1 == argc) {
                return i; /* Returns the opt if there is no value */
            }
            return i+1;
        }
        i++;
    }
    return -1;
}

bool parse_int64(const char *arg, int64_t *n) {
    if (isdigit(*arg) || *arg == '-') {
        const char *start = arg;
        arg++;

        while (isdigit(*arg)) {
            arg++;
        }

        if (*arg != '\0') {
            return false;
        }

        *n = strtoll(start, NULL, 10);
        return true;
    }
    return false;
}

bool parse_size(const char *arg, size_t *n) {
    if (isdigit(*arg)) {
        const char *start = arg;

        while (isdigit(*arg)) {
            arg++;
        }

        if (*arg != '\0') {
            return false;
        }

        *n = strtoull(start, NULL, 10);
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
        while_analyzer_cfg_dump(wa, stdout);
        while_analyzer_free(wa);
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
            print_help_analyze_p_interval(argv);
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
        };

        const char *src_path = argv[3];

        /* get_opt return NULL if not found or the pointer to the start of the flag */
        int m = get_opt("-m", 4, argc, argv);
        int n = get_opt("-n", 4, argc, argv);
        int wdelay = get_opt("-wdelay", 4, argc, argv);
        int dsteps = get_opt("-dsteps", 4, argc, argv);

        if (m != -1) {
            if (!parse_int64(argv[m], &opt.as.parametric_interval.m)) {
                fprintf(stderr, "Parsing error: (-m) INT expected.\n");
                return;
            }
        }
        if (n != -1) {
            if (!parse_int64(argv[n], &opt.as.parametric_interval.n)) {
                fprintf(stderr, "Parsing error: (-n) INT expected.\n");
                return;
            }
        }
        if (wdelay != -1) {
            if (!parse_size(argv[wdelay], &exec_opt.widening_delay)) {
                fprintf(stderr, "Parsing error: (-wdelay) N expected.\n");
                return;
            }
        }
        if (dsteps != -1) {
            if (!parse_size(argv[dsteps], &exec_opt.descending_steps)) {
                fprintf(stderr, "Parsing error: (-dsteps) N expected.\n");
                return;
            }
        }

        printf("\n/========================\\\n");
        printf("|    Analysis options    |\n");
        printf("|========================|\n");
        printf("  domain : pinterval\n");
        printf("  m      : %s\n", opt.as.parametric_interval.m == INT64_MIN ? "-INF" : argv[m]);
        printf("  n      : %s\n", opt.as.parametric_interval.n == INT64_MAX ? "+INF" : argv[n]);
        printf("  wdelay : %s\n", exec_opt.widening_delay == SIZE_MAX ? "disabled" : argv[wdelay]);
        printf("  dsteps : %zu\n", exec_opt.descending_steps);
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
