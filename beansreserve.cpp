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

#include <stdio.h>
#include <string>

#include "client.h"
#include "serverexception.h"
#include "job.h"

#include "config.h"

using namespace std;
using namespace Beanstalkpp;

int usage(const char **argv) {
  printf("Reserves and deletes all jobs in a beanstalk tube.\n\n");
  printf("Usage:\n");
  printf("%s <tubename>\n", argv[0]);
  
  return 1;
}

void demoClient(const string &tubeName) {
  Client c(BEANSTALK_SERVER, BEANSTALK_PORT);
  try {
    c.connect();
  } catch(Exception &e) {
    cerr << e.what() << endl;
    exit(1);
  }
  c.watch(tubeName);
  
  while(true) {
    Job j = c.reserve();
    
    printf("Received job:\n%s\n", j.asString().c_str());
    
    c.del(j);
  }
}

int main(int argc, const char **argv) {
  if(argc < 2) return usage(argv);
  
  try {
    demoClient(argv[1]);
  } catch(ServerException &e) {
    printf("Caught exception: %s\n", e.what());
  }
  
  return 0;
}
