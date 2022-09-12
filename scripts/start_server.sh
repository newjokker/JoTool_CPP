#!/bin/bash


dir="/usr/load_ucd_dir"
if [ ! -d "$dir" ];then
mkdir $dir
fi

# 可以根据传入的属性决定起来几个

./dete_server /usr/save_xml_dir /usr/ucd_cache_dir /usr/dete_log_dir_1 /usr/load_ucd_dir/load_ucd_dir_1 prebase_yolo5_0_4_0 ./config.ini prebase_0_5_0 192.168.3.111 11101 &

./dete_server /usr/save_xml_dir /usr/ucd_cache_dir /usr/dete_log_dir_1 /usr/load_ucd_dir/load_ucd_dir_2 prebase_yolo5_0_4_0 ./config.ini prebase_0_5_0 192.168.3.111 11101 &

./dete_server /usr/save_xml_dir /usr/ucd_cache_dir /usr/dete_log_dir_1 /usr/load_ucd_dir/load_ucd_dir_3 prebase_yolo5_0_4_0 ./config.ini prebase_0_5_0 192.168.3.111 11101 &

./dete_server /usr/save_xml_dir /usr/ucd_cache_dir /usr/dete_log_dir_1 /usr/load_ucd_dir/load_ucd_dir_4 prebase_yolo5_0_4_0 ./config.ini prebase_0_5_0 192.168.3.111 11101 &

./dete_server /usr/save_xml_dir /usr/ucd_cache_dir /usr/dete_log_dir_1 /usr/load_ucd_dir/load_ucd_dir_5 prebase_yolo5_0_4_0 ./config.ini prebase_0_5_0 192.168.3.111 11101 &

