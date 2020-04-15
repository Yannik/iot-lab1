/*
 * Copyright (c) 2017, RISE SICS.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         NullNet unicast example
 * \author
*         Simon Duquennoy <simon.duquennoy@ri.se>
 *
 */

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/leds.h"
#include "command.h"

#include <string.h>
#include <stdio.h> /* For printf() */
#include <random.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Slave"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "helpers.h"

/* Configuration */
#define SEND_INTERVAL (1 * CLOCK_SECOND)
static linkaddr_t master_addr =         {{ 0x01, 0x01, 0x01, 0x00, 0x01, 0x74, 0x12, 0x00 }};

/*---------------------------------------------------------------------------*/
PROCESS(master_process, "Master");
AUTOSTART_PROCESSES(&master_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(struct command)) {
    struct command cmd;
    memcpy(&cmd, data, sizeof(cmd));
    if (cmd.command == COMMAND_TOGGLE_LED) {
      if (!linkaddr_cmp(src, &master_addr)) {
        LOG_INFO("Received LED setting from non-master!\n");
	return;
      }

      LOG_INFO("Received LED setting %u from master\n", cmd.data);
      leds_set(1 << (4 + cmd.data));
    } else {
      log_unknown_command(cmd, src);
    }
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(master_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer periodic_timer;
  static struct command cmd;
  cmd.command = COMMAND_SEND_TEMP;

  nullnet_buf = (uint8_t *)&cmd;
  nullnet_len = sizeof(cmd);
  nullnet_set_input_callback(input_callback);

  fix_randomness(&linkaddr_node_addr);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    // Generate random temp between -100 and 200
    cmd.data = (random_rand() % 200) - 100;
   
    LOG_INFO("Sending Temperature %d\n", cmd.data); 
    NETSTACK_NETWORK.output(&master_addr);

    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
