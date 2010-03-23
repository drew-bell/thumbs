/*
 Version 0.4
 Author Drew Bell
 A new tool. This will be an attempt to make the thumbs tool able to rotate the photo's
 based on the data in the EXIF of the files. This should save me some time. This tool will
 also extract the EXIF data from the file and dump it to 2 text files. Which will then be
 imported into the database.
 
 This will require a config file. The config file will be for the user to correctly set up
 the database options for the EXIF importing. 
 */

/*
Change log :
 0.4.5
 - added some error handeling.
 - added comments
 
 0.4.2
 - changed the compiler flags for a faster build
 
 0.4.1
 - Added the man page
 
  0.4
 - Removed redundant function and made getting the function for getting the thumbnail dimensions more generic
 - Renamed a bunch of function call and variables
 - Moved code for getting the EXIF orientation to it's own function
 - Added more comments to the code
 - Added the change log at the top of the file
 - Included libgen.h for getting the basename and dirnames of the files being processed
 - Moved the rotation of images to a new and separate function.

 Known Bugs : 
 - Filenames handling fails if the files being processed are outside the current working directory.

 */
//	gcc -Wall -lgd -lexif main.c -O2 -o thumbs

#include <stdio.h>
#include <gd.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h> 
#include <time.h>
#include <ctype.h>
#include <libexif/exif-data.h>

#define LENGTH _POSIX_PATH_MAX+1

// check if the entry is a directory
int is_dir(struct dirent *entry) {
	struct stat stat_buf;
	lstat(entry->d_name, &stat_buf);
	if (S_ISDIR(stat_buf.st_mode)) {
		return 1;
	}
	return 0;
}

// check to see if the file already exists
int file_exists(char *file) {
	if (0 != access(file,W_OK)) {
		return 0;
	}
	return 1;
}

// determine if the file is a jpeg based on the extention
int is_jpg(char *file) {
	char *ext;
	if ((ext = strrchr(file, '.')))	{
		if ((strcasecmp(ext,".jpg") == 0)) {	
			return 1;// if it is an image, return true
		}
	}
	return 0;	// if not a jpg, return false
}

/*
 
 Count the number of jpeg files in the current directory : returns an int

 */

int countfiles() {
	int i=0;
	int error;
	DIR *currentdir = opendir(".");
	struct dirent *entry = NULL;
	while ((entry = readdir(currentdir))) {
		if (is_dir(entry) || !strcmp(entry->d_name, ".DS_Store")) { //if it is a directory, skip it.
			continue;
		}
		if ((entry->d_name != NULL) && is_jpg(entry->d_name)) { //check to see if it is a jpeg
			i++;
		}
	}
	if (i == 0) {
		error = printf("No Jpeg files were found in this specified path.\n");
		if (error < 0)
			exit(EXIT_FAILURE);
	}
	return i;
}

// return a list of all .jpeg files for the current directory
void getfilelist(char *cwd_dir, int numoffiles, char filenames[numoffiles][LENGTH])
{
	int i = 0;
	DIR *currentdir = opendir(cwd_dir);
	struct dirent *entry = NULL;
	while ((entry = readdir(currentdir))) {
		if (is_dir(entry) || !strcmp(entry->d_name, ".DS_Store")) { //if it is a directory, skip it.
			continue;
		}
		
		if ((entry->d_name != NULL) && is_jpg(entry->d_name)) { //check to see if it is a jpeg
			strcpy(filenames[i], entry->d_name);
			i++;
		}		
	}	
	closedir(currentdir);
}

/* Calculates the ratio for the dimensions of the image and returns the correct width or height 
for the resized image based on the desired output size and the dimensions of the origional image.
*/
double get_thumb_xy(int short_length, int long_length, int desired) {
	double tmp = 0;
	tmp =  (double)short_length / (double)long_length;
	tmp = (double)desired / tmp ;
	return tmp;
}

int exif_rotation (char *in_file) {
	int o = 0;
	ExifData *exifData = exif_data_new_from_file(in_file);
	if (exifData) {
		ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
		ExifEntry *exifEntry = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);
		if (exifEntry)
			o = exif_get_short(exifEntry->data, byteOrder);
	}
	free(exifData);
	return o;
}

/*
 This rotates the image in memory and returns a gdImagePtr to the rotated image.
 */
