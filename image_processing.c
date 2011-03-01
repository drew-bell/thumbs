#include <stdio.h>
#include "args.h"
#include <libgen.h>
#include "functions.h"
#include "image_processing.h"
#include <Accelerate/Accelerate.h>
#include <ApplicationServices/ApplicationServices.h>
#include <sys/stat.h>

#define no 0

CFDataRef convert32_24bit (CFDataRef source_data_ptr, img_prop im_props) {
	
	// create the variable for the 32bit image.
	UInt8 *new_24bit = NULL;
	UInt8 *colour = NULL;
	UInt8 *new_ptr = NULL;
	int i = 0;
	
	// valloc the memory necessary to store the image in.
	// we are multiplying height by width by 3 (RGB) because we want a 24 bit image
	new_24bit = valloc (im_props->image_h * im_props->image_w * 3);

	// Get the pointer to the actual data
	colour = (UInt8 *) CFDataGetBytePtr (source_data_ptr);

	// set a pointer to the first array value
	new_ptr = &new_24bit[0];

	// parse through the array, removing the alpha chanel to the new image
	while (i < im_props->image_h * im_props->image_w * 3) {

		new_ptr[i] = *colour++; //R
		new_ptr[i+1] = *colour++; //G
		new_ptr[i+2] = *colour++; //B
		i = i + 3; // Increment i
		colour = colour++; // Skip alpha
	}

	// create a CFDataRef from the modified image
	CFDataRef ret_val = CFDataCreate (NULL, (UInt8 *) new_24bit, im_props->image_h * im_props->image_w * 3);

	// update the image properties
	im_props->bits_ppixel = 24;
	im_props->bytes_row = im_props->image_w * 3;

	// release the old image
	CFRelease (source_data_ptr);
	source_data_ptr = NULL;
	
	free(new_24bit);
	new_24bit = NULL;

	// return the new image
	return ret_val;
	
}

CFDataRef convert24_32bit (CFDataRef source_data_ptr, img_prop im_props) {
	
	// create the variable for the 32bit image.
	UInt8 *new_32bit = NULL;
	UInt8 *colour;
	UInt8 *new_ptr;
	int i = 0;

	// Malloc the memory necessary to store the image in.
	// we are multiplying height by width by 4 (RGBA) because we want a 32 bit image
	new_32bit = valloc (im_props->image_h * im_props->image_w * 4);

	// Get the pointer to the actual data
	colour = (UInt8 *) CFDataGetBytePtr (source_data_ptr);

	// set a pointer to the first array value
	new_ptr = &new_32bit[0];

	// parse through the array, adding the alpha chanel to the new image
	while (i<im_props->image_h*im_props->image_w*4) {

		new_ptr[i] = *colour++; //R
		new_ptr[i+1] = *colour++; //G
		new_ptr[i+2] = *colour++; //B
		new_ptr[i+3] = 255; //A
		i = i + 4; // Increment i
	}
	
	// create a CFDataRef from the modified image
	CFDataRef ret_val = CFDataCreate (NULL, (UInt8 *)new_32bit, im_props->image_h * im_props->image_w * 4);

	// update the image properties
	im_props->bits_ppixel = 32;
	im_props->bytes_row = im_props->image_w * 4;

	// release the old image
	CFRelease (source_data_ptr);
	source_data_ptr = NULL;
	
	// return the new image
	return ret_val;
}

