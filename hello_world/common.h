#include <stdio.h>
#include <iostream>
#include "rpc.h"

static const std::string kServerHostname = "localhost";
static const std::string kClientHostname = "localhost";

static constexpr uint16_t kUDPPort = 31850;
static constexpr uint16_t kUDPPortClient = 31851;
static constexpr uint8_t kReqType = 2;
static constexpr size_t kMsgSize = 2600;
