#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
#include "net/rime.h"
#include "random.h"
#include "dev/sht11-sensor.h"
#include "lib/list.h"
#include "lib/memb.h"




/*---------------------------------------------------------------------------*/
//Structure for each broadcast message
struct broadcast_message {
  uint8_t seqno;
};

//Structure for each neighbor node
struct neighbor {
struct neighbor *next;
rimeaddr_t addr;
uint16_t last_rssi, last_lqi;
};
//Max number of neighbors
#define MAX_NEIGHBORS 16

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS); // Memory to store neighbors

LIST(neighbors_list); //List of neighbors


static struct broadcast_conn broadcast;

PROCESS(tempHum, "Temperature and Humidity");
PROCESS(broadcast_process, "Broadcast");
AUTOSTART_PROCESSES(&tempHum,&broadcast_process );

//Broadcast received callback gets triggered when a broadcast message is received
static void
broadcast_recv(struct broadcast_conn *c, const rimeaddr_t *from)
{

struct neighbor *n;
   struct broadcast_message *m;
   m = packetbuf_dataptr();

   //Check if the neighbor exists by looping thought the list until the pointer is null
   for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
	//If the current address in the loop matches the received address then break out from the loop
	if(rimeaddr_cmp(&n->addr, from))
	{
      		break;
    	}
   } //End forloop

   //Allocate a new memory block to a new neighbor
   if(n == NULL) {
    n = memb_alloc(&neighbors_memb);

	if(n == NULL) {
      return;
    }

    rimeaddr_copy(&n->addr, from); //Initialize the field
    list_add(neighbors_list, n); //Place neighbor on the list
  }
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
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

PROCESS_THREAD(tempHum, ev, data)
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
	printf("\nAverage humidity=%d\n", averageHum);


 		 etimer_reset(&et);
    	 SENSORS_DEACTIVATE(sht11_sensor);


  PROCESS_END();
}
