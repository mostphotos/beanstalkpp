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

#include <stdlib.h>
#include <stdio.h>

#include "client.h"
#include "job.h"
#include "serverexception.h"

using namespace Beanstalkpp;
using namespace std;

/*
 * Just some simple tests for the beanstalk++ client
 */
const char *strs[] = {
"This", "will", "return", "a", "newly-reserved", "job.", "If", "no", "job", "is", "available", "to", "be", "reserved",
"beanstalkd", "will", "wait", "to", "send", "a", "response", "until", "one", "becomes", "available.", "Once", "a",
"job", "is", "reserved", "for", "the", "client,", "the", "client", "has", "limited", "time", "to", "run", "(TTR)", "the",
"job", "before", "the", "job", "times", "out.", "When", "the", "job", "times", "out,", "the", "server", "will", "put", "the",
"job", "back", "into", "the", "ready", "queue.", "Both", "the", "TTR", "and", "the", "actual", "time", "left", "can", "be",
"found", "in", "response", "to", "the", "stats-job", "command.",

"A", "timeout", "value", "of", "0", "will", "cause", "the", "server", "to", "immediately", "return", "either", "a",
"response", "or", "TIMED_OUT.", "", "A", "positive", "value", "of", "timeout", "will", "limit", "the", "amount", "of",
"time", "the", "client", "will", "block", "on", "the", "reserve", "request", "until", "a", "job", "becomes",
"available.",

"During", "the", "TTR", "of", "a", "reserved", "job,", "the", "last", "second", "is", "kept", "by", "the", "server", "as", "a",
"safety", "margin,", "during", "which", "the", "client", "will", "not", "be", "made", "to", "wait", "for", "another",
"job.", "If", "the", "client", "issues", "a", "reserve", "command", "during", "the", "safety", "margin,", "or", "if",
"the", "safety", "margin", "arrives", "while", "the", "client", "is", "waiting", "on", "a", "reserve", "command,",
"the", "server", "will", "respond", "with:", NULL
};

void putJobs(Client &c) {
  const char *s;
  for(int i = 0; ; i++) {
    s = strs[i];
    if(!s) break;
    
    c.put(s);
  }
  
  printf("Put all jobs\n");
}

void verifyJobs(Client &c) {
  const char *s;
  int i;
  for(i = 0; true; i++) {
    s = strs[i];
    if(!s) break;
    
    Job j = c.reserve();
    if(j.asString().compare(s) != 0)
      printf("Bad beanstalk reply: %s\n", j.asString().c_str());
    c.del(j);
  }
  
  printf("Verified %d strings\n", i);
}

int main() {
  Client c("127.0.0.1", 11300);
  try {
    c.connect();
    for(int i = 0; i < 10000; i++) {
      printf("\r%d      ", i);
      putJobs(c);
      verifyJobs(c);
    }
  } catch(Exception &e) {
    printf("Caught exception: %s\n", e.what());
  }
  
  printf("Hit enter to exit\n");
  
  string s;
  cin >> s;
  
  return 0;
}
