#include "contiki.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>

#include "net/rime.h"

#include "random.h"

#include "dev/sht11-sensor.h"





    //PROCESS(example_broadcast_process, "Broadcast example");

PROCESS(assignment, "Assignment");
PROCESS(example_broadcast_process, "Broadcast example");

AUTOSTART_PROCESSES(&assignment,&example_broadcast_process );

/*---------------------------------------------------------------------------*/

static const struct broadcast_callbacks broadcast_call;

static struct broadcast_conn broadcast;

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(example_broadcast_process, ev, data)
{
  SENSORS_ACTIVATE(button_sensor);

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {


    PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));

    packetbuf_copyfrom("Assignment", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }

  PROCESS_END();
}


PROCESS_THREAD(assignment, ev, data)

{

  static struct etimer et;

  static int counter = 0;

  static int valTemp;

  static int averageTemp = 0;

  static float sTemp = 0;

  static int decTemp;

  static int valHum;

  static int averageHum = 0;

  static float sHum = 0;

  static int decHum;







  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)



  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);



 // broadcast_open(&broadcast, 129, &broadcast_call);

	 //etimer_set(&et, CLOCK_SECOND * 2);

	while(counter < 5)

	{

		//Activate sensors

	  etimer_set(&et, CLOCK_SECOND * 2);

	   SENSORS_ACTIVATE(sht11_sensor);

        //Trigger process start

	   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));



		//Calculate average temperature

	   valTemp = sht11_sensor.value(SHT11_SENSOR_TEMP);



	   if(valTemp != -1)

      	   {

		sTemp= ((0.01*valTemp) - 39.60);

           }

           decTemp = sTemp;

	   averageTemp = averageTemp + decTemp;



	   //Calculate average humidity

	   valHum=sht11_sensor.value(SHT11_SENSOR_HUMIDITY);



	   if(valHum != -1)

      	   {

		sHum= (((0.0405*valHum) -4) + ((-2.8 * 0.000001)*((valHum)*(valHum))));

           }

           decHum = sHum;

           averageHum = averageHum + decHum;

           counter++;

	}

	averageHum = averageHum/5;

	averageTemp = averageTemp/5;

	printf("\nAverage temperature=%d C\n", averageTemp);

	printf("\nAverage humidity=%d", averageHum);





 		 etimer_reset(&et);

    	 SENSORS_DEACTIVATE(sht11_sensor);





  PROCESS_END();

}
