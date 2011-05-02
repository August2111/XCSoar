/* Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2010 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Language/MOLoader.hpp"
#include "OS/PathName.hpp"

#include <stdio.h>

#ifdef _UNICODE
#include <windows.h>
#include <syslimits.h>
#endif

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s FILE.mo STRING\n", argv[0]);
    return 1;
  }

#ifdef _UNICODE
  TCHAR path[PATH_MAX];
  int length = ::MultiByteToWideChar(CP_ACP, 0, argv[1], -1, path, PATH_MAX);
  if (length == 0)
    return 2;
#else
  const char *path = argv[1];
#endif

  const char *original = argv[2];

  MOLoader mo(path);
  if (mo.error()) {
    fprintf(stderr, "Failed to load %s\n", (const char *)NarrowPathName(path));
    return 2;
  }

  const char *translated = mo.get().lookup(original);
  if (translated == NULL) {
    fprintf(stderr, "No such string\n");
    return 3;
  }

  puts(translated);
  return 0;
}
