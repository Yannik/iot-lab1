/**
 * \file
 *         Lab1 Slave
 * \author
 *         Yannik Sembritzki <yannik@sembritzki.org>
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