void process_1_image (args cli_flags, char *files) {
	
	char *out_file_name = get_out_filename (files, cli_flags);
	
	if (file_exists (out_file_name)) {
		printf ("| Output file %s already exists. skipping... ", out_file_name);
		return;
	}
	
	// Origional Image Properties struct
	img_prop o = (img_prop) malloc (sizeof (image_properties));
	
	// set all the vales in the imapg properties struct to -1
	null_ip (o);
	
	// Create a data provider
	CGDataProviderRef source_image_provider = CGDataProviderCreateWithFilename (files);
	
	// Check for a null returned value
	if (source_image_provider == NULL) {
		
		// something went wrong 
		printf ("error: Couldn't create CGDataProvider from URL.\n"); 
		exit (0);
	}
	
	// get the information from the image exif here
	o->image_rot = get_exif_rot (source_image_provider);
	

	// Create the image in memory from the JPEG data
	CGImageRef source_image = CGImageCreateWithJPEGDataProvider (source_image_provider, NULL, no, kCGRenderingIntentDefault);
	
  /********************************************/
  /* Getting the colour space **/
  
  o->colorSpace = CGImageGetColorSpace(source_image);
  
  /********************************************/
  
	// populate the image info struct
	pop_img_props (source_image, o);
	
	// create a data provider from the decoded JPEG data
	CGDataProviderRef image_data_provider = CGImageGetDataProvider (source_image);
	
	// Create a pointer to the data section of the image in memory
	CFDataRef source_data_ptr = CGDataProviderCopyData (image_data_provider);
	
	// The vImage_Buffers we will use
	vImage_Buffer *vImage_source = (vImage_Buffer*) malloc (sizeof (vImage_Buffer));
	
	// Check for NULL
	if (NULL == vImage_source) {
		printf ("Cannot malloc vImage_source buffer\n");
		exit (0);
	}
	
	if (o->bits_ppixel == 24) {
		
		// convert from 24bit to 32bit by adding the alpha channel.
		source_data_ptr = convert24_32bit (source_data_ptr, o);
		
	}
	
	// Setup the vImage Buffer for the image
	setupBuffer (vImage_source, o->image_h, o->image_w, o->bytes_row);

	// Assign the data to the vImage Buffer for the source
	vImage_source->data = (void *) CFDataGetBytePtr (source_data_ptr);
	
	// Check for NULL
	if (vImage_source->data == NULL) 
		printf ("Unable to get the vimage.data pointer\n");

	if (o->image_rot != 1 && o->image_rot != 4 && o->image_rot != 2) // rotate the image
		rotate_image (vImage_source, o, NULL);
	
	// flip the image
	if (o->image_rot == 2  || o->image_rot == 4 || o->image_rot == 7 || o->image_rot == 5)
		flip_image (vImage_source, o, NULL);
	
	// Resize the images
	resize_image (vImage_source, o, cli_flags);

	// save the image
	save_image (vImage_source, o, cli_flags->quality, out_file_name);
	
	// Release the source provider
	CGDataProviderRelease (source_image_provider);
	source_image_provider = NULL;
	
	// Release the source image
	CGImageRelease (source_image);
	source_image = NULL;
	
	free(source_data_ptr);
//	CFRelease (source_data_ptr);
	// Free the filename created by get_out_filename ()
	free (out_file_name);
	out_file_name = NULL;
	
	// free the image properties
	free (o);
	o = NULL;
	
	// if there is info in the buffer
	if (vImage_source->data != NULL) {
		free (vImage_source->data);
		vImage_source->data = NULL;
	}
	
	// free the buffer
	free (vImage_source);
	vImage_source = NULL;

} // Process 1 image

void print_ip (img_prop p) {
	
	printf ("rotation = %d \t| height = %d \t | width = %d \t | bits/pixel = %d \t | bytes/row = %d \n", 
		   p->image_rot, 
		   p->image_h, 
		   p->image_w, 
		   p->bits_ppixel,
		   p->bytes_row);
	
} // print_ip

void null_ip (img_prop p) {
	
	// set all the ints in the struct to -1
	p->image_rot = -1;
	p->image_w = -1;
	p->image_h = -1;
	p->bits_ppixel = -1;
	p->bytes_row = -1;

} // null_ip

void print_vBuff (vImage_Buffer *b) {
	
	printf ("Width = %d \t | Height = %d \t | RowBytes = %i \n",
		   (int)b->width, 
		   (int)b->height, 
		   (int)b->rowBytes);

} // Print_vBuff

void pop_img_props (CGImageRef source_image, img_prop im_props) {
	
	// Populate the remaining image property struct values
	im_props->image_w = CGImageGetWidth (source_image);
	im_props->image_h = CGImageGetHeight (source_image);
	im_props->bits_ppixel = CGImageGetBitsPerPixel (source_image);
	im_props->bytes_row = CGImageGetBytesPerRow (source_image);
} // pop_img_props 

void setupBuffer (vImage_Buffer *buffer, int h, int w, int br) {
	
	// Assign values into the buffer.
	buffer->height = h;
	buffer->width = w;
	buffer->rowBytes = br;
} // setupBuffer

