#include <Accelerate/Accelerate.h>
#include <ApplicationServices/ApplicationServices.h>

/* Data structure for each image */
typedef struct {
	int image_rot;
	int image_w;
	int image_h;
	int bits_ppixel;
	int bytes_row;
  CGColorSpaceRef colorSpace;
} image_properties; // image properties

typedef image_properties *img_prop;

/* This function takes the command line arguments and 1 file name */
void process_1_image (args cli_flags, char *file_name);

/* Get EXIF data */
int get_exif_rot (CGDataProviderRef image_source);

/* Flip the image 180 */
void flip_image (vImage_Buffer *vImage_source, img_prop image_prperties, args cli_flags);

/* Rotate the image by the EXIF rotation tag specified amount */
void rotate_image (vImage_Buffer *vImage_source, img_prop image_prperties, args cli_flags);

/* Resize the image to the required size or by default of 800px long side */
void resize_image (vImage_Buffer *vImage_source, img_prop image_prperties, args cli_flags);

/* Save the image to disk */
void save_image (vImage_Buffer *vImage_source, img_prop image_prperties, float compression, char *file_name);

double calcImageLongSide (int shortside, int sShort, int sLong);

double calcImageShortSide (int longside, int sShort, int sLong);

void print_vBuff (vImage_Buffer *buffer);

void setupBuffer (vImage_Buffer *buffer, int height, int width, int rowBytes);

void* convert_space (const void *source_data_ptr, int width, int height);

void null_ip (img_prop image_prperties);

void print_ip (img_prop image_prperties);

// Returns a malloced char* that must be freed
char* get_out_filename (char *file_name, args cli_flags);

void reset_vImage (vImage_Buffer *source, vImage_Buffer *output, img_prop image_prperties);

void pop_img_props (CGImageRef source_image, img_prop im_props);
