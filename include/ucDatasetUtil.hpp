

#ifndef _UCDATASETUTIL_HPP_
#define _UCDATASETUTIL_HPP_

#include <iostream>



class UCDatasetUtil
{
    public:
        //
        UCDatasetUtil(std::string host, int port);
        std::string json_path;
        std::string host;
        std::string root_url;
        int port;
        //
        void save_img_xml_json(std::string save_dir, bool need_img=true, bool need_xml=true, bool need_json=false);
        void save_ucd(std::string ucd_name, std::string save_dir);
        //
        void check_ucd();
        void delete_ucd(std::string ucd_name);
        void upload_ucd(std::string ucd_path, std::string ucd_name="");
    
    private:
        void load_file(std::string url, std::string save_path);
};





#endif