// returns a malloced file name pointer
char* get_out_filename (char *file, args cf) {
	
	// the temporary filename to return
	char *out_filename = calloc (FILENAME_MAX, 1);
	char *tmp;

	// copy anything from infront of the last pat component into the outfile name
	strncpy (out_filename, file, strlen (file) - strlen (basename (file)));
	
	// because strncpy may not have termintated the string we double check
	out_filename[strlen (out_filename)] = '\0';
	
	// add the subdir if it is not null [THIS WILL NEVER BE NULL]
	if (NULL != cf->subdir) {
		
		// add the subdir to the path
		strcat (out_filename, cf->subdir);
		
		// check to see if the path already exists and create it if necessary
		if (!file_exists (out_filename))
			
			// recurrsively create sub directories
			create_dirs (out_filename);
		
		// add the trailing slash
		strcat (out_filename, "/");
	}
	
	// add the prefix if one is specified
	if (NULL != cf->prefix) {
		strcat (out_filename, cf->prefix);
	}

	// get the string
	tmp = rem_ext (basename (file));

	// copy the filename without the extention onto the end of the string
	strcat (out_filename, tmp);

	// free the string
	free(tmp);
	tmp = NULL;

	// ensure the string is terminated properly
	out_filename[strlen (out_filename)] = '\0';
	
	// append the suffix to the file name
	if (NULL != cf->suffix) {
		strcat (out_filename, cf->suffix);
	}
	
	// append the file extention to the filename
	strcat (out_filename, ".jpg");
	
	// return the a pointer to the malloced file name
	return out_filename;

} // get_out_filename ()

int get_exif_rot (CGDataProviderRef image_source) {

	int val;
	
	// Create a source to read the exif from
	CGImageSourceRef image_exif_source = CGImageSourceCreateWithDataProvider (image_source, NULL);
	
	// Check for a NULL value in the CGImageSourceRef
	if (NULL == image_exif_source) {
		
		// Could not create the CGImageSoureRef
		printf ("Could not create CGImageSourceRef for image.\n");
	}

	// NSDictionary to hold the exif from the file.
	CFDictionaryRef exif;
	
	// create an NSDictionary and populate it with the EXIF
	exif = (CFDictionaryRef) CGImageSourceCopyPropertiesAtIndex (image_exif_source, 0, NULL);
	
	// Check for a NULL value for the exif CFDict
	if (NULL == exif) {
		
		// there was no EXIF
		printf ("This file has no EXIF.\n"); 

		// Release the exif source provider
		CFRelease (image_exif_source);
		image_exif_source = NULL;

		// Release the CFDictionary
		CFRelease (exif);
		exif = NULL;

		// set the orientation to Normal.
		return 1;

	} else {

		val = CFNumberToCInt (CFDictionaryGetValue (exif, kCGImagePropertyOrientation));

		// Release the exif source provider
		CFRelease (image_exif_source);
		image_exif_source = NULL;

		// Release the CFDictionary
		CFRelease (exif);
		exif = NULL;

		// get the value at the "Orientation" tag
		return val;
	}

	// Release the exif source provider
	CFRelease (image_exif_source);
	image_exif_source = NULL;

	// Release the CFDictionary
	CFRelease (exif);
	exif = NULL;

	return 1;
	
} // get_exif ()

void reset_vImage (vImage_Buffer *s, vImage_Buffer *p, img_prop o) {
	
	// free the source data
	if (s->data != NULL) free (s->data);
	
	// Give the converted data to the source buffer
	s->data = p->data;
	
	// reset the buffer info to match the data in buffer.data
	o->image_w = s->width = p->width;
	o->image_h = s->height = p->height;
	o->bytes_row = s->rowBytes = p->rowBytes;

} // reset_vImage ()

double calcImageShortSide (int longside, int sShort, int sLong) {
	return (double)longside/ ( (double)sShort/ (double)sLong);
} // calcImageShortSide ()

double calcImageLongSide (int shortside, int sShort, int sLong) {
	return (double)shortside* ( (double)sShort/ (double)sLong);
} // calcImageLongSide ()

