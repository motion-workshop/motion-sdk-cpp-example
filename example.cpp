/*
  @file    motion-sdk-cpp-example/example.cpp
  @version 2.6

  Copyright (c) 2018, Motion Workshop
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/
#include <Client.hpp>
#include <Format.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


/**
  Utility class to store all of the options to run our Motion SDK data stream.
*/
class command_line_options {
public:

  command_line_options();

  int parse(int argc, char **argv);

  int print_help(std::ostream *out, char *program_name);

  std::string message;
  std::string filename;
  int frames;
  std::string address;
  unsigned port;
  std::string separator;
  std::string newline;
}; // class command_line_options


/**
  Connect to a Motion Service by IP address and port number. Defaults to
  loopback (127.0.0.1) address and port 32076 (which is the Configurable data
  service).

  Request a list of channels by sending an XML list. Read frames forever and
  write out one row in a CSV spreadsheet per frame.

  Frames are a fixed size, so one channel is one column and one frame is one
  row in the CSV format.
*/
int stream_data_to_csv(
  std::ostream *output,
  std::ostream *error,
  const command_line_options &options)
{
  using Motion::SDK::Client;
  using Motion::SDK::Format;

  // Open connection to the configurable data service.
  Client client(options.address, options.port);
  if (!client.isConnected()) {
    *error
      << "failed to connect to Motion Service on " << options.address << ":"
      << options.port;
    return -1;
  }

  {
    // Request the channels that we want from every connected device. The full
    // list is available here:
    //
    //   https://www.motionshadow.com/download/media/configurable.xml
    //
    // Select the local quaternion (Lq) and positional constraint (c)
    // channels here. 8 numbers per device per frame. Ask for inactive nodes
    // which are not necssarily attached to a sensor but are animated as part
    // of the Shadow skeleton.
    const std::string xml_definition =
      "<?xml version=\"1.0\"?>"
      "<configurable inactive=\"1\">"
      "<Lq/>"
      "<c/>"
      "</configurable>";

    if (!client.writeData(
      Client::data_type(xml_definition.begin(), xml_definition.end()))) {
      *error
        << "failed to send channel list request to Configurable service";
      return -1;
    }
  }

  if (!client.waitForData(1)) {
    *error
      << "no active data stream available, giving up";
    return -1;
  }

  {
    std::string xml_node_list;
    if (client.getXMLString(xml_node_list)) {
    }
  }

  int num_frames = 0;
  for (;;) {
    // Read one frame of data from all connected devices.
    Client::data_type data;
    if (!client.readData(data)) {
      *error
        << "data stream interrupted or timed out";
      return -1;
    }

    bool have_output_line = false;

    // Iterate through the entries, one per device.
    auto list = Format::Configurable(data.begin(), data.end());
    for (const auto &item : list) {
      // Iterate through the channels per device. From the channel list we
      // know that each node has 8 channels.
      // [Lqw, Lqx, Lqy, Lqz, cw, cx, cy, cz]
      // Lq is unit quaternion rotation in the skeletal joint coordinate frame
      // cw is unitless scalar, 0 is not constrained, 1 is fully constrained
      // cx, cy, cz are global position in centimeters
      for (std::size_t i=0; i<item.second.size(); ++i) {
        if (have_output_line) {
          *output << options.separator;
        } else {
          have_output_line = true;
        }

        *output << item.second[i];
      }
    }

    if (!have_output_line) {
      *error
        << "unknown data format in stream";
      return -1;
    }

    *output << options.newline;

    if (options.frames > 0) {
      if (++num_frames >= options.frames) {
        break;
      }
    }
  }

  return 0;
}


int main(int argc, char **argv)
{
  command_line_options options;
  if (0 != options.parse(argc, argv)) {
    return options.print_help(&std::cerr, *argv);
  }

  // Stream frames to a CSV spreadsheet file.
  std::ofstream fout;
  if (!options.filename.empty()) {
    fout.open(options.filename, std::ios_base::out | std::ios_base::binary);
  }
  // Capture error messages so we do not interfere with the CSV output stream.
  std::ostringstream err;

  const int result = stream_data_to_csv(
    fout.is_open() ? &fout : &std::cout,
    &err,
    options);
  if (0 != result) {
    std::cerr << err.str();
  }

  return result;
}


command_line_options::command_line_options()
  : message(), filename(), frames(), address("127.0.0.1"), port(32076),
    separator(","), newline("\n")
{
}

int command_line_options::parse(int argc, char **argv)
{
  for (int i=1; i<argc; ++i) {
    const std::string arg(argv[i]);
    if ("--file" == arg) {
      ++i;
      if (i < argc) {
        filename = argv[i];
      } else {
        message = "Missing required argument for --file";
        return -1;
      }
    } else if ("--frames" == arg) {
      ++i;
      if (i < argc) {
        frames = std::stoi(argv[i]);
      } else {
        message = "Missing required argument for --frames";
        return -1;
      }
    } else if ("--help" == arg) {
      return 1;
    } else {
      message = "Unrecognized option \"" + arg + "\"";
      return -1;
    }
  }

  return 0;
}

int command_line_options::print_help(std::ostream *out, char *program_name)
{
  if (!message.empty()) {
    *out
      << message << newline << newline;
  }

  *out
    << "Usage: " << program_name << " [options...]" << newline
    << newline
    << "Allowed options:" << newline
    << "  --help         show help message" << newline
    << "  --file arg     output file" << newline
    << "  --frames N     read N frames" << newline
    << newline;

  return 1;
}