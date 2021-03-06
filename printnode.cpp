/*
  printnode.cpp - generate file/directory list

  Copyright (C) 2013 Fabian Renn - fabian.renn (at) gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "printnode.hpp"
#include "crc32.hpp"

using namespace std;

/* on unix system_path = path; on windows system_path is the windows native path format of path */
void print_node( ostream & output, const string & base, const string & system_path, const string & path ) {
  struct stat s;
  string fullpath( base );
#if defined(WINDOWS)
  if ( system_path.size() != 0 ) {
    fullpath += string( "\\" ) + system_path;
  }
#else
  fullpath += string( "/" ) + system_path;
#endif
  if ( stat( fullpath.c_str(), &s ) != 0 ) {
    cerr << "Unable to find path: " << fullpath << endl;
    exit(1);
  }
  output << path << "\t" << setbase(8) << s.st_mode << "\t" << setbase(10) << s.st_uid << "/" << s.st_gid;
  if ( S_ISREG(s.st_mode) ) {
    output << "\t" << s.st_size << "\t" << calc_crc32( fullpath.c_str() );
  }
  output << endl;
#if !defined(WINDOWS)
  if ( S_ISLNK( s.st_mode) ) {
    cerr << endl << "We don't support symbolic links yet: please modify code" << endl;
    exit(1);
  }
#endif
  if ( S_ISDIR( s.st_mode ) ) {
    DIR * d = opendir( fullpath.c_str() );
    struct dirent * dir;
    while ( ( dir = readdir( d ) ) != NULL ) {
      if ( dir->d_name[0] != '.' ) {
	string new_path(path);
	new_path += string( "/" ) + string( dir->d_name );
#if defined(WINDOWS)
	string new_system_path(system_path);
	new_system_path += string( "\\" ) + string( dir->d_name );
#else
	string new_system_path( new_path );
#endif
	print_node( output, base, new_system_path, new_path );
      }
    }
    closedir( d );
  }
}

void print_node( ostream & output, string directory ) {
  if ( directory.size() < 1 ) {
    cerr << "Invalid path" << endl;
    exit(1);
  }
  if ( directory[directory.size()-1] == '/' ) {
    directory = directory.substr( 0, directory.size() - 1 );
  }
  struct stat s;
  if ( stat( directory.c_str(), &s ) != 0 ) {
    cerr << "Unable to find path: " << directory << endl;
    exit(1);
  }
  if ( S_ISDIR( s.st_mode ) == false ) {
    cout << endl << "Argument must be a directory" << endl;
    exit(1);
  }
  print_node( output, directory, "", "." );
}
