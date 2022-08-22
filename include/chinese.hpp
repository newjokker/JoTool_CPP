#include <iostream>
#include <fstream>
#include <memory>
#include <opencv2/opencv.hpp>


std::unique_ptr<char[]> unicode_to_utf8(uint16_t unicode) {
  const int k_buffer_length = 3;
  auto buffer = std::make_unique<char[]>(k_buffer_length);
  buffer[0] = 0xE0 | (unicode >> 12);
  buffer[1] = 0x80 | ((unicode >> 6) & 0x3F);
  buffer[2] = 0x80 | (unicode & 0x3F);
  return buffer;
}

void word() {
  const auto k_chinese_encoding_begin = 0x4E00;
  const auto k_chinese_encoding_end = 0x9FFF;
  const auto k_output_filename = "test_01.txt";

  std::ofstream out(k_output_filename, std::ios::out | std::ios::binary);
  for (int i = k_chinese_encoding_begin; i <= k_chinese_encoding_end; i++) {
    uint16_t unicode = i;
    auto buffer = unicode_to_utf8(unicode);
    out.write(buffer.get(), 3);
    out.write("\n", 1);
  }

  out.close();
}

void test_word(std::string name) 
{

    std::string save_dir = "/home/disk3/word";
    std::string word_path = save_dir + "/" + name + ".png";
    // read word
    cv::Mat word_mat = cv::imread(word_path);
    // 

    int height = word_mat.rows;
    int width = word_mat.cols;
    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            int index = (i * width + j)*3;
            int pix_value = (int)word_mat.data[index];
            if(pix_value > 254)
            {
                std::cout << "  ";
            }
            else
            {
                std::cout << "**";
            }
        }
        std::cout << std::endl;
    }


    std::cout << word_mat.size() << std::endl;


//     std::ofstream out(word_path, std::ios::out | std::ios::binary);
//     out.write(name.c_str(), 3);
//     out.write("\n", 1);
//   out.close();
}