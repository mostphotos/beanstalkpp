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

int main(int argc, const char **argv) {
  Client c(BEANSTALK_SERVER, BEANSTALK_PORT);
  c.connect();
  
  vector<string> tubes = c.listTubes();
  for(vector<string>::iterator i = tubes.begin(); i != tubes.end(); i++) 
    cout << *i << endl;
  
  return 0;
}
