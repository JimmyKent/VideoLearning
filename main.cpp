#include <iostream>
using namespace std;

int simplest_yuv420_split(char *url, int w, int h, int num);
int simplest_yuv420_gray(char *url, int w, int h, int num);
int simplest_yuv420_halfy(char *url, int w, int h, int num);

int simplest_yuv444_split(char *url, int w, int h, int num);

// ffplay 打开yuv格式文件 yuv420p 编码格式 s size i input
// ffplay -f rawvideo -pix_fmt yuv420p -s 640x360 -i graybar_640x360.yuv

// ffplay 打开yuv格式文件 yuv420p 编码格式 video_size size i input
// ffplay -f rawvideo -pixel_format yuv420p -video_size 640x360 -i graybar_640x360.yuv

// ffplay打开yuv分量
// ffplay -f rawvideo -pixel_format gray -video_size 128*128 -i output_420_v.y

//ffplay -f rawvideo -pixel_format yuv420p -video_size 256x256 output_gray.yuv

//ffplay -f rawvideo -pixel_format yuv420p -video_size 256x256 output_half.yuv

int main() {
	std::cout << "start split yuv file y, u, v\n";

	//分离yuv
	simplest_yuv420_split((char *)"lena_256x256_yuv420p.yuv", 256, 256, 1);
	//取灰色
	simplest_yuv420_gray((char *)"lena_256x256_yuv420p.yuv", 256, 256, 1);

	//亮度减半
	simplest_yuv420_halfy((char *)"lena_256x256_yuv420p.yuv", 256, 256, 1);

	simplest_yuv444_split((char *)"lena_256x256_yuv444p.yuv", 256, 256, 1);

	return 0;
}