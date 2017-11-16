#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
#include "net/rime.h"
#include "random.h"
#include "dev/sht11-sensor.h"

    
    //PROCESS(example_broadcast_process, "Broadcast example");
PROCESS(example_temperature, "Temp proc");
AUTOSTART_PROCESSES(&example_temperature);
/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call;
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(example_temperature, ev, data)
{
  static struct etimer et;
  static int counter = 0;
  static int val;
  static int averageTemp = 0;
  static float s = 0;
  static int dec;
  


  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);

  //broadcast_open(&broadcast, 129, &broadcast_call);
	 //etimer_set(&et, CLOCK_SECOND * 2);
	while(counter < 5)
	{

	  etimer_set(&et, CLOCK_SECOND * 2);
	   SENSORS_ACTIVATE(sht11_sensor);
        
	   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	   val = sht11_sensor.value(SHT11_SENSOR_TEMP);
	   
	   if(averageTemp != -1) 
      	   {
		s= ((0.01*val) - 39.60);
           }
           dec = s;
	   averageTemp = averageTemp + dec;
           counter++;
	}
	averageTemp = averageTemp/5;
	printf("\nAverage temperature=%d C\n", averageTemp);
	
  while(1) {

    /* Delay 2-4 seconds */
  // etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));

    packetbuf_copyfrom("Eugen", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }

  PROCESS_END();
}
