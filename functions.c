#include <stdio.h>
#include "args.h"
#include "functions.h"
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

int array_count (char **array) {
	int i = 0;
	
	while (array[i] != NULL) {
		i++;
	}
	return i;
}

void out (char *string) {

	printf ("%s\n",string);

} // out ()

// print all the cli_arguments entered
void print_args (args cli_flags) {
	if (cli_flags->verbose) printf ("Verbose : true\n"); else printf ("Verbose : false\n");
	if (cli_flags->gallery) printf ("Gallery : true\n"); else printf ("Gallery : false\n");
	
	if (NULL == cli_flags->suffix) printf ("Suffix : NULL\n"); else printf ("Suffix : %s\n", cli_flags->suffix);
	if (NULL == cli_flags->prefix) printf ("Prefix : NULL\n"); else printf ("Prefix : %s\n", cli_flags->prefix);
	if (NULL == cli_flags->subdir) printf ("Subdir : NULL\n"); else printf ("Subdir : %s\n", cli_flags->subdir);
	
	printf ("Width : %d \n", cli_flags->image_w);
	printf ("Height : %d \n", cli_flags->image_h);
	printf ("Quality : %f \n", cli_flags->quality);
	printf ("Length : %d \n", cli_flags->image_l);

} // print_args ()

void print_files (char **files) {
	
	int i = 0;
	
	while (files[i] != NULL) {
		
		printf ("Filename : files[%d] = %s\n", i, files[i]);
		i++;
	}

} // print_files

/* THIS FUNCTION WAS FOUND at http://skype4pidgin.googlecode.com/svn/trunk/skype_messaging_carbon2.c */
int CFNumberToCInt (CFNumberRef input) {
	
	// Check for null
	if (input == NULL)
		
		return 0;
	
	// create the return var
	int output;
	
	// convert the value to a C int
	CFNumberGetValue (input, kCFNumberIntType, &output);
	
	// return the int
	return output;

} // CFNumberToCInt ()

// free all the enteries in an array
void free_files (char **files) {

	int i = 0;

	while (files[i] != NULL) {

		free (files[i++]); // free the individual file names
	}

	free (files); // free the main array

} // free_files ()

// check for a string in an array of strings
bool in_array (char **array, char *entry) {
	
	int i = 0;
	
	while (array[i] != NULL) {
		
		if (strcmp (array[i], entry) == 0) { 
			
			// If the file is found to exist in the array, return true
			return true;
		} 
		
		// move to the next item
		i++;
	}
	
	// If not found, return false
	return false;

} // in_array ()

// determine if the file is a jpeg based on the extention
int is_jpg (char *file) {
	
	char *ext;
	
	if ( (ext = strrchr (file, '.')))	{
		
		if ( (strcasecmp (ext,".jpg") == 0)) {	
			
			// if it is an image, return true
			return true;
		}
	}
	
	// if not a jpg, return false
	return false;

} // is_jpg ()

// check to see if the file already exists
int file_exists (char *file) {
	
	// Check for access
	if (0 == access (file, W_OK)) {
		
		return true;
	}
	
	return false;

} // file_exists ()

// remove the extention from a filename.
char* rem_ext (char *file) {
	
	char *minus_ext = malloc ( strlen (file));
	
	// copy all the chars before the last . to the minus_ext var
	strncpy (minus_ext, file, strlen (file)-strlen (strrchr (file, '.')));
	
	// properly terminate the string
	minus_ext[strlen (file) - strlen (strrchr (file, '.'))] = '\0';
	
	// return the string
	return minus_ext;

} // rem_ext ()

// recurrsively create dirs
void create_dirs (char *dir) {
	
	char tmp[strlen (dir)];
	int ret;
	
	// check to see if the dir exists
	if (!file_exists (dir)) {
		
		// try to make the dir
		ret = mkdir (dir, 0744);
		
		// if it fails, go down one level
		if (ret != 0) {
			
			// remove the last path element
			strncpy (tmp, dir, strlen (dir) - strlen (strrchr (dir, '/')));
			
			// ensure the string is terminated
			tmp[strlen (dir) - strlen ( strrchr (dir, '/'))] = '\0';

			// try to make the dir below
			create_dirs (tmp);
		}
		
		// try to make the dir
		create_dirs (dir);
	}

} // create_dirs ()
