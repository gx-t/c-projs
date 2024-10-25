#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <setjmp.h>

// Error handling struct
struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

// Custom error handler
METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

int convert_to_progressive(const char *input_path, const char *output_path)
{
    struct jpeg_decompress_struct srcinfo;
    struct jpeg_compress_struct dstinfo;
    struct my_error_mgr jsrcerr, jdsterr;
    FILE *input_file;
    FILE *output_file;
    jvirt_barray_ptr *src_coef_arrays;

    // Open input file
    if ((input_file = fopen(input_path, "rb")) == NULL) {
        fprintf(stderr, "Cannot open %s for reading\n", input_path);
        return 0;
    }

    // Open output file
    if ((output_file = fopen(output_path, "wb")) == NULL) {
        fprintf(stderr, "Cannot open %s for writing\n", output_path);
        fclose(input_file);
        return 0;
    }

    // Initialize error handling for decompression
    srcinfo.err = jpeg_std_error(&jsrcerr.pub);
    jsrcerr.pub.error_exit = my_error_exit;
    if (setjmp(jsrcerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&srcinfo);
        fclose(input_file);
        fclose(output_file);
        return 0;
    }

    // Initialize error handling for compression
    dstinfo.err = jpeg_std_error(&jdsterr.pub);
    jdsterr.pub.error_exit = my_error_exit;
    if (setjmp(jdsterr.setjmp_buffer)) {
        jpeg_destroy_compress(&dstinfo);
        jpeg_destroy_decompress(&srcinfo);
        fclose(input_file);
        fclose(output_file);
        return 0;
    }

    // Initialize JPEG decompression object
    jpeg_create_decompress(&srcinfo);
    jpeg_create_compress(&dstinfo);

    // Specify data source for decompression
    jpeg_stdio_src(&srcinfo, input_file);
    // Specify data destination for compression
    jpeg_stdio_dest(&dstinfo, output_file);

    // Read file header
    jpeg_read_header(&srcinfo, TRUE);

    // Read source file as DCT coefficients
    src_coef_arrays = jpeg_read_coefficients(&srcinfo);

    // Copy critical parameters from source
    jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

    // Set progressive mode
    jpeg_simple_progression(&dstinfo);

    // Start compression
    jpeg_write_coefficients(&dstinfo, src_coef_arrays);

    // Finish compression and decompression
    jpeg_finish_compress(&dstinfo);
    jpeg_destroy_compress(&dstinfo);
    jpeg_finish_decompress(&srcinfo);
    jpeg_destroy_decompress(&srcinfo);

    // Close files
    fclose(input_file);
    fclose(output_file);

    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.jpg output.jpg\n", argv[0]);
        return 1;
    }

    if (!convert_to_progressive(argv[1], argv[2])) {
        fprintf(stderr, "Failed to convert image\n");
        return 1;
    }

    printf("Successfully converted %s to progressive JPEG\n", argv[1]);
    return 0;
}
