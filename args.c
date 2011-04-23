#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "args.h"
#include <limits.h>
#include "functions.h"
#define no false
#define yes true
static const char PROGRAM_VERSION[] = "2.0";

void release_options (args o) {

	// Free the arguments buffers
	if (NULL != o->prefix) free (o->prefix);
	if (NULL != o->suffix) free (o->suffix);
	if (NULL != o->subdir) free (o->subdir);
	if (NULL != o) free (o);

} // release_options

void null_options (args o) {
	o->verbose = no; // verbosely tell the user what is happening
	o->gallery = no; // create a gallery 
    o->disableCC = no;
	o->prefix = NULL; // prefix to add to files
	o->suffix = NULL; // suffix to add to files
	o->subdir = NULL; // subdir to put the files in
	o->image_w = -1; // max height of the output files
	o->image_h = -1; // max width of the output files
	o->image_l = -1; // max long side
	o->num_of_files = -1; // As yet unused information
	o->quality = -1.0; // jpeg compression quality

} // null_options

static void set_defaults (args o, char **in_files) {

	// set tht number of files to be processed
	o->num_of_files = array_count (in_files);

	// set the default for the image sizes
	if (o->image_h == -1) {
		if (o->image_w == -1) {
			if (o->image_l == -1) {

				// defualt lenth on the long side of an image
				o->image_l = 800;
			}
		}
	}

	// set the image quality
	if (o->quality == -1.0) {

		// default quality
		o->quality = 0.9;
	}

	// set the default subdir
	if (NULL == o->subdir) {

		if (o->gallery) {
		
			// malloc some space for the dir name
			o->subdir = malloc (sizeof (char)*8);
			
			// default subdir
			strcpy (o->subdir, "Gallery");
			
			// ensure termination of the string
			o->subdir[7] = '\0';
			
		} else {
			
			// malloc some space for the dir name
			o->subdir = malloc (sizeof (char)*11);
			
			// default subdir
			strcpy (o->subdir, "Thumbnails");
			
			// ensure termination of the string
			o->subdir[10] = '\0';
			
		}
	}

} // set_defaults ()

void gallery_defaults (args o, args g) {
	
	// defualt lenth on the long side of an image
	g->image_l = 130;
	
	// set the image quality
	if (o->quality == -1.0) {
		
		// default quality
		g->quality = 0.8;

	} else {

		// set the quality
		g->quality = o->quality;
	}
	
	// set the default subdir
	if (NULL == o->subdir && (strcmp (o->subdir, "Thumbnails") == 0)) {
		
		// malloc some space for the dir name
		g->subdir = malloc (sizeof (char)*19);
		
		// default subdir
		strcpy (g->subdir, "Gallery/Thumbnails");
		
		// ensure termination of the string
		g->subdir[18] = '\0';
		
	} else {
		
		g->subdir = malloc (sizeof (char) * (strlen (o->subdir) + 12) );
		
		strcpy (g->subdir, o->subdir);

		strcat (g->subdir, "/Thumbnails");
	}
	
}

static void help (const char *argv0) {
	
	char *argv0_copy = strdup (argv0);
    char *argv0_base = basename (argv0_copy);
	
	fprintf (stderr, "Usage: %s [OPTIONS] <SVG_file> <out_file>\n", argv0_base);
	puts ("");
    fprintf (stderr, "  -c, --disable-ColCon\tDisable conversion from Adobe1998 to sRGB.\n");
	fprintf (stderr, "  -w, --width=WIDTH\tWidth of output image in pixels.\n");
	fprintf (stderr, "  -h, --height=HEIGHT\tHeight of output image in pixels.\n");
	fprintf (stderr, "  -l, --longside=LENGTH\tLongside of output image in pixels.\n");
	puts ("");
	fprintf (stderr, "  -s, --suffix=FILE_NAME_SUFFIX\tA suffux to be applied to all files.\n");
	fprintf (stderr, "  -p, --prefix=FILE_NAME_PREFIX\tA prefix to be applied to all files.\n");
	fprintf (stderr, "  -d, --subdir=SUD_DIRECTORY\tThe subdir to output files into [DEFAULT=Thumbnails].\n");
	fprintf (stderr, "  -q, --quality=QUALITY\tThe compression quality for the output files 0 - 100 [DEFAULT=60].\n");
	puts ("");
	fprintf (stderr, "  -g, --gallery\t\tResize files and create a 2nd level of smaller thumbnails in a Thumbnails subdir.\n");
	puts ("");
	fprintf (stderr, "  -?, --help\t\tGive this help.\n");
	fprintf (stderr, "  -V, --version\t\tProgram Version.\n");
	fprintf (stderr, "  -v, --verbose\t\tVerbose output.\n");
	puts ("");
	free (argv0_copy);
	exit (0);

} // help ()

