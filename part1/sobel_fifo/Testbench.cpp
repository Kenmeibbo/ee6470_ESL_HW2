#include <iostream>  // 包含iostream頭文件以使用std::cout
#include <string>    // 包含string頭文件以使用string類型

#include <cassert>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include "Testbench.h"

unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), initiator("initiator"), output_rgb_raw_data_offset(54) { ///初始化
  SC_THREAD(do_sobel);
  //sensitive << i_clk.pos();  /// positive trigger, for thread-do_sobel
  //dont_initialize();       
}

int Testbench::read_bmp(string infile_name) {
  ///std::cout << "Reading BMP file: " << infile_name << std::endl;

  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name.c_str(), "rb");  /// read binary
  if (fp_s == NULL) {                   /// check whether the file open successfully
    printf("fopen %s error\n", infile_name.c_str());
    return -1;
  }
  // move offset to 10 to find rgb raw data offset
  fseek(fp_s, 10, SEEK_SET);
  assert(fread(&input_rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));  /// read and save in input_rgb_raw_data_offset

  // move offset to 18 to get width & height;
  fseek(fp_s, 18, SEEK_SET);
  assert(fread(&width, sizeof(unsigned int), 1, fp_s));
  assert(fread(&height, sizeof(unsigned int), 1, fp_s));

  // get bit per pixel
  fseek(fp_s, 28, SEEK_SET);
  assert(fread(&bits_per_pixel, sizeof(unsigned short), 1, fp_s));
  bytes_per_pixel = bits_per_pixel / 8;

  // move offset to input_rgb_raw_data_offset to get RGB raw data
  fseek(fp_s, input_rgb_raw_data_offset, SEEK_SET);

  source_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (source_bitmap == NULL) {
    printf("malloc images_s error\n");
    return -1;
  }

  target_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (target_bitmap == NULL) {
    printf("malloc target_bitmap error\n");
    return -1;
  }

  printf("Image width=%d, height=%d\n", width, height);
  assert(fread(source_bitmap, sizeof(unsigned char),            /// from fp_s read file and save to source_bitmap 
               (size_t)(long)width * height * bytes_per_pixel, fp_s));
  fclose(fp_s);
  return 0;
}

int Testbench::write_bmp(string outfile_name) {
  FILE *fp_t = NULL;      // target file handler
  unsigned int file_size; // file size

  fp_t = fopen(outfile_name.c_str(), "wb");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
    return -1;
  }

  // file size
  file_size = width * height * bytes_per_pixel + output_rgb_raw_data_offset; /// count the file size
  header[2] = (unsigned char)(file_size & 0x000000ff);    /// write file size into header
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bits_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), output_rgb_raw_data_offset, fp_t);

  // write image
  fwrite(target_bitmap, sizeof(unsigned char),
         (size_t)(long)width * height * bytes_per_pixel, fp_t);

  fclose(fp_t);
  return 0;
}

void Testbench::do_sobel() {
  int x, y, v, u;        // for loop counter
  unsigned char R, G, B; // color of R, G, B
  int adjustX, adjustY, xBound, yBound;
  int total;
  //int reuse_count = 0;

  word data;
  unsigned char mask[4];
  //o_rst.write(false);
  //o_rst.write(true);
  for (y = 0; y != height; ++y) {     ///0~255
    for (x = 0; x != width; ++x) {    ///0~255
      adjustX = (MASK_X % 2) ? 1 : 0; /// 1  if MASK_X%2 != 0, adjustX = 1 then adjustX = 0
      adjustY = (MASK_Y % 2) ? 1 : 0; /// 1
      xBound = MASK_X / 2;            /// 2
      yBound = MASK_Y / 2;            /// 2

      for (v = -yBound; v != yBound + adjustY; ++v) {   //-2, -1, 0, 1, 2
        for (u = -xBound; u != xBound + adjustX; ++u) { //-2, -1, 0, 1, 2
          if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {
            //printf("(%d , %d) = {%d , %d}\n",x,y,x+u,y+v);
            R = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 2);  /// 3 * (256*...)
            ///printf("source_bitmap = %d\n",*source_bitmap); 57
            G = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
            B = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
          } 
          else {
            R = 0;
            G = 0;
            B = 0;
          }
          //printf("here is test.cpp -- ");
            //reuse_count++;
            //printf("\n\nreuse %d\n",reuse_count);
            
            // Data reuse 只讀需要讀的  x + u(from -2~2) 將他write 給 sobel.cpp read
            if(x == 0 || x+u ==  x+2 ){
              /*
              o_r.write(R);
              o_g.write(G);
              o_b.write(B);
              wait(1); //emulate channel delay
              */
              data.uc[0] = R;   /// uc為 unsigned char 將資料包在data內傳給Initiator 去 read
              data.uc[1] = G;
              data.uc[2] = B;
              mask[0] = 0xff;
              mask[1] = 0xff;
              mask[2] = 0xff;
              mask[3] = 0;
              initiator.write_to_socket(SOBEL_FILTER_R_ADDR, mask, data.uc, 4);///跟Initiator.h的read_from_socket對接
              wait(1 * CLOCK_PERIOD, SC_NS);
            }
        }
      }
      /*
      if(i_result.num_available()==0) wait(i_result.data_written_event());
      total = i_result.read();
      //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time
      */
      /// sobel_filter計算好將result傳回給test_bench
      bool done=false; ///initialize done 一開始還沒做完，所以是false
      int output_num=0;  /// 之後會將data的值傳給他(用來判斷是否有data傳來)
      while(!done){
        initiator.read_from_socket(SOBEL_FILTER_CHECK_ADDR, mask, data.uc, 4);
        output_num = data.sint; /// signed int
        if(output_num>0) done=true;
      }
      wait(10 * CLOCK_PERIOD, SC_NS);
      initiator.read_from_socket(SOBEL_FILTER_RESULT_ADDR, mask, data.uc, 4);
      total = data.sint;
      
      ///bytes_per_pixel = 3
      *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = total; ///r
      *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = total; ///g
      *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = total; ///b

      /*
      if (total - THRESHOLD >= 0) {  ///THRESHOLD = 90
        // black
        *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = BLACK;
        *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = BLACK;
        *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = BLACK;
      } else {
        // white
        *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = WHITE;
        *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = WHITE;
        *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = WHITE;
      }
      */
    }
  }
  //printf("reuse conunt %d\n",reuse_count);
  sc_stop();
}
