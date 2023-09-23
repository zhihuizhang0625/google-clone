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

#include <cstdint>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace searchserver {

static const char *kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::next_request(HttpRequest *request) {
  // Use "wrapped_read" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header.
  //
  // Once you've seen the request header, use parse_request()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes next_request()!

  // TODO: implement
  size_t next = buffer_.find(kHeaderEnd, 0, kHeaderEndLen);
  while (next == string::npos) {
    wrapped_read(fd_, &buffer_);
    next = buffer_.find(kHeaderEnd, 0, kHeaderEndLen);
  }
  string s = buffer_.substr(0, next + kHeaderEndLen);
  buffer_.erase(0, next + kHeaderEndLen);
  return parse_request(s, request);
}

bool HttpConnection::write_response(const HttpResponse &response) {
  // Implement so that the response is converted to a string
  // and written out to the socket for this connection

  // TODO: implement
  return wrapped_write(fd_, response.GenerateResponseString()) >= 0;
}

bool HttpConnection::parse_request(const string &request, HttpRequest* out) {
  HttpRequest req("/");  // by default, get "/".

  // Split the request into lines.  Extract the URI from the first line
  // and store it in req.URI.  For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
  // at HttpRequest.h for details about the HTTP header format that
  // you need to parse.
  //
  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.
  //
  // If a request is malfrormed, return false, otherwise true and 
  // the parsed request is retrned via *out
  
  // TODO: implement


// GET [URI] [http_protocol]\r\n
// [headername]: [headerval]\r\n
// [headername]: [headerval]\r\n
// ... more headers ...
// [headername]: [headerval]\r\n
// \r\n

  vector<string> lines_vector;
  boost::split(lines_vector, request, boost::is_any_of("\r\n"));
  if (lines_vector.empty()) return false;

// GET /foo/bar?baz=bam HTTP/1.1\r\n

  vector<string> uri_vector;
  boost::split(uri_vector, lines_vector[0], boost::is_any_of(" "));
  if (uri_vector.size() < 3) return false;
  req.set_uri(uri_vector[1]);;  

// Host: www.news.com\r\n
  for (size_t i = 1; i < lines_vector.size(); i++) {
    if (lines_vector[i].empty()) continue;
    vector<string> header_vector;
    boost::split(header_vector, lines_vector[i], boost::is_any_of(":"));
    if (header_vector.size() < 2) return false;
    req.AddHeader(boost::trim_copy(boost::to_lower_copy(header_vector[0])), boost::trim_copy(boost::to_lower_copy(header_vector[1])));
  }
  *out = req;
  return true;
}

}  // namespace searchserver
