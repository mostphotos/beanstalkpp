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

#include "job.h"

#include <sstream>

#include "exception.h"

using namespace std;

Beanstalkpp::Job::Job() {
  client = NULL;
  this->payloadSize = 0;
  this->jobId = 0;
}


Beanstalkpp::Job::Job(Beanstalkpp::Client& c, job_id_t jobId, size_t payloadSize, char* payload):
  client(&c), payload(payload) {
  this->payloadSize = payloadSize;
  this->jobId = jobId;
}

Beanstalkpp::Job::Job(const Beanstalkpp::Job& job): client(job.client), payload(job.payload) {
  this->payloadSize = job.payloadSize;
  this->jobId = job.jobId;
}


Beanstalkpp::Job::~Job() {

}

std::string Beanstalkpp::Job::asString() const {
  std::string s;
  s.assign((const char*)this->payload.get(), this->payloadSize);
  
  return s;
}

Beanstalkpp::Job Beanstalkpp::Job::operator=(const Beanstalkpp::Job& job) {
  this->payloadSize = job.payloadSize;
  this->client = job.client;
  this->payload = job.payload;
  this->jobId = job.jobId;
  
  return *this;
}

Beanstalkpp::job_id_t Beanstalkpp::Job::getJobId() const {
  return this->jobId;
}

int Beanstalkpp::Job::asAsciiInt() const {
  istringstream s(this->asString());
  int ret = 0;
  
  if(!(s >> ret)) throw Exception("Payload is not a valid ASCII int");
  
  return ret;
}