// flip the image either vertically or horizontally based on the rotation flag in the images EXIF
void flip_image (vImage_Buffer *vImage_source, img_prop o, args flags) {
	
	// Return error value for vImage functions
	vImage_Error error;

	// Create a buffer for processing
	vImage_Buffer *vImage_processed = (vImage_Buffer*) malloc (sizeof (vImage_Buffer));

	// Check for null
	if (vImage_processed->data == NULL) {
		printf ("Could not malloc the memory for vImage_processed->data.\n");
		exit (0);
	}	

	// setup the vimage buffers
	setupBuffer (vImage_processed, o->image_h, o->image_w, o->bytes_row);
	
	// mallocing blank data are for the processed image buffer
	vImage_processed->data = (void *) malloc (vImage_processed->rowBytes * vImage_processed->height);
	
	// Check for null
	if (vImage_processed->data == NULL) 
		printf ("Unable to get the vimage.data pointer\n");
	
	// Flipping the image.
	if (o->image_rot == 4 || o->image_rot == 7 || o->image_rot == 5) {
		printf ("Flip vertical.\n");
		error = vImageVerticalReflect_ARGB8888 (vImage_source, 
											   vImage_processed, 
											   kvImageNoFlags);	
		if (error) { 
			printf ("Flip vertical error: %d\n", (int)error);
		}
	} 
	
	// Flipping the image.
	if (o->image_rot == 2) {
		printf ("Flip Horizontal.\n");
		error = vImageHorizontalReflect_ARGB8888 (vImage_source, 
												 vImage_processed, 
												 kvImageNoFlags);	
		if (error) {
			printf ("Flip Horizontal error: %d\n", (int)error);
		}
	}

	// return the processed data to the sourcebuffer
	reset_vImage (vImage_source, vImage_processed, o);
	
	// free the processing buffer
	free (vImage_processed);
	vImage_processed = NULL;

} // flip_image ()

