/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
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

/*
 * This program creates a pseudo-TTY symlinked to /tmp/nmea, and feeds
 * NMEA data read from stdin to it.  It is useful to feed WINE with
 * it: symlink ~/.wine/dosdevices/com1 to /tmp/nmea, and configure
 * "COM1" in XCSoar.
 */

#include "DebugPort.hpp"
#include "Device/Port/ConfiguredPort.hpp"
#include "Device/Port/LineHandler.hpp"
#include "Device/Internal.hpp"
#include "Profile/DeviceConfig.hpp"
#include "OS/Args.hpp"
#include "OS/Sleep.h"
#include "NMEA/InputLine.hpp"
#include "NMEA/Checksum.hpp"
#include "Util/Macros.hpp"

#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>

class VegaHandler : public PortLineHandler {
  Port *port;

  std::map<std::string, std::string> settings;

public:
  VegaHandler():port(NULL) {}

  Port::Handler &GetBase() {
    return *this;
  }

  void SetPort(Port &_port) {
    port = &_port;
  }

private:
  void PDVSC_S(NMEAInputLine &line) {
    char name[64], value[256];
    line.read(name, ARRAY_SIZE(name));
    line.read(value, ARRAY_SIZE(value));

    settings[name] = value;

    char buffer[512];
    snprintf(buffer, ARRAY_SIZE(buffer), "PDVSC,A,%s,%s", name, value);
    PortWriteNMEA(*port, buffer);
  }

  void PDVSC_R(NMEAInputLine &line) {
    char name[64];
    line.read(name, ARRAY_SIZE(name));

    auto i = settings.find(name);
    if (i == settings.end())
      return;

    const char *value = i->second.c_str();

    char buffer[512];
    snprintf(buffer, ARRAY_SIZE(buffer), "PDVSC,A,%s,%s", name, value);
    PortWriteNMEA(*port, buffer);
  }

  void PDVSC(NMEAInputLine &line) {
    char command[4];
    line.read(command, ARRAY_SIZE(command));

    if (strcmp(command, "S") == 0)
      PDVSC_S(line);
    else if (strcmp(command, "R") == 0)
      PDVSC_R(line);
  }

protected:
  virtual void DataReceived(const void *data, size_t length) {
    fwrite(data, 1, length, stdout);
    PortLineHandler::DataReceived(data, length);
  }

  virtual void LineReceived(const char *_line) {
    if (!VerifyNMEAChecksum(_line))
      return;

    NMEAInputLine line(_line);
    if (line.read_compare("$PDVSC"))
      PDVSC(line);
  }
};

int
main(int argc, char **argv)
{
  Args args(argc, argv, "PORT BAUD");
  const DeviceConfig config = ParsePortArgs(args);
  args.ExpectEnd();

  VegaHandler handler;
  Port *port = OpenPort(config, handler);
  if (port == NULL) {
    fprintf(stderr, "Failed to open COM port\n");
    return EXIT_FAILURE;
  }

  /* turn off output buffering */
  setvbuf(stdout, NULL, _IONBF, 0);

  handler.SetPort(*port);

  if (!port->StartRxThread()) {
    delete port;
    fprintf(stderr, "Failed to start the port thread\n");
    return EXIT_FAILURE;
  }

  while (port->IsValid())
    Sleep(1000);

  delete port;
  return EXIT_SUCCESS;
}
