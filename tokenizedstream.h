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

#ifndef TOKENIZEDSTREAM_H
#define TOKENIZEDSTREAM_H

#include <boost/asio.hpp>
#include <string>
#include <cstdint>

namespace Beanstalkpp {

/**
 * Reads values in a tokenized way from a beanstalk server. 
 * 
 * Spaces, \r and \n are considered to be token delimiters.
 */
class TokenizedStream {
public:
  /**
   * Creates a new token reader stream.
   * 
   * @param s The socket the stream will operate on
   */
  TokenizedStream(boost::asio::ip::tcp::socket &s);
  
  /**
   * Treat the next token as a string and return it
   * 
   * @throws ServerException If there are no more characters left in this token string
   */
  std::string nextString();
  
  /**
   * Makes sure that the next token matches @p expected
   * 
   * @param expected Make sure the next token matches this string
   * 
   * @throws ServerException With reason BAD_FORMAT if the next token didn't match
   */
  void expectString(const std::string &expected);
  
  /**
   * Treat the next token as an integer and return it
   * 
   * @throws ServerException If the next token is not an integer
   * @throws ServerException If there are no more characters left in this token string
   */
  unsigned int expectInt();

  /**
   * Treat the next token as an unsigned long long and return it
   * 
   * @throws ServerException If the next token is not an unsigned long long 
   * @throws ServerException If there are no more characters left in this token string
   */
  uint64_t expectULL();
  
  /**
   * Expects the next token to be \r\n
   * 
   * @throws ServerException If the next token is not \r\n
   * @throws ServerException If there are no more characters left in this token string
   */
  void expectEol();
  
  /**
   * Reads @p bytes from the stream as raw data and returns them. The receiving function will be 
   * responsible for freeing the returned memory.
   * 
   * @param bytes Read this many bytes. The call will block until the number of bytes are read.
   * 
   * @throws ServerException If EOF is hit in the stream
   * @throws ServerException On other network errors
   */
  char *readChunk(size_t bytes);
private:
  /**
   * Our current buffer we store the data in
   */
  boost::asio::streambuf socketBuffer;
  
  boost::asio::ip::tcp::socket &socket;
};

}

#endif

