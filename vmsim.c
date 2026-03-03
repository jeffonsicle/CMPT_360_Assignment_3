/* ID Header:
 Student Name:
 Student ID:
 Submission Date:
 File:
*/

#include "vmsim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

// usage
static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s --mode=bb  --base=N --limit=N --trace=FILE \n"
        "  %s --mode=seg --config=FILE --trace=FILE \n",
        prog, prog);
}

// CLI
bool parse_args(int argc, char **argv, sim_opts_t *o) {
    if (o) memset(o, 0, sizeof(*o));
    
    // Initialize mode to invalid value so we can detect if it was set
    o->mode = -1;
    
    // Need at least mode and trace
    if (argc < 3) {
        fprintf(stderr, "Error: missing required arguments\n");
        return false;
    }
    
    // Parse each argument
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        
        // Parse --mode=...
        if (strncmp(arg, "--mode=", 7) == 0) {
            const char *mode = arg + 7;
            if (strcmp(mode, "bb") == 0) {
                o->mode = MODE_BB;
            } else if (strcmp(mode, "seg") == 0) {
                o->mode = MODE_SEG;
            } else {
                fprintf(stderr, "Error: invalid mode '%s'. Expected 'bb' or 'seg'\n", mode);
                return false;
            }
        }
        // Parse --base=...
        else if (strncmp(arg, "--base=", 7) == 0) {
            char *endptr;
            o->base = strtol(arg + 7, &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: --base must be a valid integer\n");
                return false;
            }
        }
        // Parse --limit=...
        else if (strncmp(arg, "--limit=", 8) == 0) {
            char *endptr;
            o->limit = strtol(arg + 8, &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: --limit must be a valid integer\n");
                return false;
            }
        }
        // Parse --trace=...
        else if (strncmp(arg, "--trace=", 8) == 0) {
            o->trace_path = arg + 8;
        }
        // Parse --config=...
        else if (strncmp(arg, "--config=", 9) == 0) {
            o->config_path = arg + 9;
        }
        else {
            fprintf(stderr, "Error: unknown argument '%s'\n", arg);
            return false;
        }
    }
    
    // Validate: mode must be set
    if (o->mode < 0 || (o->mode != MODE_BB && o->mode != MODE_SEG)) {
        fprintf(stderr, "Error: missing --mode=bb or --mode=seg\n");
        return false;
    }
    
    // Validate: trace is required for both modes
    if (o->trace_path == NULL) {
        fprintf(stderr, "Error: missing --trace=FILE\n");
        return false;
    }
    if(access(o->trace_path, R_OK) != 0) {
        fprintf(stderr, "Error: cannot read trace file '%s': %s\n", o->trace_path, strerror(errno));
        return false;
    }
    
    // Validate mode-specific requirements
    if (o->mode == MODE_BB) {
        if (o->base == 0) {
            fprintf(stderr, "Error: BB mode requires --base=N\n");
            return false;
        }
        if (o->limit == 0) {
            fprintf(stderr, "Error: BB mode requires --limit=N\n");
            return false;
        }
    } else if (o->mode == MODE_SEG) {
        if (o->config_path == NULL) {
            fprintf(stderr, "Error: SEG mode requires --config=FILE\n");
            return false;
        }
        if(access(o->config_path, R_OK) != 0) {
            fprintf(stderr, "Error: cannot read config file '%s': %s\n", o->config_path, strerror(errno));
            return false;
        }
    }
    
    return true;
}

//bb
int run_bb(const sim_opts_t *o, stats_t *st) {
    (void)o; (void)st;
    FILE *fp = fopen(o->trace_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open trace file '%s': %s\n", o->trace_path, strerror(errno));
        return 1;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Skip comment lines (lines starting with #)
        if (line[0] == '#' || line[0] == '\n') continue;
        printf("%s", line);
    }
    
    fclose(fp);
    return 0;
}

//seg
int run_seg(const sim_opts_t *o, stats_t *st) {
    (void)st;
    FILE *fp = fopen(o->trace_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open trace file '%s': %s\n", o->trace_path, strerror(errno));
        return 1;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Skip comment lines (lines starting with #)
        if (line[0] == '#' || line[0] == '\n') continue;
        printf("%s", line);
    }
    
    fclose(fp);

    printf("----\n");

    fp = fopen(o->config_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open config file '%s': %s\n", o->config_path, strerror(errno));
        return 1;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        // Skip comment lines (lines starting with #)
        if (line[0] == '#' || line[0] == '\n') continue;
        printf("%s", line);
    }
    
    fclose(fp);
    return 0;
}

//main()

int main(int argc, char **argv) {
    sim_opts_t opts;
    if (!parse_args(argc, argv, &opts)) { usage(argv[0]); return 1; }
    stats_t st = (stats_t){0};
    if (opts.mode == MODE_BB) return run_bb(&opts, &st);
    else return run_seg(&opts, &st);
}