void process_cli_args (int argc, char **argv, args opts, char **in_files) {
	
	int c;
	int i = 0;
	char *file;
	// the long options for the program
	static struct option long_options[] = {
		
		/* These options set a bool flag. */
		{"version", no_argument, NULL, 'V'}, 
        {"disable-ColCon", no_argument, NULL, 'c'},
		{"verbose", no_argument, NULL, 'v'}, 
		{"help", no_argument, NULL, '?'}, 
		{"gallery", no_argument, NULL, 'g'},
		
		/* THese options have an argument requirement */
		{"suffix", required_argument, NULL, 's'}, 
		{"prefix", required_argument, NULL, 'p'},  
		{"subdir", required_argument, NULL, 'd'},
		{"quality", required_argument, NULL, 'q'},
		{"width", required_argument, NULL, 'w'}, 
		{"height", required_argument, NULL, 'h'},
		{"longside", required_argument, NULL, 'l'},
		{NULL, 0, NULL, 0}
	};
	
	// parse the options and set the values in the options structure
	while ( (c = getopt_long (argc, argv, "V?vcgq:l:p:d:s:w:h:", long_options, NULL)) != -1) {
		switch (c) {
			case 'v':
				// Output the strokes in individual images collectively
				opts->verbose = yes;
				break;
			case 'g':
				// Set the remove numbers option
				opts->gallery = yes;
				break;
            case 'c':
                // Turn off colour conversion
                opts->disableCC = yes;
                break;
			case 'w':
				// Add the argument for the -w tag as an integer to the struct
				opts->image_w = atoi (optarg);
				break;
			case 'q':
				// Add the argument for the -w tag as an integer to the struct
				opts->quality = atof (optarg)/100;
				break;
			case 'h':
				// Add the argument for the -h tag as an integer to the struct
				opts->image_h = atoi (optarg);
				break;
			case 'l':
				// Add the argument for the -l tag as an integer to the struct
				opts->image_l = atoi (optarg);
				break;
			case 's':
				// Set the remove start mark option
				opts->suffix = malloc (strlen (optarg));
				strcpy (opts->suffix, optarg);
				break;
			case 'p':
				// Set the remove arrows option
				opts->prefix = malloc (strlen (optarg));
				strcpy (opts->prefix, optarg);
				break;
			case 'd':
				// Set the remove arrows option
				opts->subdir = malloc (strlen (optarg));
				strcpy (opts->subdir, optarg);
				break;
			case 'V':
				// Output the version of the program option
				fprintf (stderr, "Version v%s\n", PROGRAM_VERSION);
				exit (0);
				break;
			case '?':
				// Output the help information
				help (argv[0]);
				break;
			default:
				break;
		}
	}
	
	if (optind < argc) {

		while (optind < argc) {
			
			file = argv[optind];

			// Check to see if the file is already in the array
			if (file_exists (file) && is_jpg (file) && !in_array (in_files, file)) {

				// Allocate space for the file name
				in_files[i] = malloc (strlen (argv[optind]));

				// copy the string into the memory allocated
				strcpy (in_files[i], argv[optind]);

				// move to the next position
				i++;
				optind++;

				// Null the next pointer
				in_files[i] = NULL;
			} else {
				
				// move to the next position
			//	i++;
				optind++;
			}
		}
	} else {
		
		printf ("No files to process.\n");
		
		// Free the memory used for the options structure.
		release_options (opts);
		
		// Free the memory used for the file array
		free_files (in_files);
		
		// exit the program
		exit (0);
	}
	
	// set the defaults
	set_defaults (opts, in_files);

} // process_1_image ()