gdImagePtr rotate_image(gdImagePtr target_id, int thumbnail_actual_height,int thumbnail_actual_width, int orientation) {
	gdImagePtr tmp = NULL;

	if (orientation == 8) {
		tmp = gdImageCreateTrueColor(thumbnail_actual_height, thumbnail_actual_width);
		gdImageCopyRotated(tmp,target_id,thumbnail_actual_height/2-1,thumbnail_actual_width/2-1,0,0,thumbnail_actual_width,thumbnail_actual_height,90);
	}
	else if (orientation == 6) {
		tmp = gdImageCreateTrueColor(thumbnail_actual_height, thumbnail_actual_width);
		gdImageCopyRotated(tmp,target_id,thumbnail_actual_height/2-1,thumbnail_actual_width/2,0,0,thumbnail_actual_width,thumbnail_actual_height,270);
	}
	// free some memory
	gdImageDestroy(target_id);
	
	// assign the target pointer to the rotated image
	target_id = tmp;
	
	return target_id;
}

 /*	Creates a new image from the source and writes it to disk in a subdirectory called Thumbnails.
 If the image contains EXIF rotation data, the output file is automatically rotated. This is done as the intended 
  use of this program is to process files for a web gallery. As most browsers don't support rotation information, 
  it is best to simply rotate the output image.

  May add rotation as an option in a later release.
 */
void makethumbnails(char out_file[_POSIX_PATH_MAX+1], char in_file[_POSIX_PATH_MAX+1], int thumbnail_max) {

	// variables for this function
	int image_width = 0; // width of the origional image
	int image_height = 0; // height of the origional image
	int thumbnail_actual_width = 0; // width to be output
	int thumbnail_actual_height = 0; // height to be output
	FILE *in = NULL, *out = NULL; // I/O file pointers
	gdImagePtr source_id, target_id; // image memory pointers
	int orientation = 0; // EXIF image orientation
			
			// open the file and load it into memory
			in = fopen(in_file,"r");
			source_id = gdImageCreateFromJpeg(in);
			fclose(in);

			// return the width and height of the original image
			image_width = gdImageSX(source_id);
			image_height = gdImageSY(source_id);
		
			// determine if the image is portrait or landscape and assign the proper ratio for resizing
			if (image_width > image_height) { 	
				thumbnail_actual_width = thumbnail_max;
				thumbnail_actual_height = get_thumb_xy(image_width, image_height,thumbnail_actual_width);
			}
			else {
				thumbnail_actual_height = thumbnail_max;
				thumbnail_actual_width = get_thumb_xy(image_height, image_width, thumbnail_actual_height);
			}
			
			// get the orientation information for the picture
			orientation = exif_rotation(in_file);
	 
			// create the image blank in memory
			target_id = gdImageCreateTrueColor(thumbnail_actual_width, thumbnail_actual_height);
									
			//resize and copy the image to the blank in memory
			gdImageCopyResampled(target_id,source_id,0,0,0,0,thumbnail_actual_width,thumbnail_actual_height,image_width,image_height);
			
			// destroy the origional from memory
			gdImageDestroy(source_id);
			
			// rotate the image 90 degree if necesary
			if (((orientation != 0) && (orientation == 6)) || (orientation == 8))
				target_id =	rotate_image(target_id, thumbnail_actual_height, thumbnail_actual_width, orientation);

			// open the file for output
			if (!(out = fopen(out_file,"wb"))) {
				printf("Can't write to %s \n",out_file);	// if there is a error opening a file
				exit(EXIT_FAILURE);							//return and error and exit
			}
			
			//write the new image to jpeg format
			gdImageJpeg(target_id, out, 80);
			
			//destroy the processed image from memory
			gdImageDestroy(target_id);
		
			//flush the buffer to disk
			fclose(out);
}

// check to make sure that a string is numeric
int is_numeric(const char *p) {
     if (*p) {
          char c;
          while ((c=*p++)) {
                if (!isdigit(c)) return 0;
          }
          return 1;
      }
      return 0;
}

// if the person enters the wrong parameters. output some info for the user.
int fail() {
		printf("\n");
		printf("Usage : thumbs <options> <files>\n");
		printf("      : You may leave the files option empty to process all files in the current directory\n");
		printf("\n");
		exit(EXIT_FAILURE);
}

