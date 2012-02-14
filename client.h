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

#ifndef _BEANSTALK_POOL_H
#define _BEANSTALK_POOL_H

#include <string>
#include <iostream>
#include <sstream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

#include "tokenizedstream.h"
#include "job.h"
#include "serverexception.h"

namespace Beanstalkpp {

class Job;

/**
 * The beanstalk client. Used for sending and receiving jobs over beanstalk.
 */
class Client {
public:
  /**
   * Creates a new client and connects it to a server.
   * 
   * @param hostname The hostname to connect to
   * @param port     The TCP port to use
   */
  Client(const std::string &server, int port);
  
  /**
   * Connect to the server. This must be done before performing any other operations.
   * 
   * @throws Exception If we couldn't connect to the server (network errors)
   */
  void connect();
  
  /**
   * Selects the tube to send jobs through. If no tube has been selected, the tube "default" is
   * used.
   * 
   * @param tubeName The name of the tube
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  void use(const std::string &tubeName);
  
  /**
   * Adds a job consisting of a string to the server
   * 
   * @param data The data to send
   * 
   * @return The id the job got on the server
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   * @throws ServerException With reason JOB_TOO_BIG on jobs the server deems too big
   */
  int put(const std::string &data);
  
  /**
   * Reserves the next job in the queue. This function is blocking until a job becomes available in
   * the queue.
   * 
   * TJob should be either @c Job, or a subclass of @c Job.
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  template<class TJob>
  TJob reserve() {
    job_id_t jobId;
    size_t payloadSize;
    char *payload;
    std::stringstream s("reserve\r\n");

    this->sendCommand(s);
    
    this->tokenStream.expectString("RESERVED");
    jobId = this->tokenStream.expectInt();
    payloadSize = this->tokenStream.expectInt();
    this->tokenStream.expectEol();
    
    payload = this->tokenStream.readChunk(payloadSize);
    this->tokenStream.expectEol();
    
    return TJob(*this, jobId, payloadSize, payload);
  }
  
  /**
   * Alias for reserve<Job>
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  Job reserve();
  
  /**
   * Tries to reserve a job from the queue. If a job is available within @p timeout, it will be 
   * placed in @p jobPtr, and true will be returned. Otherwise @p jobPtr will be left unchanged, 
   * and false will be returned.
   * 
   * @param jobPtr  Return-value if a job was found. Should be @c Job or a subclass of @c Job
   * @param timeout The timeout counted in seconds
   * 
   * @return Wether a job was found and put into @p jobPtr
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  template<class TJob>
  bool reserveWithTimeout(boost::shared_ptr<TJob> &jobPtr, int timeout) {
    job_id_t jobId;
    size_t payloadSize;
    char *payload;
    std::stringstream s;
    
    s << "reserve-with-timeout " << timeout << "\r\n";
    this->sendCommand(s);
    
    std::string response = this->tokenStream.nextString();
    
    if(response.compare("RESERVED") == 0) {
      jobId = this->tokenStream.expectInt();
      payloadSize = this->tokenStream.expectInt();
      this->tokenStream.expectEol();
      
      payload = this->tokenStream.readChunk(payloadSize);
      this->tokenStream.expectEol();
      
      boost::shared_ptr<TJob> newJob(new TJob(*this, jobId, payloadSize, payload));
      jobPtr = newJob;
      return true;
    }
    
    if(response.compare("TIMED_OUT") == 0) {
      this->tokenStream.expectEol();
      return false;
    }
    
    throw ServerException(
      ServerException::BAD_FORMAT, 
      "Didn't get RESERVED or TIMED_OUT reply to reserve-with-timeout command"
    );    
  }
  
  /**
   * Wrapper for reserveWithTimeout<Job>.
   * 
   * @param jobPtr  Return-value if a job was found. Should be @c Job or a subclass of @c Job
   * @param timeout The timeout counted in seconds
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  bool reserveWithTimeout(job_p_t &jobPtr, int timeout);
  
  /**
   * Peeks at the next ready job in the queue. This will not reserve the job.
   * 
   * @param jobPtr Return-value if a job was found. Should be @c Job or a subclass of @c Job
   * 
   * @return True of a job was found, otherwise false.
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  bool peekReady(job_p_t &jobPtr);
  
  /**
   * The delete command removes a job from the server entirely. It is normally used by the client 
   * when the job has successfully run to completion. A client can only delete jobs that it has 
   * reserved or jobs that are buried. 
   * 
   * @param j The job to delete
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  void del(const Job &j);
  
  /**
   * The delete command removes a job from the server entirely. It is normally used by the client 
   * when the job has successfully run to completion. A client can only delete jobs that it has 
   * reserved or jobs that are buried. 
   * 
   * @param j The job to delete
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  void del(const job_p_t &j);
  
  /**
   * The bury command puts a job into the "buried" state. Buried jobs are put into a FIFO linked 
   * list and will not be touched by the server again until a client kicks them with the "kick"
   * command.
   * 
   * @param j        The job to bury
   * @param priority The priority in the buried queue
   * 
   * @throws ServerException With status NOT_FOUND if the job was not found on the server
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  void bury(const Job &j, int priority = 10);
  
  /**
   * The "watch" command adds the named tube to the watch list for the current connection. A reserve
   * command will take a job from any of the tubes in the watch list. For each new connection, the
   * watch list initially consists of one tube, named "default".
   * 
   * @param tube The new tube to watch
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   * 
   * @return The number of tubes currently watched
   */
  size_t watch(const std::string &tube);
  
  /**
   * Returns a list of all tubes available at the beanstalk server
   * 
   * @throws ServerException With reason BAD_FORMAT on unexpected server response
   */
  std::vector<std::string> listTubes();
private:
  std::string tubeName;
  
  /**
   * Sends a command over the TCP wire
   * 
   * @param cmd The command to send
   * 
   * @throws Exception On network errors
   */
  void sendCommand(const std::stringstream &cmd);
  
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::socket socket;
  
  std::string hostname;
  int port;
  
  TokenizedStream tokenStream;
};

}

#endif
