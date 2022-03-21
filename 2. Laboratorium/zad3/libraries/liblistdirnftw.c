#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include "liblistdirnftw.h"

/*
 * Private functions
 */
static int process_entity(const char* fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
static void update_stats(const struct stat *s);

// Global statistics
Stats *global_stats;


bool list_dir(char* path) {
    // Allocate memory for the final statistics of the listed directory
    global_stats = (Stats*) calloc(1, sizeof(Stats));
    if (global_stats == NULL) {
        perror("Error: Cannot allocate memory\n");
        return false;
    }

    char* abs_path = get_abs_path(path);
    if (!abs_path) return false;
    print_info_headers();
    if (nftw(abs_path, process_entity, 10, FTW_PHYS) != 0) {
        perror("Error: Issues while processing entity.\n");
        return false;
    }
    free(abs_path);
    print_summary();
    free(global_stats);
    return true;
}

static int process_entity(const char* fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    print_entity_info(fpath, sb);
    update_stats(sb);
    return 0;
}

char* get_abs_path(char* path) {
    char buff[PATH_MAX];
    realpath(path, buff);
    char* abs_path = (char*) calloc(strlen(buff) + 1, sizeof(char));
    if (abs_path == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }
    strcpy(abs_path, buff);
    return abs_path;
}

bool print_entity_info(const char* path, const struct stat *sb) {
    char* type = get_entity_type(sb);
    if (type == NULL) {
        perror("Error: Entity's type is not recognized.\n");
        return false;
    }

    printf("%8ld |", sb->st_nlink);
    printf(" %9s |", type);
    printf(" %10ldB |", sb->st_size);

    char* lat = get_formatted_time(sb->st_atime);
    char* lmt = get_formatted_time(sb->st_mtime);

    bool status = true;
    if (lat != NULL && lmt != NULL) {
        printf(" %s |", lat);
        printf(" %s |", lmt);
        printf(" %s\n", path);
    } else status = false;

    if (lat != NULL) free(lat);
    if (lmt != NULL) free(lmt);

    return status;
}

char* get_formatted_time(time_t time) {
    // Convert time to the formatted string
    char buff[20];
    struct tm* time_info;
    time_info = localtime(&time);
    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", time_info);
    int length = strlen(buff);

    // Copy formatted string to the persistent memory
    char* f_time = (char*) calloc(length + 1, sizeof(char));
    if (f_time == NULL) {
        perror("Error: Cannot allocate memory.\n");
        return NULL;
    }
    strcpy(f_time, buff);
    return f_time;
}

void print_info_headers() {
    printf("%-8s | %-9s | %-11s | %-19s | %-19s | %s\n",
           "No Links",
           "Type",
           "Size",
           "Last access",
           "Last modification",
           "Absolute path");
    for (int i = 0; i < 95; i++) printf("-");
    printf("\n");
}

void print_summary() {
    printf("\n");
    for (int i = 0; i < 29; i++) printf("-");
    printf("\n| %-25s |\n", "SUMMARY");
    printf("| Plain files: %-12d |\n", global_stats->no_files);
    printf("| Directories: %-12d |\n", global_stats->no_dirs);
    printf("| Char devices: %-11d |\n", global_stats->no_char_devs);
    printf("| Block devices: %-10d |\n", global_stats->no_block_devs);
    printf("| Named pipes: %-12d |\n", global_stats->no_fifos);
    printf("| Symbolic link: %-10d |\n", global_stats->no_slinks);
    printf("| Sockets: %-16d |\n", global_stats->no_socks);
    for (int i = 0; i < 29; i++) printf("-");
}

char* get_entity_type(const struct stat *s) {
    char* type = NULL;
    switch (s->st_mode & S_IFMT) {
        case S_IFREG: return "file";
        case S_IFDIR: return "dir";
        case S_IFCHR: return "char dev";
        case S_IFBLK: return "block dev";
        case S_IFIFO: return "fifo";
        case S_IFLNK: return "slink";
        case S_IFSOCK: return "sock";
    }
    return type;
}

static void update_stats(const struct stat *s) {
    switch (s->st_mode & S_IFMT) {
        case S_IFREG:
            global_stats->no_files++;
            break;
        case S_IFDIR:
            global_stats->no_dirs++;
            break;
        case S_IFCHR:
            global_stats->no_char_devs++;
            break;
        case S_IFBLK:
            global_stats->no_block_devs++;
            break;
        case S_IFIFO:
            global_stats->no_fifos++;
            break;
        case S_IFLNK:
            global_stats->no_slinks++;
            break;
        case S_IFSOCK:
            global_stats->no_socks++;
            break;
    }
}
