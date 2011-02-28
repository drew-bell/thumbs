/*** DISCLAIMER ***/
I am a hobby programmer, and not a very good one. This program works as far as my testing goes.
I make no promises and take NO RESPONSIBILITY for anything that you do with it or that happens during it's use. That said, i wouldn't expect any trouble. Just be sure to back up your images until your happy with the results.

resize - a small utility for resizing images quickly from the cli.

This program only works with jpeg files at present. I built it to quickly resize large numbers of images to a desired maximum height or width, whichever is longer.

At present it does not install by itself: I haven't added that to the build process yet.

If you do not specify a subdirectory name for the scaled files, it will use "Thumbnails".

Images are output using a set quality of 80%. The ability to change the JPEG quality will be added in a later release.

ICC profiles and EXIF are removed from the scaled file. This was done on purpose to achieve a smaller file size for web galleries. I may add the ability to retain them in the future.

Scaling is done using the Lanczos5 filter provided by Apple. This is their "High quality resize" filter.

resize successfully compiles on PPC and x86. It does not run on PPC though. For some reason it segfaults. I have not yet worked out why. I think that CGImageProvider is not loading the complete image from disk.

Known Issues
	- resized images do not seem as sharp as they should be. Perhaps I have made a settings error somewhere.
	- Segfaults if the input image is square.