#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define __USE_XOPEN     // strptime
#include <time.h>
#include <assert.h>

#define LBUF_SIZE 128

typedef enum { EMPTY, STARTED, STOPPED, UNKNOWN } Tstate;

char usage[] = "wt [<option>]\n"
    "<option>:\n"
    "  -a\t\t\tstart counting\n"
    "  -s\t\t\tstop counting\n"
    "  -c\t\t\tdisplay current elapsed time\n"
    "  -r [<start> [<stop>]]\tdisplay time for period\n"
    "  -h\t\t\tdisplay help\n";

int fgetlastl(FILE * fd, char *lbuf, int size)
{
    int lines = 0;

    if (ftell(fd) != 0)
        fseek(fd, 0, SEEK_SET);

    while (fgets(lbuf, size, fd) != NULL)
        lines++;

    return (lines);
}

Tstate cur_state(FILE * fd)
{
    char lbuf[LBUF_SIZE];
    int words = 0;
    int buflen;

    if (fgetlastl(fd, lbuf, LBUF_SIZE) == 0)
        return EMPTY;

    // count 'words' in the last line
    buflen = strlen(lbuf);
    for (int i = 0; i < buflen; i++) {
        // beggining of word
        if (!isspace(lbuf[i]) && (i == 0 || isspace(lbuf[i - 1])))
            words++;
    }

    if (words == 1)
        return STARTED;
    else if (words == 2)
        return STOPPED;
    else
        return UNKNOWN;
}

void start_counting(FILE * fd)
{
    time_t tnow = time(NULL);

    fseek(fd, 0, SEEK_END);
    fprintf(fd, "%i\n", (int) tnow);
}

void stop_counting(FILE * fd)
{
    int tnow = (int) time(NULL);

    fseek(fd, -1, SEEK_END);
    fprintf(fd, " %i\n", tnow);
}

void print_elapsed(FILE * fd)
{
    char buf[LBUF_SIZE];
    int tnow = (int) time(NULL);
    int tprev = 0;

    fgetlastl(fd, buf, LBUF_SIZE);
    sscanf(buf, "%i", &tprev);
    printf("%i\n", (tnow - tprev));
}

void print_report(FILE * fd, time_t start, time_t end)
{
    assert(start <= end);
    char lbuf[LBUF_SIZE];
    char *ret;
    int c;
    int tstart, tend, tsum = 0, tnow = time(NULL);

    if (ftell(fd) != 0)
        fseek(fd, 0, SEEK_SET);

    while ((ret = fgets(lbuf, LBUF_SIZE, fd)) != NULL) {
        c = sscanf(lbuf, "%i %i", &tstart, &tend);
        if (tstart >= start && tend <= end) {
            // one match means a count in progress
            if (c == 1) {
                tsum += (tnow - tstart);
            } else {
                tsum += (tend - tstart);
            }
        }
    }

    printf("%i\n", (int) tsum);
}

int main(int argc, char *argv[])
{
    FILE *fd;
    Tstate prev_state;
    char filename[256];
    time_t rstart, rend, tnow;
    struct tm tm;

    char *home = getenv("HOME");
    snprintf(filename, sizeof(filename), "%s/.wtimed", home);

    if ((fd = fopen(filename, "r+")) == NULL) {
        if ((fd = fopen(filename, "w+")) == NULL) {
            fprintf(stderr, "Error while opening file.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (argc >= 2) {
        if (argv[1][0] != '-') {
            fprintf(stderr, "Unknown parameter: %s\n", argv[1]);
        } else {
            if ((prev_state = cur_state(fd)) == UNKNOWN && argv[1][1] != 'x') {
                fprintf(stderr, "Corrupted data file.\n");
                exit(EXIT_FAILURE);
            }
            switch (argv[1][1]) {
            case 'h':
                printf("usage: %s\n", usage);
                break;
            case 'a':
                if (prev_state == STARTED)
                    fprintf(stderr, "We are allready counting.\n");
                else
                    start_counting(fd);
                break;
            case 's':
                if (prev_state != STARTED)
                    fprintf(stderr, "We are not counting.\n");
                else
                    stop_counting(fd);
                break;
            case 'r':
                if (argc == 4) {        // end
                    if (strptime(argv[3], "%d-%m-%Y", &tm) == NULL) {
                        fprintf(stderr, "Malformated end date\n");
                        exit(EXIT_FAILURE);
                    }
                    tm.tm_min = tm.tm_sec = 59;
                    tm.tm_hour = 23;
                    rend = mktime(&tm);
                } else {
                    rend = time(NULL);
                }
                if (argc >= 3) {        // start
                    if (strptime(argv[2], "%d-%m-%Y", &tm) == NULL) {
                        fprintf(stderr, "Malformated start date\n");
                        exit(EXIT_FAILURE);
                    }
                    tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
                    rstart = mktime(&tm);
                } else {        // beginning of month
                    tnow = time(NULL);
                    localtime_r(&tnow, &tm);
                    tm.tm_mday = 1;
                    tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
                    rstart = mktime(&tm);
                }
                print_report(fd, rstart, rend);
                break;
            case 'c':
                if (prev_state != STARTED)
                    fprintf(stderr, "We are not counting.\n");
                else
                    print_elapsed(fd);
                break;
            default:
                fprintf(stderr, "Unknown parameter: %s\n", argv[1]);
                break;
            }
        }
    } else
        printf("usage: %s\n", usage);

    fclose(fd);
    exit(EXIT_SUCCESS);
}
