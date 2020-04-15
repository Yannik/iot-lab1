#ifndef HELPERS_H
#define HELPERS_H

#include "net/netstack.h"
#include "sys/log.h"
#include "command.h"

uint8_t linkaddr_to_node_id(const linkaddr_t *linkaddr);

void fix_randomness(const linkaddr_t *linkaddr);

// inlining this here so we don't have to define LOG_MODULE and LOG_LEVEL in helpers.c
inline void log_unknown_command(struct command cmd, const linkaddr_t *src) {
      LOG_INFO("Received unknown command %u with data 0x%04x from ", cmd.command, cmd.data);
      LOG_INFO_LLADDR(src);
      LOG_INFO_("\n");
}

#endif
