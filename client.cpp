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

#include "client.h"

#include <iostream>
#include <sstream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "serverexception.h"
#include "job.h"

using namespace std;
using namespace boost::asio::ip;

#define DEFAULT_PRIORITY 1024
#define DEFAULT_PORT 11300
// no delay
#define DEFAULT_DELAY 0 
// 1 minute
#define DEFAULT_TTR 60 

Beanstalkpp::Client::Client(const std::string& server, int port): 
  socket(io_service), tokenStream(socket) {
  this->hostname = server;
  this->port = port;
}

void Beanstalkpp::Client::connect() {
  stringstream portStr;
  portStr << this->port;
  
  tcp::resolver resolver(this->io_service);
  tcp::resolver::query query(this->hostname, portStr.str());
  tcp::resolver::iterator endpoint_iterator;
  
  try {
    endpoint_iterator = resolver.resolve(query);
    socket.connect(*endpoint_iterator);
  } catch(boost::system::system_error &e) {
    throw Exception(string("Unable to connect to beanstalk server: ") + e.what());
  }
}

int Beanstalkpp::Client::put(const std::string& data) {
  stringstream str;
  string reply;
  
  str << "put " << DEFAULT_PRIORITY << " 0 " << DEFAULT_TTR << " " << data.length() << "\r\n";
  str << data << "\r\n";
  
  this->sendCommand(str);
  reply = this->tokenStream.nextString();
  
  // "INSERTED <id>\r\n" and "BURIED <id>\r\n" are accepted commands
  if(reply.compare("INSERTED") == 0 || reply.compare("BURIED") == 0) {
    unsigned int id = this->tokenStream.expectInt();
    this->tokenStream.expectEol();
    
    return id;
  }
  
  if(reply.compare("JOB_TOO_BIG") == 0) {
    this->tokenStream.expectEol();
    stringstream err;
    err << "Job too big (" << data.length() << "b)";
    throw ServerException(ServerException::JOB_TOO_BIG, err.str());
  }
  
  // We got some other unknown command
  throw ServerException(ServerException::BAD_FORMAT, "Received bad reply to put: " + reply);
}

void Beanstalkpp::Client::use(const std::string& tubeName) {
  stringstream str;
  string reply;
  string expectedReply = "USING " + tubeName + "\r\n";
  
  this->tubeName = tubeName;
  
  str << "use " << this->tubeName << "\r\n";
  
  this->sendCommand(str);
  
  this->tokenStream.expectString("USING");
  this->tokenStream.expectString(tubeName);
  this->tokenStream.expectEol();
}

void Beanstalkpp::Client::sendCommand(const std::stringstream& str) {
  boost::system::error_code error;
  
  boost::asio::write(socket, boost::asio::buffer(str.str()), boost::asio::transfer_all(), error);
  if(error) 
    throw Exception("Unable to write to socket");
}

Beanstalkpp::Job Beanstalkpp::Client::reserve() {
  return this->reserve<Job>();
}


bool Beanstalkpp::Client::reserveWithTimeout(Beanstalkpp::job_p_t& jobPtr, int timeout) {
  return this->reserveWithTimeout<Job>(jobPtr, timeout);
}

bool Beanstalkpp::Client::peekReady(Beanstalkpp::job_p_t& jobPtr) {
  job_id_t jobId;
  size_t payloadSize;
  char *payload;
  std::stringstream s("peek-ready\r\n");
  
  this->sendCommand(s);
  
  std::string response = this->tokenStream.nextString();
  if(response.compare("NOT_FOUND") == 0) {
    this->tokenStream.expectEol();
    return false; 
  }
  
  if(response.compare("FOUND") == 0) {
    jobId = this->tokenStream.expectInt();
    payloadSize = this->tokenStream.expectInt();
    this->tokenStream.expectEol();
    
    payload = this->tokenStream.readChunk(payloadSize);
    this->tokenStream.expectEol();
    
    boost::shared_ptr<Job> newJob(new Job(*this, jobId, payloadSize, payload));
    jobPtr = newJob;
    return true;
  }
  
  throw ServerException(
    ServerException::BAD_FORMAT, 
    "Didn't get FOUND or NOT_FOUND reply to peek-ready command"
  );
}

void Beanstalkpp::Client::del(const Beanstalkpp::Job& j) {
  stringstream s;
  s << "delete " << j.getJobId() << "\r\n";
  this->sendCommand(s);
  
  this->tokenStream.expectString("DELETED");
  this->tokenStream.expectEol();
}

void Beanstalkpp::Client::del(const Beanstalkpp::job_p_t& j) {
  del(*j);
}


void Beanstalkpp::Client::bury(const Beanstalkpp::Job& j, int priority) {
  stringstream s;
  s << "bury " << j.getJobId() << " " << priority << "\r\n";
  this->sendCommand(s);
  
  string response = this->tokenStream.nextString();
  this->tokenStream.expectEol();
  
  if(response.compare("NOT_FOUND") == 0)
    throw ServerException(ServerException::NOT_FOUND, "Got not found in reply to bury");
  
  if(response.compare("BURIED") != 0)
    throw ServerException(ServerException::BAD_FORMAT, "Didn't get BURIED reply to bury command");
}

size_t Beanstalkpp::Client::watch(const std::string& tube) {
  stringstream s("watch " + tube + "\r\n");
  int ret;
  
  this->sendCommand(s);
  this->tokenStream.expectString("WATCHING");
  ret = this->tokenStream.expectInt();
  this->tokenStream.expectEol();
  
  return ret;
}

vector< string > Beanstalkpp::Client::listTubes() {
  vector<string> ret;
  stringstream s("list-tubes\r\n");
  
  this->sendCommand(s);
  
  this->tokenStream.expectString("OK");
  size_t payloadSize = this->tokenStream.expectInt();
  this->tokenStream.expectEol();
  
  char* payload = this->tokenStream.readChunk(payloadSize);
  
  try {
    string p;
    p.assign(payload, payloadSize);
    size_t lastHit = 4; // We ignore the first four characters, which are "---\n"
    for(size_t i = lastHit; i < p.size(); i++) {
      if(payload[i] == '\n') {
        string str = p.substr(lastHit + 2, i - lastHit - 2);
        
        ret.push_back(str);
        lastHit = i+1;
      }
    }
    delete[] payload;
  } catch(...) {
    delete[] payload;
    throw;
  }
  
  return ret;
}

