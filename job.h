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

#ifndef JOB_H
#define JOB_H

#include <boost/shared_ptr.hpp>
#include <string>

namespace Beanstalkpp {
  
class Client;
class Job;

typedef unsigned int job_id_t;
typedef boost::shared_ptr<Job> job_p_t;

/**
 * A received beanstalk job.
 * 
 * To receive jobs, use @c Client::reserve
 */
class Job {
public:
  Job();
  
  /**
   * Creates a new job.
   * 
   * The new job will take ownership of the memory in @p payload and free it when apropriate.
   * 
   * @param c           The client which the job arrived from
   * @param jobId       The job id (comes from the beanstalk server)
   * @param payloadSize The size of the payload, in bytes
   * @param payload     The actual payload of the job
   */
  Job(Client &c, job_id_t jobId, size_t payloadSize, char *payload);
  
  /**
   * Copy constructor to allow returning classes from functions.
   */
  Job(const Job &job);
  
  virtual ~Job();
  
  /**
   * Treats the job as a string message and returns it.
   */
  std::string asString() const;
  
  /**
   * Treats the job as a number written in ascii, converts it to an int, and returns it.
   * 
   * This is more or less equivalent to calling atoi(asString())
   * 
   * @throws Exception If asString() can not be converted to an int
   */
  int asAsciiInt() const;
  
  /**
   * Returns the beanstalk job id 
   */
  job_id_t getJobId() const;
  
  Job operator =(const Job &job);
private:
  Client *client;
  boost::shared_ptr<char> payload;
  size_t payloadSize;
  job_id_t jobId;
};

}

#endif
