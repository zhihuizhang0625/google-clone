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

#include <cstdio>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <cstring>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

namespace searchserver {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}


bool ServerSocket::bind_and_listen(int *listen_fd) {
  struct addrinfo info;
  memset(&info, 0, sizeof(struct addrinfo));
  info.ai_family = AF_INET6;       // IPv6 
  info.ai_socktype = SOCK_STREAM;  // stream
  info.ai_flags = AI_PASSIVE;      // adress
  info.ai_flags |= AI_V4MAPPED;    
  info.ai_protocol = IPPROTO_TCP;  // tcp protocol
  info.ai_canonname = nullptr;
  info.ai_addr = nullptr;
  info.ai_next = nullptr;

  // get address info
  char portStr[6];
  sprintf(portStr, "%u", port_);
  struct addrinfo *result;

  int status = getaddrinfo(nullptr, portStr, &info, &result);
  if (status != 0) {
    std::cerr << "getaddrinfo() failed: " << gai_strerror(status) << std::endl;
    return EXIT_FAILURE;
  }

  // create socket and bind
  struct addrinfo *rp = result;
  while (rp != nullptr) {
    listen_sock_fd_ = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    //set the return parameter
    *listen_fd = listen_sock_fd_;
    if (listen_sock_fd_ == -1) {
      std::cerr << "socket() failed: " << strerror(errno) << std::endl;
      continue;
    }

    int opt = 1;
    setsockopt(listen_sock_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    //success

    if (bind(listen_sock_fd_, rp->ai_addr, rp->ai_addrlen) == 0) {
        
        std::cout << "Socket [" << listen_sock_fd_ << "] is bound to:" << std::endl;
          if (rp->ai_addr->sa_family == AF_INET6) {

            // Print out the IPV6 address and port
            char astring[INET6_ADDRSTRLEN];
            struct sockaddr_in6 *in6 = reinterpret_cast<struct sockaddr_in6 *>(rp->ai_addr);
            inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
            std::cout << " IPv6 address " << astring;
            std::cout << " and port " << ntohs(in6->sin6_port) << std::endl;

          } else {
            std::cout << " ???? address and port ????" << std::endl;
        }
      break;
    }

    close(listen_sock_fd_);
    listen_sock_fd_ = -1;
    rp = rp->ai_next;
  }
  //free the data structure
  freeaddrinfo(result);

  if (listen_sock_fd_ == -1) {
    std::cerr << "Couldn't bind to any addresses." << std::endl;
    return false;

  }else if (listen(listen_sock_fd_, SOMAXCONN) != 0) {
    // success
    std::cerr << "Failed to mark socket as listening: " << strerror(errno) << std::endl;
    close(listen_sock_fd_);
    listen_sock_fd_ = -1;
    return false;

  }else{
    *listen_fd = listen_sock_fd_;
    return true;

  }


  
 }
 
  


bool ServerSocket::accept_client(int *accepted_fd,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dns_name,
                          std::string *server_addr,
                          std::string *server_dns_name) const {


 
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // TODO: implement

  
  if (listen_sock_fd_ <= 0) {
    // We failed to bind/listen to a socket.  Quit with failure.
    std::cerr << "Couldn't bind to any addresses." << std::endl;
    return false;
  }


  while (1) {
    struct sockaddr_storage client_addr_stor;
    socklen_t caddr_len = sizeof(client_addr_stor);
    *accepted_fd = accept(listen_sock_fd_,
                           reinterpret_cast<struct sockaddr *>(&client_addr_stor),
                           &caddr_len);

    int client_fd = *accepted_fd;
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
        continue;
      std::cerr << "Failure on accept: " << strerror(errno) << std::endl;
      break;
    }

    //client addr
    char client_addr_str[INET6_ADDRSTRLEN];
        struct sockaddr_in6 *s = reinterpret_cast<struct sockaddr_in6 *> (&client_addr_stor);
        inet_ntop(AF_INET6, &s->sin6_addr, client_addr_str, INET6_ADDRSTRLEN);
        *client_port = ntohs(s->sin6_port);
        *client_addr = client_addr_str;
  

  // get client dns name
      char client_dns_name_str[NI_MAXHOST];
      int res = getnameinfo(reinterpret_cast<struct sockaddr*>(&client_addr_stor),
                            caddr_len,
                            client_dns_name_str,
                            NI_MAXHOST,
                            nullptr,
                            0,
                            0);
      if (res != 0) {
        sprintf(client_dns_name_str, "[reverse DNS failed]");
      }
      *client_dns_name = client_dns_name_str;

// get server address and port
      char hname[1024];
      hname[0] = '\0';

      //get srver dns name
        struct sockaddr_in6 srvr;
        socklen_t srvrlen = sizeof(srvr);
        char addrbuf[INET6_ADDRSTRLEN];
        getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
        inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
        *server_addr = std::string(addrbuf);
        // Get the server's dns name
        getnameinfo((const struct sockaddr *) &srvr,
                    srvrlen, hname, 1024, nullptr, 0, 0);
        *server_dns_name = std::string(hname);

        break;
    }
       
    
  
  return true;

}





}  // namespace searchserver


