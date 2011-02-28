#include <stdio.h>
#include "args.h"
#include "functions.h"
#include "image_processing.h"


int main (int argc, char **argv) {
	time_t start,end;
	double dif;
	int error;
	
	// start the process timer
	time (&start);
  
	// Array of files from argv
	char **in_files;
	int i = 0;
	
	// Malloc the memory for the array
	in_files = malloc (sizeof (char*) * argc);
	
	// Null the first position so we can check for null when there are no files present
	in_files[0] = NULL;
	
	// Create the args struct in memory to be filled
	args cli_flags = (args) malloc (sizeof (options));
	
	// set all the cli options to basic inputs
	null_options (cli_flags);
	
	// parse all the cli flags from the user into a struct for use.
	process_cli_args (argc, argv, cli_flags, in_files);
	
	// Print out the stored arguments
  //	print_args (cli_flags);
  
	// print out all the files in the in_file array
  //	print_files (in_files);
  
	if (cli_flags->gallery) {
    
		// Create the args struct in memory to be filled
		args g_cli_flags = (args) malloc (sizeof (options));
		
		// set all the cli options to basic inputs
		null_options (g_cli_flags);
    
		// set the gallery defaults
		gallery_defaults (cli_flags, g_cli_flags);
    
		// parse the array of files, processing each one
		while (in_files[i] != NULL) {
      
			// Info for the user
			printf ("Processing file for gallery : %s\n", in_files[i]);
      
			// Process the current image
			process_1_image (g_cli_flags, in_files[i]);
      
			// increment the counter
			i++;
		}
    
		release_options (g_cli_flags);	
		g_cli_flags = NULL;
    
		// reset counter to 0
		i = 0;
    
	}
	
	// Info for the user
	printf ("%i images to process.\n", cli_flags->num_of_files);
  
	// parse the array of files, processing each one
	while (in_files[i] != NULL) {
    
		// Info for the user
		printf ("\nProcessing file %i of %i : %s ",i+1 , cli_flags->num_of_files, in_files[i]);
    
		// Process the current image
		process_1_image (cli_flags, in_files[i]);
    
		// Info for the user
		printf ("| %.2f complete. \t", (double) (i+1) / cli_flags->num_of_files * 100);
    
		// increment the counter
		i++;
	}
  
	// Free the memory used for the options structure.
	release_options (cli_flags);
	cli_flags = NULL;
	
	// Free the memory used for the file array
	free_files (in_files);
	in_files = NULL;
	
	// finish timer and output the time taken to complete the job in seconds
	time (&end);
  
	dif = difftime (end,start);
	char *s;
	if (i == 1) {
		s = "";
	} else {
		s = "s";
	}
  
	error = printf ("\nIt took %.2lf seconds to process your %d image%s.\n", dif, i, s);
	if (error < 0)
		exit (EXIT_FAILURE);
	
} // Main Function
