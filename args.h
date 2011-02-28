#include <stdbool.h>

// cli arguments structure
typedef struct {
	bool verbose; // verbosely tell the user what is happening
	bool gallery; // create a gallery 
	char *prefix; // prefix to add to files
	char *suffix; // suffix to add to files
	char *subdir; // subdir to put the files in
	int image_w; // max height of the output files
	int image_h; // max width of the output files
	int image_l; // max long side
	int num_of_files; // As yet unused information
	float quality;
} options;

typedef options *args;

void process_cli_args (int argc, char **argv, args cli_flags, char **in_files);

void null_options (args cli_flags);

void release_options (args cli_flags);

static void help (const char *argv0);

static void set_defaults (args cli_flags, char **in_files);

void gallery_defaults (args o, args g);
