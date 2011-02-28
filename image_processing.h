#include <Accelerate/Accelerate.h>
#include <ApplicationServices/ApplicationServices.h>

typedef struct {
	int image_rot;
	int image_w;
	int image_h;
	int bits_ppixel;
	int bytes_row;
} image_properties; // image properties

typedef image_properties *img_prop;

// This function takes the command line arguments and 1 file name
void process_1_image (args cli_flags, char *file_name);

/****************************/
/****** Get EXIF data *******/
/****************************/
int get_exif_rot (CGDataProviderRef image_source);

void flip_image (vImage_Buffer *vImage_source, img_prop image_prperties, args cli_flags);

void rotate_image (vImage_Buffer *vImage_source, img_prop image_prperties, args cli_flags);

void resize_image (vImage_Buffer *vImage_source, img_prop image_prperties, args cli_flags);

void save_image (vImage_Buffer *vImage_source, img_prop image_prperties, float compression, char *file_name);

double calcImageLongSide (int shortside, int sShort, int sLong);

double calcImageShortSide (int longside, int sShort, int sLong);

void print_vBuff (vImage_Buffer *buffer);

void setupBuffer (vImage_Buffer *buffer, int height, int width, int rowBytes);

void null_ip (img_prop image_prperties);

void print_ip (img_prop image_prperties);

// Returns a malloced char* that must be freed
char* get_out_filename (char *file_name, args cli_flags);

void reset_vImage (vImage_Buffer *source, vImage_Buffer *output, img_prop image_prperties);

void pop_img_props (CGImageRef source_image, img_prop im_props);
