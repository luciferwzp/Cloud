#include"httplib.h"
#include"compress.hpp"
#include<iostream>
#include<fstream>
#include<sstream>
#include<boost/filesystem.hpp>
#include<unistd.h>
#include<fcntl.h>

using namespace httplib;                                                                                                        
namespace bf = boost::filesystem;

#define SERVER_BASE_DIR "www"
#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 9000
#define SERVER_BACKUP_DIR SERVER_BASE_DIR"/list/"


CompressStore cstore;

class CloudServer
{
    private:
        httplib::Server srv;
    public:
        CloudServer(const char *cert, const char *key){
            bf::path base_path(SERVER_BASE_DIR);
            if(!bf::exists(base_path))
            {
                std::cout << SERVER_BASE_DIR << std::endl;
                bf::create_directory(base_path);
            }
            bf::path list_path(SERVER_BACKUP_DIR);     
            if(!bf::exists(list_path))
            {
                bf::create_directory(list_path);
            }
        }

        bool Start()
        {
            srv.set_base_dir(SERVER_BASE_DIR);
            srv.Get("/(list(/)){0,1}{0,1}", GetFileList);
            srv.Get("/list/(.*)",GetFileData);
            srv.Put("/list/(.*)", PutFileData);
            srv.listen(SERVER_ADDR, SERVER_PORT);
            return true; 
        }

    private:
        //分块上传数据
        static void PutFileData(const httplib:: Request &req, httplib::Response &rsp)
        {
            if(!req.has_header("Range"))
            {
                rsp.status = 400;
                return;
            }
            std::string range = req.get_header_value("Range");
            int64_t range_start;
            if(RangeParse(range, range_start) == false)
            {
                rsp.status = 400;
                return;
            }
            std::cout << "backup file:[ " << req.path << " ] range: [ " << range << " ]\n";  
            std::string realpath = SERVER_BASE_DIR + req.path;
            cstore.SetFileData(realpath, req.body, range_start);

            return;
        }

        static bool RangeParse(std::string &range, int64_t &start)
        {
            //range geshi
            //Range: bytes=start-end
            size_t pos1 = range.find("=");
            size_t pos2 = range.find("-");
            if(pos1 == std::string::npos || pos2 == std::string::npos)
            {
                std::cerr << "range:["<< range <<"] format error\n";
                return false;
            }
            std::stringstream rs;
            rs << range.substr(pos1 + 1, pos2 - pos1 - 1);
            rs >> start;
            return true;
        }

        //获取列表信息
        static void GetFileList(const httplib::Request &req, httplib::Response &rsp)
        {
            std::vector<std::string> list;
            cstore.GetFileList(list);

            std::string body;
            body = "<html><body><ol>";
            for(auto i : list)
            {
                //<h4><li><a href= '/list/filename'>filename</a></li></h4>
                bf::path path(i);
                std::string file = path.filename().string();
                std::string uri = "/lsit/" + file;
                body += "<h4><li>";//li 编号
                body += "<a href= '";
                body += uri;
                body += "'>";
                body += file;
                body += "</a>";
                body += "</li></h4>";
            }
            body += "<hr /></ol></body></html>";
            rsp.set_content(&body[0], "text/html");
            return; 
        }

        //文件下载
        static void GetFileData(const httplib::Request &req, httplib::Response &rsp)
        {
            std::string body;
            std::string real = SERVER_BASE_DIR + req.path;
            cstore.GetFileData(real, body);

            rsp.set_content(body, "text/plain");//正文只能设置一次，第二次会覆盖
        }   
};

        //使用线程将低热度文件压缩存储
        void thr_start()
        {
            cstore.LowHeatFileStore();
        }

int main()
{
    std::thread thr(thr_start);
    thr.detach();
    CloudServer srv("./cert.pem","./key.pem");
    srv.Start();
    return 0;
}


