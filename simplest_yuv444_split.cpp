#include <iostream>
using namespace std;



//simplest_yuv444_split("lena_256x256_yuv444p.yuv",256,256,1);

int simplest_yuv444_split(char *url, int w, int h, int num);
/**
 * Split Y, U, V planes in YUV444P file.
 * @param url  Location of YUV file.
 * @param w    Width of Input YUV file.
 * @param h    Height of Input YUV file.
 * @param num  Number of frames to process.
 *
 */
int simplest_yuv444_split(char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_444_y.y", "wb+");
    FILE *fp2 = fopen("output_444_u.y", "wb+");
    FILE *fp3 = fopen("output_444_v.y", "wb+");
    unsigned char *pic = (unsigned char *)malloc(w * h * 3);

    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3, fp);
        //Y
        fwrite(pic, 1, w * h, fp1);
        //U
        fwrite(pic + w * h, 1, w * h, fp2);
        //V
        fwrite(pic + w * h * 2, 1, w * h, fp3);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    return 0;
}