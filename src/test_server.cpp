#include "httplib.h"

void helloworld (const httplib::Request &req, httplib::Response &rsp)
{
  rsp.set_content("<html><h1>Hello world </h1></html>","text/html");
}
int main()
{
  httplib::Server srv;
  srv.set_base_dir("./wwww");

  srv.Get("/", helloworld);
  srv.listen("0.0.0.0",9000);
  return 0;
}
