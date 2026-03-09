/* ID Header:
 Student Name: Jeffrey Moniz
 Student ID: 3148591
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
    
    // check to make sure that there are enough arguments (at least --mode and --trace)
    if (argc < 3) {
        fprintf(stderr, "Error: missing required arguments\n");
        return false;
    }
    
    // go through each argument and parse it
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
        // Parse --base=... (checks if the string is a base, if it is move to the end of the string and convert the integer value into a long integer, otherwise print an error message)
        else if (strncmp(arg, "--base=", 7) == 0) {
            char *endptr;
            o->base = strtol(arg + 7, &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: --base must be a valid integer\n");
                return false;
            }
        }
        // Parse --limit=... (checks if the string is a limit, if it is move to the end of the string and convert the integer value into a long integer, otherwise print an error message)
        else if (strncmp(arg, "--limit=", 8) == 0) {
            char *endptr;
            o->limit = strtol(arg + 8, &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "Error: --limit must be a valid integer\n");
                return false;
            }
        }
        // Parse --trace=... (checks to see if the string value starts with --trace=, if it does move to the end of the string and set the trace_path to the value after --trace=, otherwise print an error message)
        else if (strncmp(arg, "--trace=", 8) == 0) {
            o->trace_path = arg + 8;
        }
        // Parse --config=... (checks to see if the string value starts with --config=, if it does move to the end of the string and set the config_path to the value after --config=, otherwise print an error message)
        else if (strncmp(arg, "--config=", 9) == 0) {
            o->config_path = arg + 9;
        }
        //if the argument does not match any of the expected patterns, print an error message and return false
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
    //tries to access the trace file to see if it is readable, if it is not print an error message and return false
    if(access(o->trace_path, R_OK) != 0) {
        fprintf(stderr, "Error: cannot read trace file '%s': %s\n", o->trace_path, strerror(errno));
        return false;
    }
    
    // Validate mode-specific requirements
    if (o->mode == MODE_BB) {
        //if the mode is set to BB and the base or limit is not set, print an error message and return false
        if (o->base == 0) {
            fprintf(stderr, "Error: BB mode requires --base=N\n");
            return false;
        }
        if (o->limit == 0) {
            fprintf(stderr, "Error: BB mode requires --limit=N\n");
            return false;
        }
    } else if (o->mode == MODE_SEG) {
        //if the mode is set to SEG and the config file is not set, print an error message and return false, also print an error if the config file is not readable
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
    int run = 0;
    (void)o; (void)st;
    // Open the trace file for reading, if it cannot be opened print an error message and return 1
    FILE *fp = fopen(o->trace_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open trace file '%s': %s\n", o->trace_path, strerror(errno));
        return 1;
    }
    
    // Read the trace file line by line, skipping comment lines (lines starting with #) and empty lines, and print each non-comment line to standard output
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Skip comment lines (lines starting with #)
        if (line[0] == '#' || line[0] == '\n') continue;
        
        run++;
        //these variables will hold the operation and the address from the trace file
        char *op = strtok(line, " \t\n");
        char *addr = strtok(NULL, " \t\n");
        char *extra = strtok(NULL, " \t\n");

        //this if statement will check to see if the address is null, if the operation is null or if there is a comment on the address line. if any of these are true the program will print an error
        if(addr == NULL || op == NULL || addr[0] == '#') {
            printf("Trace: '%s':%d: malformed: expected 'OP ADDR'\n", o->trace_path, run);
            st->accesses++;
            st->faults_bounds++;
            continue;
        }

        //this if statement will check to see if the operation is valid (R or W), if it is not print an error message and continue to the next line
        if(strcmp(op, "R") != 0 && strcmp(op, "W") != 0) {
            printf("Trace: '%s':%d: malformed: op must be R/W, got %s\n", o->trace_path, run, op);
            st->accesses++;
            st->faults_bounds++;
            continue;
        }

        char *endptr;
        long address = strtol(addr, &endptr, 10);

        //the following if statement will check to see if the end pointer is \0, if it is not \0 that means that the strtol was not able to convert the address to a long integer, thus we will print an error
        if (*endptr != '\0') {
            printf("Trace: '%s':%d: bad address '%s' (not decimal)\n",o->trace_path, run, addr);
            st->accesses++;
            st->faults_bounds++;
            continue;
        }

        //the following if else statement will first create the max address which is the address plus the base, then it will check to see if the address is greater than the limit, if it is print an error, otherwise print ok and the max address
        long max_address = address+o->base;
        if(address >= o->limit) {
            printf("%s %ld  -> fault: BOUNDS\n", op, address);
            st->accesses++;
            st->faults_bounds++;
        } else {
            st->accesses++;
            st->ok++;
            printf("%s %ld  -> PA %ld : ok\n", op, address, max_address);
        }
    }

    //the following statements will print the stats for the simulation, including the number of accesses, the number of ok accesses, and the number of faults due to bounds violations
    printf("== stats ==\n");
    printf("accesses: %lu, ok: %lu, fault.bounds: %lu\n", st->accesses, st->ok, st->faults_bounds);
    
    //close the file
    fclose(fp);
    return 0;
}

//seg
int run_seg(const sim_opts_t *o, stats_t *st) {
    (void)o; (void)st;

    // Open the trace file for reading, if it cannot be opened print an error message and return 1
    FILE *fp = fopen(o->trace_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open trace file '%s': %s\n", o->trace_path, strerror(errno));
        return 1;
    }
    
    // Read the trace file line by line, skipping comment lines (lines starting with #) and empty lines, and print each non-comment line to standard output
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Skip comment lines (lines starting with #)
        if (line[0] == '#' || line[0] == '\n') continue;
        printf("%s", line);
    }
    fclose(fp);

    printf("----\n");

    // Open the config file for reading, if it cannot be opened print an error message and return 1
    fp = fopen(o->config_path, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open config file '%s': %s\n", o->config_path, strerror(errno));
        return 1;
    }
    
    // Read the config file line by line, skipping comment lines (lines starting with #) and empty lines, and print each non-comment line to standard output
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
