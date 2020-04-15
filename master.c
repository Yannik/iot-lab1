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
 *         NullNet broadcast example
 * \author
*         Simon Duquennoy <simon.duquennoy@ri.se>
 *
 */

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */
#include <random.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Master"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "helpers.h"

/* Configuration */
#define SEND_INTERVAL (1 * CLOCK_SECOND)
#define NUM_MASTERS 1
#define NUM_SLAVES 4

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

#define TEMP_NOT_RECEIVED -128
int8_t temperature[NUM_SLAVES] = {[0 ... NUM_SLAVES-1] = TEMP_NOT_RECEIVED};

int received_all_temperatures() {
  int i;
  for (i = 0; i < NUM_SLAVES; i++) {
    if (temperature[i] == TEMP_NOT_RECEIVED) {
      return 0;
    }
  }
  return 1;
}

void print_temperatures() {
  LOG_INFO("Received all temperatures!\n");

  int total = 0;
  int i;
  for (i = 0; i < NUM_SLAVES; i++) {
    total += temperature[i];
    LOG_INFO("Slave %u Temp: %d\n", i, temperature[i]);
    temperature[i] = TEMP_NOT_RECEIVED;
  }
  LOG_INFO("Average Temp: %d\n", total/NUM_SLAVES);
}

/*---------------------------------------------------------------------------*/
PROCESS(slave_process, "Slave");
AUTOSTART_PROCESSES(&slave_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  int node_id = linkaddr_to_node_id(src);
  if (node_id > NUM_SLAVES+NUM_MASTERS) {
    LOG_INFO("A unknown slave appeared!\n");
    return;
  }
  if(len == sizeof(struct command)) {
    struct command cmd;
    memcpy(&cmd, data, sizeof(cmd));
    if (cmd.command == COMMAND_SEND_TEMP) {
      LOG_INFO("Received Temp %d from %u\n", cmd.data, node_id);
      temperature[node_id-NUM_MASTERS-1] = cmd.data;
      if (received_all_temperatures()) {
        print_temperatures();
      }
      
    } else {
      log_unknown_command(cmd, src);
    }
  }
}

	

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(slave_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer periodic_timer;
  static struct command cmd;
  cmd.command = COMMAND_TOGGLE_LED;

  nullnet_buf = (uint8_t *)&cmd;
  nullnet_len = sizeof(cmd);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    cmd.data = random_rand() % 3;

    LOG_INFO("Broadcasting LED %u \n", cmd.data);
    NETSTACK_NETWORK.output(NULL);

    etimer_reset(&periodic_timer);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