// process a directory.
void process_files(int numoffiles, char filenames[numoffiles][LENGTH], char *cwd, int max_output_size) {
	int i = 0;
	int error;
	char out_file[_POSIX_PATH_MAX+1];
	char in_file[_POSIX_PATH_MAX+1];
	char working_dir[_POSIX_PATH_MAX+1];
	char thumdir[_POSIX_PATH_MAX+1];
	char thumbs[14] = "/Thumbnails";

	while (i < numoffiles) {
		// allocate memory to the string to hold the output file name
		// set the fully qualified names for the infile and out_file
		if (getcwd(in_file, _POSIX_PATH_MAX) == 0){perror("Something Fucked up.");}
		
		strcpy(working_dir, in_file);
		strcpy(out_file, in_file);
		strcat(in_file,"/");
		strcat(in_file,filenames[i]);
		strcat(out_file, thumbs);
		strcpy(thumdir, out_file);
		strcat(out_file,"/");
		strcat(out_file, filenames[i]);

		// check to see if the file has already been created.
		if (!file_exists(out_file)) {	
			if (!file_exists(thumdir)) {
				mkdir(thumdir, 0766);
			}
				// some nice output for the user
				error = printf("Processing %s : File %d of %d : %.2f complete\n", basename(in_file), i+1, numoffiles, (double)(i+1)/numoffiles*100);
					if (error < 0)
						exit(EXIT_FAILURE);
				makethumbnails(out_file, in_file, max_output_size);
		}
		else {
				// Indicate images that are not being processed
				error = printf("%s Exists\n", out_file);
					if (error < 0)
						exit(EXIT_FAILURE);
			}
		i++;
	}
}

/*********************************>main<*/
int main (int argc, char **argv) {
	time_t start,end;
	double dif;
	int error;
	
	// start the process timer
	time (&start);
	int numoffiles=0;
	int max_output_size = 0;
	int i = 1;
	int j = 0;
	int specificfile = 0;
	int onlyonesize = 0;
	char *name = (char *)malloc(LENGTH);char *cwd;
	
	//process arguments better
	if (argc >= 2) {
		while (i < argc) {
			if (is_numeric(argv[i])) {
				if (onlyonesize == 1) {
					printf("You attempted to set the output size twice.\n");
					fail();
				}
				// set the output size to the user input size
				sscanf(argv[i],"%d", &max_output_size);
				onlyonesize = 1;
			}

			if (!strcasecmp(argv[i], "s")) {
				if (onlyonesize == 1) {
					printf("You attempted to set the output size twice.\n");
					fail();
				}
				// set the thumbnail size for the "s" cli option to 130px
				max_output_size = 130;
				onlyonesize = 1;
			}

			if (file_exists(argv[i]) && is_jpg(argv[i])) {
				specificfile = 1;
				numoffiles = numoffiles + 1;
				fprintf(stderr,"Adding File %s\n", argv[i]);
			}
					
			if (!file_exists(argv[i]) && (strcasecmp(argv[i],"s") != 0) && !is_numeric(argv[i])) {
				fail();
				}
			i++;
		}
	}
	
	if (numoffiles == 0) {
		numoffiles = countfiles();
	}

	char filenames[numoffiles][LENGTH]; 

	if (max_output_size != 130 && max_output_size == 0) max_output_size = 800;

	// get the current working directory
	cwd = getcwd(name, (size_t)200);
	
	// add cli files to process list
	if (specificfile) {
		i = 1;
		j = 0;
		int	m = 0;
		while (m < argc) {
			if (file_exists(argv[i]) && is_jpg(argv[i]) && argv[i] != NULL) {
				strcpy(filenames[j], argv[i]);
				j++;
			}
			m++;
			i++;
		}
	}
	else {
		getfilelist(cwd, LENGTH, filenames);	
	}

	/*
	 I am going to output all the options here to see what is being set and to work out how to add some more error checking.
	 */
	process_files(numoffiles, filenames, cwd, max_output_size);
	free(name);
	// finish timer and output the time taken to complete the job in seconds
	time (&end);
	dif = difftime (end,start);
	error = printf ("It took %.2lf seconds to process your images.\n", dif );
		if (error < 0)
			exit(EXIT_FAILURE);
	return 0;
}
/* EOF */