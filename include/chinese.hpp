#include <iostream>
#include <opencv2/opencv.hpp>
#include "include/utf8.h"


 
int test_word(std::string name, int width=50, int height=50)
{
    // refer : https://www.codeleading.com/article/25882730552/
    string strChar;
    std::vector<std::string> words;
    for(int i = 0; name[i] != '\0'; )
    {
        char chr = name[i];
        if((chr & 0x80) == 0)
        {
            strChar = name.substr(i,1);
            words.push_back(strChar);
            ++i;
        }
        else if((chr & 0xF8) == 0xF8)
        {
            strChar = name.substr(i, 5);
            words.push_back(strChar);
            i+=5;
        }
        else if((chr & 0xF0) == 0xF0)
        {
            strChar = name.substr(i, 4);
            words.push_back(strChar);
            i+=4;
        }
        else if((chr & 0xE0) == 0xE0)
        {
            strChar = name.substr(i, 3);
            words.push_back(strChar);
            i+=3;
        }
        else if((chr & 0xC0) == 0xC0)
        {
            strChar = name.substr(i, 2);
            words.push_back(strChar);
            i+=2;
        }
    }
    // 
    std::string save_dir = "/home/disk3/word";
    std::vector<cv::Mat> word_mat_vector;

    for(int i=0; i<words.size(); i++)
    {
        std::string word_path = save_dir + "/" + words[i] + ".png";
        std::cout << word_path << std::endl;
        cv::Mat word_mat = cv::imread(word_path);
        cv::Mat word_resize;
        cv::resize(word_mat, word_resize, cv::Size(height, width));
        word_mat_vector.push_back(word_resize);
    }

    // print
    for(int i=0; i<height; i++)
    {
        for(int m=0; m<word_mat_vector.size(); m++)
        {
            for(int j=0; j<width; j++)
            {
                int index = (i * width + j)*3;
                int pix_value = (int)word_mat_vector[m].data[index];
                if(pix_value > 254)
                {
                    std::cout << "  ";
                }
                else
                {
                    std::cout << "**";
                }
            }
        }
        std::cout << std::endl;
    }
}