// rotate the image based on the rotation flag in the images EXIF
void rotate_image (vImage_Buffer *vImage_source, img_prop o, args flags) {

	// malloc space for the processed buffer
	vImage_Buffer *vImage_processed = (vImage_Buffer*) malloc (sizeof (vImage_Buffer));

	// Check for null
	if (NULL == vImage_processed) {
		printf ("Cannot malloc vImage_processed buffer\n");
		exit (0);
	}
	
	// Return error value for vImage functions
	vImage_Error error;
	
	// Make a back colour
	Pixel_8888 backColour = { (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0 };
	
	// setup vImage_processed buffer
	if (o->image_rot < 5) {

		// No change in the image dimensions
		vImage_processed->width = o->image_w;
		vImage_processed->height = o->image_h;
	} else {

		// a 90 Deg change to the image dimentions
		vImage_processed->height = o->image_w;
		vImage_processed->width = o->image_h;
	}
	
	// setup the last of the vImage_Buffer
	vImage_processed->rowBytes = vImage_processed->width*4;
	
	// malloc the memory for image processing
	vImage_processed->data = malloc (vImage_processed->rowBytes * vImage_processed->height);

	// Check for null
	if (NULL == vImage_processed->data) {
		printf ("Could not malloc the memory for vImage_processed->data.\n");
		exit (0);
	}

	// Chose the correct rotation based on the orientation flag taken from the file
	switch (o->image_rot) {
		case 3:
			printf ("\tRotating: 180 Deg .\n");

			// Rotate the image
			error = vImageRotate90_ARGB8888 (vImage_source, vImage_processed, kRotate180DegreesClockwise, backColour, kvImageNoFlags);
			if (error) { 
				printf ("Rotation Error: %d\n", (int)error);	
			}			
			break;
			
		case 5:
			printf ("\tRotating: 90 Deg Counter Clockwise - Horizontal Reflected.\n");

			// Rotate the image
			error = vImageRotate90_ARGB8888 (vImage_source, vImage_processed, kRotate90DegreesCounterClockwise, backColour, kvImageNoFlags);
			if (error) {
				printf ("Rotation Error: %d\n", (int)error);
			}
			break;
		case 6:
			printf ("\tRotating: 90 Deg Counter Clockwise.\n");

 			// Rotate the image
			error = vImageRotate90_ARGB8888 (vImage_source, vImage_processed, kRotate90DegreesClockwise, backColour, kvImageNoFlags);
			if (error) { 
				printf ("Rotation Error: %d\n", (int)error);
 			}
			break;
		case 7:
			printf ("\tRotating: 90 Deg Clockwise - Horixontal Reflected.\n");  

			// Rotate the image
			error = vImageRotate90_ARGB8888 (vImage_source, vImage_processed, kRotate90DegreesClockwise, backColour, kvImageNoFlags);
			if (error) { 
				printf ("Rotation Error: %d\n", (int)error);
			}
			break;
		case 8:
			printf ("\tRotating: 90 Deg Clockwise.\n"); 

			// Rotate the image
			error = vImageRotate90_ARGB8888 (vImage_source, vImage_processed, kRotate90DegreesCounterClockwise, backColour, kvImageNoFlags);
			if (error) { 
				printf ("Rotation Error: %d\n", (int)error);
			}
			break;
		default:
			printf ("\tNo rotation required.\n");   
			break;
	}
	
	// return the processed data to the source buffer
	reset_vImage (vImage_source, vImage_processed, o);
	
	// free the processing buffer
	free (vImage_processed);
	vImage_processed = NULL;

} // rotate_image

// resize the image to the desired output size (DEFAULT = 800px[max x or y length])
void resize_image (vImage_Buffer *vImage_source, img_prop o, args flags) {

	// create the output buffer for processing too
	vImage_Buffer *vImage_processed = (vImage_Buffer*) malloc (sizeof (vImage_Buffer));

	// Check for null
	if (NULL == vImage_processed) {
		printf ("Cannot malloc vImage_processed buffer\n");
		exit (0);
	}
	
	// Return error value for vImage functions
	vImage_Error error;
	
	// if both the width and height have been set
	if (flags->image_h != -1 && flags->image_w != -1) {
		o->image_h = flags->image_h;
		o->image_w = flags->image_w;
	}
	
	// If the output width has been set
	if (flags->image_w != -1 && flags->image_h == -1) {

		// calculate the required length or width to scale the image and apply them to the image destination attributes
		if (vImage_source->width < vImage_source->height) {
			o->image_w = flags->image_w;
			o->image_h = calcImageLongSide (flags->image_w, 
											 vImage_source->height, 
											 vImage_source->width);
		} else {
			o->image_w = flags->image_w;
			o->image_h = calcImageShortSide (flags->image_w, 
											  vImage_source->width, 
											  vImage_source->height);
		}
	}

	// If the output height has been set
	if (flags->image_h != -1 && flags->image_w == -1) {

		// calculate the required length or width to scale the image and apply them to the image destination attributes
		if (vImage_source->width < vImage_source->height) {
			o->image_h = flags->image_h;
			o->image_w = calcImageShortSide (flags->image_h, 
											  vImage_source->height, 
											  vImage_source->width);
		} else {
			o->image_h = flags->image_h;
			o->image_w = calcImageLongSide (flags->image_h, 
											 vImage_source->width, 
											 vImage_source->height);
		}
	}

	// if the max length has been set ** overides the width or height settings
	if (flags->image_l != -1) {
		
		// calculate the required length or width to scale the image and apply them to the image destination attributes
		if (vImage_source->width < vImage_source->height) {
			o->image_h = flags->image_l;
			o->image_w = calcImageShortSide (flags->image_l, 
											  vImage_source->height, 
											  vImage_source->width);
		} else {
			o->image_w = flags->image_l;
			o->image_h = calcImageShortSide (flags->image_l, 
											  vImage_source->width, 
											  vImage_source->height);
		}
	}
	
	// setup the vimage buffers
	setupBuffer (vImage_processed, o->image_h, o->image_w, o->image_w * 4);
	
	//	printf ("mallocing blank data are for the processed image buffer\n");
	vImage_processed->data = malloc (vImage_processed->rowBytes * vImage_processed->height);
	
	// Check for null
	if (NULL == vImage_processed->data) 
		printf ("Unable to get the vimage.data pointer\n");
	
	// Scale the image
	error = vImageScale_ARGB8888 (vImage_source, 
								 vImage_processed, 
								 NULL, 
								 kvImageHighQualityResampling);
	if (error) {
		printf ("Resize error: %d\n", (int) error);
	}

	// return the processed data to the sourcebuffer
	reset_vImage (vImage_source, vImage_processed, o);

	// free the processing buffer
	free (vImage_processed);
	vImage_processed = NULL;

} // resize_image

// save the image
void save_image (vImage_Buffer *src_i, img_prop o, float compression, char *o_file) {
	
	// Create a CFDataRef from the rotated data in the destination vImage_Buffer
	CFDataRef output_Data = CFDataCreate (NULL, src_i->data, src_i->height * src_i->rowBytes);
	
	if (o->bits_ppixel == 32) {
		
		// convert from 24bit to 32bit by adding the alpha channel.
		output_Data = convert32_24bit (output_Data, o);
		src_i->rowBytes = src_i->width * 3;
		
	}

	// Check for a NULL value.
	if (NULL == output_Data) {
		printf ("Could not create CFDataRef from vImage_Buffer.\n"); 
		exit (0);
	}
	
	// Create a Data provider from the rotated data
	CGDataProviderRef destination_data_provider = CGDataProviderCreateWithCFData (output_Data);
	
	// Check for null
	if (NULL == destination_data_provider) {
		printf ("Could not create CGDataProviderRef from CGDataRef.\n"); 
		exit (0);
	}
    
  	// Create a CGImageRef from the rotated data provider
  CGImageRef processed_image = CGImageCreate (src_i->width, // 1 width
                                                src_i->height, // 2 height
                                                (size_t)o->bits_ppixel/ (o->bits_ppixel/8), // bitsPerComponent
                                                (size_t)o->bits_ppixel, //bitsPerPixel
                                                src_i->rowBytes, // bytesPerRow
                                                o->colorSpace, // ColourSpace
                                                kCGBitmapByteOrder32Big, // bitmapInfo
                                                destination_data_provider, // Data provider ** DataProviderRef 
                                                NULL, // decode
                                                0, // Interpolate
                                                kCGRenderingIntentSaturation); // rendering intent
  	if (NULL == processed_image) exit (0);
    

	// create a CFStringRef from the C string
	CFStringRef fn = CFStringCreateWithCString (NULL, o_file, kCFStringEncodingUTF8);
	if (NULL == fn) exit (0);
	
	// Convert the CFStringRef to a CFURLRef
	CFURLRef fon = CFURLCreateWithFileSystemPath (NULL, fn, kCFURLPOSIXPathStyle, false);
	if (NULL == fon) exit (0);

	// Create an image destination
	CGImageDestinationRef image_destination = CGImageDestinationCreateWithURL (fon, kUTTypeJPEG, 1, NULL);

	// Release the CFURLRef
	CFRelease (fon);
	fon = NULL;
	
	// release the CFStringRef
	CFRelease (fn);
	fn = NULL;

	// Check for a NULL value in image_destination
	if (NULL == image_destination) {
		printf ("Null Image_destination: Could not create the CGImageDestinationRef from the supplied URL.\n");
		exit (0);
	}

	// Set the compression factor for the images
	CFStringRef keys[1];
	CFTypeRef value[1];
	
	// Use compression key
	keys[0] = kCGImageDestinationLossyCompressionQuality;

	// set the compression amount. 1 = no compression 0 = max compression
	value[0] = CFNumberCreate (NULL, kCFNumberFloatType, &compression);
	
	// Pointer to the image attribs dictionary
	CFDictionaryRef options;
	
	// create the dictionary
	options = CFDictionaryCreate (kCFAllocatorDefault, (void *)keys, (void *)value, 1, NULL, NULL);
	
	// Copy data to the output information to the destination
	CGImageDestinationAddImage (image_destination, processed_image, options);
	
	// Check for a NULL value in image_destination
	if (NULL == image_destination) {
		printf ("Null Image_destination: Could not add the rotated image.\n"); 
		exit (0);
	}
	
	// Write the image to disk
	if (!CGImageDestinationFinalize (image_destination)) {
		// Could not write the file for some reason
		printf ("Could not write the file to disk.\n"); 
		exit (1);
	}

	// Release the pointer the the scaled buffer
	CFRelease (output_Data);
	output_Data = NULL;
	
	// Release the dictionary
	CFRelease (keys[0]);
	CFRelease (value[0]);
	CFRelease (options);
	options = NULL;
	
	// Release the rotated image.
	CGImageRelease (processed_image);
	processed_image = NULL;
	
	// Release a data provider
	CGDataProviderRelease (destination_data_provider);
	destination_data_provider = NULL;
	
	// Release the image destination
	CFRelease (image_destination);
	image_destination = NULL;

} // save_image
