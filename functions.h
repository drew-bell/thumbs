#include <stdlib.h>
#import <ApplicationServices/ApplicationServices.h>

void print_args (args cli_flags);

void print_files (char **files_array);

void free_files (char **files_array);

bool in_array (char **array, char *entry);

void out (char *string);

int is_jpg (char *file);

int file_exists (char *file);

/* THIS FUNCTION WAS FOUND at http://skype4pidgin.googlecode.com/svn/trunk/skype_messaging_carbon2.c */
int CFNumberToCInt (CFNumberRef input);

char* rem_ext (char *file_name);

void create_dirs (char *directory);

int array_count (char **array);