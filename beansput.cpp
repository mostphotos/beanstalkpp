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

#include "config.h"

#include <boost/concept_check.hpp>

using namespace std;
using namespace Beanstalkpp;

int usage(const char **argv) {
  printf("Puts a raw job into a beanstalk tube.\n\n");
  printf("Usage:\n");
  printf("%s <tubename> [job]\n", argv[0]);
  printf("\n");
  printf("If job is not specified, it will be read from stdin.\n");
  
  return 1;
}

void demoClient(const string &tubeName, const string &job) {
  Client c(BEANSTALK_SERVER, BEANSTALK_PORT);
  c.connect();
  
  cout << job << endl;
  
  c.use(tubeName);
  c.put(job);
}

int main(int argc, const char **argv) {
  if(argc < 2) return usage(argv);
  
  string jobstr;
  
  if(argc == 3) {
    jobstr = argv[2];
  } else {
    // Read the job from stdin
    cout << "Reading from stdin..." << endl;
    stringstream s;
    string line;
    while(getline(cin, line)) {
      s << line << endl;
    }
    
    jobstr = s.str();
  }
  
  try {
    demoClient(argv[1], jobstr);
  } catch(Exception &e) {
    printf("Caught exception: %s\n", e.what());
  }
    
  return 0;
}
