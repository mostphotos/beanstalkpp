// Copyright (C) 2011 Mostphotos AB
// 
// Author(s):
// Andreas Andersen <andreas@mostphotos.com>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License (COPYING.txt) for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "tokenizedstream.h"

#include <boost/regex.h>
#include <iostream>

#include "exception.h"
#include "serverexception.h"

using namespace std;

typedef boost::asio::buffers_iterator<
    boost::asio::streambuf::const_buffers_type> boost_iterator;

std::pair<boost_iterator, bool> match_token_delimiter(boost_iterator begin, boost_iterator end) {
  boost_iterator i = begin;
  while (i != end) {
    switch(*i++) {
      case ' ':
      case '\r':
      case '\n':
        return std::make_pair(i, true);
      default:
        continue;
    }
  }
  return std::make_pair(i, false);
}

Beanstalkpp::TokenizedStream::TokenizedStream(boost::asio::ip::tcp::socket &s): 
  socket(s) {

}

std::string Beanstalkpp::TokenizedStream::nextString() {
  boost::system::error_code error;
  string line;
  
  boost::asio::read_until(
    this->socket, this->socketBuffer, match_token_delimiter, error
  ); 
  if(error && error != boost::asio::error::eof)
    throw ServerException(ServerException::BAD_FORMAT, "Unable to read from socket");
  
  istream strResponse(&this->socketBuffer);
  strResponse >> line;
  
  return line;
}

void Beanstalkpp::TokenizedStream::expectString(const std::string& expected) {
  string s = this->nextString();
  if(s.compare(expected) != 0)
    throw ServerException(ServerException::BAD_FORMAT, "Expected '" + expected + "' but got: " + s);
}

unsigned int Beanstalkpp::TokenizedStream::expectInt() {
  string s = this->nextString();
  unsigned int ret;
  
  istringstream stream(s);
  if(!(stream >> ret)) 
    throw ServerException(ServerException::BAD_FORMAT, "Expected integer but got: " + s);
  
  return ret;
}

void Beanstalkpp::TokenizedStream::expectEol() {
  char *buf = this->readChunk(2);
  
  try {
    if(buf[0] != '\r') throw ServerException(ServerException::BAD_FORMAT, "Expected \\r\\n");
    if(buf[1] != '\n') throw ServerException(ServerException::BAD_FORMAT, "Expected \\r\\n");
    delete[] buf;
  } catch(...) {
    delete[] buf;
    throw;
  }
}

char* Beanstalkpp::TokenizedStream::readChunk ( size_t bytes ) {
  char *buf = new char[bytes];
  char *bufp = buf;
  size_t read = 0, totalRead = 0;
  
  while(totalRead < bytes) {
    size_t avail = this->socketBuffer.in_avail();
    if(avail == 0) {
      boost::asio::read(
        this->socket, this->socketBuffer, boost::asio::transfer_at_least(bytes - totalRead)
      ); 
    }
    read = this->socketBuffer.sgetn(bufp, bytes - read);

    if(read == 0) 
      throw ServerException(ServerException::BAD_FORMAT, "Got 0 bytes from the server");
    
    bufp += read;
    totalRead += read;
  }
  
  return buf;
}
