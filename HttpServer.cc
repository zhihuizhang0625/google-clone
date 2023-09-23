/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include "./FileReader.h"
#include "./HttpConnection.h"
#include "./HttpRequest.h"
#include "./HttpResponse.h"
#include "./HttpUtils.h"
#include "./HttpServer.h"


using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

namespace searchserver {
///////////////////////////////////////////////////////////////////////////////
// Constants, internal helper functions
///////////////////////////////////////////////////////////////////////////////
static const char *kFivegleStr =
  "<html><head><title>5950gle</title></head>\n"
  "<body style=\"background-color: #f9d5de;\">\n"
  //ffc0cb。eeb0c9。fadadd。f9d5de
  "<center style=\"font-size:500%;\">\n"
  "<span style=\"position:relative;bottom:-0.33em;color:orange;\">5</span>"
    "<span style=\"color:red;\">9</span>"
    "<span style=\"color:gold;\">5</span>"
    "<span style=\"color:blue;\">g</span>"
    "<span style=\"color:green;\">l</span>"
    "<span style=\"color:red;\">e</span>\n"
  "</center>\n"
  "<p>\n"
  "<div style=\"height:20px;\"></div>\n"
  "<center>\n"
  "<form action=\"/query\" method=\"get\">\n"
  "<input type=\"text\" size=30 name=\"terms\" />\n"
  "<input type=\"submit\" value=\"Search\" />\n"
  "</form>\n"
  "</center><p>\n";

// static
const int HttpServer::kNumThreads = 100;

// This is the function that threads are dispatched into
// in order to process new client connections.
static void HttpServer_ThrFn(ThreadPool::Task *t);

// Given a request, produce a response.
static HttpResponse ProcessRequest(const HttpRequest &req,
                            const string &base_dir,
                            WordIndex *indices);

// Process a file request.
static HttpResponse ProcessFileRequest(const string &uri,
                                const string &base_dir);

// Process a query request.
static HttpResponse ProcessQueryRequest(const string &uri,
                                 WordIndex *index);


///////////////////////////////////////////////////////////////////////////////
// HttpServer
///////////////////////////////////////////////////////////////////////////////
bool HttpServer::run(void) {
  // Create the server listening socket.
  int listen_fd;
  cout << "  creating and binding the listening socket..." << endl;
  if (!socket_.bind_and_listen(&listen_fd)) {
    cerr << endl << "Couldn't bind to the listening socket." << endl;
    return false;
  }

  // Spin, accepting connections and dispatching them.  Use a
  // threadpool to dispatch connections into their own thread.
  cout << "  accepting connections..." << endl << endl;
  ThreadPool tp(kNumThreads);
  while (1) {
    HttpServerTask *hst = new HttpServerTask(HttpServer_ThrFn);
    hst->base_dir = static_file_dir_path_;
    hst->index = index_;
    if (!socket_.accept_client(&hst->client_fd,
                    &hst->c_addr,
                    &hst->c_port,
                    &hst->c_dns,
                    &hst->s_addr,
                    &hst->s_dns)) {
      // The accept failed for some reason, so quit out of the server.
      // (Will happen when kill command is used to shut down the server.)
      break;
    }
    // The accept succeeded; dispatch it.
    tp.dispatch(hst);
  }
  return true;
}

static void HttpServer_ThrFn(ThreadPool::Task *t) {
  // Cast back our HttpServerTask structure with all of our new
  // client's information in it.
  unique_ptr<HttpServerTask> hst(static_cast<HttpServerTask *>(t));
  cout << "  client " << hst->c_dns << ":" << hst->c_port << " "
       << "(IP address " << hst->c_addr << ")" << " connected." << endl;

  // Read in the next request, process it, write the response.
  
  // Use the HttpConnection class to read and process the next
  // request from our current client, then write out our response.  If
  // the client sends a "Connection: close\r\n" header, then shut down
  // the connection -- we're done.
  //
  // Hint: the client can make multiple requests on our single connection,
  // so we should keep the connection open between requests rather than
  // creating/destroying the same connection repeatedly.

  // TODO: Implement
  bool done = false;
  HttpConnection connection(hst -> client_fd);

  while(!done){
    HttpRequest req;
      if(connection.next_request(&req) && req.GetHeaderValue("connection") != "close"){
          //process and write
          HttpResponse resp = ProcessRequest(req, hst->base_dir, hst->index);
          connection.write_response(resp);
      } else{
        break;
      } 
  } 
}

static HttpResponse ProcessRequest(const HttpRequest &req,
                            const string &base_dir,
                            WordIndex *index) {
  // Is the user asking for a static file?
  if (req.uri().substr(0, 8) == "/static/") {
    return ProcessFileRequest(req.uri(), base_dir);
  }

  // The user must be asking for a query.
  return ProcessQueryRequest(req.uri(), index);
}

static HttpResponse ProcessFileRequest(const string &uri,
                                const string &base_dir) {
  // The response we'll build up.
  HttpResponse ret;

  // Steps to follow:
  //  - use the URLParser class to figure out what filename
  //    the user is asking for. Note that we identify a request
  //    as a file request if the URI starts with '/static/'
  //
   
  
   // //build url parser
   URLParser up;
   up.parse(uri);
   string file_name = up.path().substr(8);
  //  - use the FileReader class to read the file into memory

  FileReader reader (up.path().substr(8));
  string file_content; //out parameter

  //  - copy the file content into the ret.body
  
  //  - depending on the file name suffix, set the response
  //    Content-type header as appropriate, e.g.,:
  //      --> for ".html" or ".htm", set to "text/html"
  //      --> for ".jpeg" or ".jpg", set to "image/jpeg"
  //      --> for ".png", set to "image/png"
  //      etc.
  //    You should support the file types mentioned above,
  //    as well as ".txt", ".js", ".css", ".xml", ".gif",
  //    and any other extensions to get bikeapalooza
  //    to match the solution server.
  //
  // be sure to set the response code, protocol, and message
  // in the HttpResponse as well.

   // test_tree/books/huckfinn.txt
  if(is_path_safe(base_dir, file_name) && reader.read_file(&file_content)){
    // If you found the file
    vector<string> parts;
    boost::split(parts, file_name, boost::is_any_of("."));
    //.txt
    string file_type = parts[parts.size() - 1];
    //get the last parts[len(parts) -1]
    //fill the last parts by cases
    const std::map<std::string, std::string> content_type_map = {
      {"html", "text/html"},
      {"htm", "text/html"},
      {"jpeg", "image/jpeg"},
      {"jpg", "image/jpeg"},
      {"png", "image/png"},
      {"txt", "text/plain"},
      {"js", "text/javascript"},
      {"css", "text/css"},
      {"xml", "text/xml"},
      {"gif", "image/gif"}
    };
    auto it = content_type_map.find(file_type);
    if (it != content_type_map.end()) {
      ret.set_content_type(it->second);
    } else {
      ret.set_content_type("text/plain");
    }
    //set http response to 200, OK
    ret.set_protocol("HTTP/1.1");
    ret.set_response_code(200);
    ret.set_message("OK");
    ret.AppendToBody(file_content);

  }else{
     // If you couldn't find the file, return an HTTP 404 error.
    ret.set_protocol("HTTP/1.1");
    ret.set_response_code(404);
    ret.set_message("Not Found");
    ret.AppendToBody("<html><body> \""
                    + escape_html(file_name)
                    + "\"</body></html>\n");
  }

  return ret;

}

static HttpResponse ProcessQueryRequest(const string &uri,
                                 WordIndex *index) {
  // The response we're building up.
  HttpResponse ret;

  // Your job here is to figure out how to present the user with
  // the same query interface as our solution_binaries/httpd server.
  // A couple of notes:
  //
  //  - no matter what, you need to present the 5950gle logo and the
  //    search box/button
  //
    //set http response to 200, OK
    ret.set_protocol("HTTP/1.1");
    ret.set_response_code(200);
    ret.set_message("OK");
  
  //  - if the user sent in a search query, you also
  //    need to display the search results. You can run the solution binaries to see how these should look
  
  string req(kFivegleStr);
  URLParser up;
  up.parse(uri);
  std::map<string, string> args = up.args();

   // if not find terms
  if (args.find("terms") == args.end()) {
    ret.AppendToBody(req+ "</body>\n</html>\n");
    return ret;
  }

  //  - you'll want to use the URLParser to parse the uri and extract
  //    search terms from a typed-in search query.  convert them
  //    to lower case.
  
  //  - Use the specified index to generate the query results

  // TODO: implement
  // query
  string terms = boost::to_lower_copy(args["terms"]);
  //cout<<terms<<endl;
  
  vector<string> search_terms;
  boost::split(search_terms, terms, boost::is_any_of(" "));
  
  vector<Result> results = index->lookup_query(search_terms);

  if (results.size() == 0) {
    ret.AppendToBody(req + "<p style=\"color: white;\"><br>\nNo results found for <b>" + terms + "</b>\n<p>\n\n</body>\n</html>\n");
  } else {
    // check if results found, then write a string
    req = req+ "<p style=\"color: white;\"><br>\n"+ std::to_string(results.size()) + " results found for <b>" + escape_html(terms) + "</b>\n</p>\n\n<ul>\n";
    for (Result res : results) {
      req += " <li style=\"color: white;\"> <a href=\"/static/" + res.doc_name + "\" style=\"color: black;\">" + res.doc_name + "</a> [" + std::to_string(res.rank) + "]<br>\n";
    }
    req += "</ul>\n</body>\n</html>\n";
    ret.AppendToBody(req);
  }
  return ret;
}

}  // namespace searchserver
