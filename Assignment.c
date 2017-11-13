#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
#include "net/rime.h"
#include "random.h"
#include "dev/sht11-sensor.h"

    
    PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
//static int;
get_temp(void)
{
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    /* Delay 4 seconds */
   etimer_set(&et, CLOCK_SECOND * 4);
   SENSORS_ACTIVATE(sht11_sensor);
   
   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    //PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));
	printf("Temperature: %d \n", get_temp());
    packetbuf_copyfrom("Eugen", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }

  PROCESS_END();
}