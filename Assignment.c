#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
#include "net/rime.h"
#include "random.h"
#include "dev/sht11-sensor.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "net/rime/rime.h"

struct neighbor {
  /* The ->next pointer is needed since we are placing these on a
     Contiki list. */
  struct neighbor *next;

  /* The ->addr field holds the Rime address of the neighbor. */
  linkaddr_t addr;

  /* The ->last_rssi and ->last_lqi fields hold the Received Signal
     Strength Indicator (RSSI) and CC2420 Link Quality Indicator (LQI)
     values that are received for the incoming broadcast packets. */
  uint16_t last_rssi;

};

/* This #define defines the maximum amount of neighbors we can remember. */
#define MAX_NEIGHBORS 16
 
    //PROCESS(example_broadcast_process, "Broadcast example");
PROCESS(assignment, "Assignment");
AUTOSTART_PROCESSES(&assignment);

/* These hold the broadcast and unicast structures, respectively. */
static struct broadcast_conn broadcast;
static struct unicast_conn unicast;

/* These two defines are used for computing the moving average for the
   broadcast sequence number gaps. */
#define SEQNO_EWMA_UNITY 0x100
#define SEQNO_EWMA_ALPHA 0x040

/*---------------------------------------------------------------------------*/
/* We first declare our two processes. */
PROCESS(broadcast_process, "Broadcast process");
PROCESS(unicast_process, "Unicast process");

/* The AUTOSTART_PROCESSES() definition specifices what processes to
   start when this module is loaded. We put both our processes
   there. */
AUTOSTART_PROCESSES(&broadcast_process, &unicast_process);
/*---------------------------------------------------------------------------*/
/* This function is called whenever a broadcast message is received. */
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct neighbor *n;
  struct broadcast_message *m;
  uint8_t seqno_gap;

  /* The packetbuf_dataptr() returns a pointer to the first data byte
     in the received packet. */
  m = packetbuf_dataptr();

  /* Check if we already know this neighbor. */
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {

    /* We break out of the loop if the address of the neighbor matches
       the address of the neighbor from which we received this
       broadcast message. */
    if(linkaddr_cmp(&n->addr, from)) {
      break;
    }
  }

  /* If n is NULL, this neighbor was not found in our list, and we
     allocate a new struct neighbor from the neighbors_memb memory
     pool. */
  if(n == NULL) {
    n = memb_alloc(&neighbors_memb);

    /* If we could not allocate a new neighbor entry, we give up. We
       could have reused an old neighbor entry, but we do not do this
       for now. */
    if(n == NULL) {
      return;
    }

    /* Initialize the fields. */
    linkaddr_copy(&n->addr, from);
    n->last_seqno = m->seqno - 1;
    n->avg_seqno_gap = SEQNO_EWMA_UNITY;

    /* Place the neighbor on the neighbor list. */
    list_add(neighbors_list, n);
  }

  /* We can now fill in the fields in our neighbor entry. */
  n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

  /* Compute the average sequence number gap we have seen from this neighbor. */
  seqno_gap = m->seqno - n->last_seqno;
  n->avg_seqno_gap = (((uint32_t)seqno_gap * SEQNO_EWMA_UNITY) *
                      SEQNO_EWMA_ALPHA) / SEQNO_EWMA_UNITY +
                      ((uint32_t)n->avg_seqno_gap * (SEQNO_EWMA_UNITY -
                                                     SEQNO_EWMA_ALPHA)) /
    SEQNO_EWMA_UNITY;

  /* Remember last seqno we heard. */
  n->last_seqno = m->seqno;

  /* Print out a message. */
  printf("broadcast message received from %d.%d with seqno %d, RSSI %u, LQI %u, avg seqno gap %d.%02d\n",
         from->u8[0], from->u8[1],
         m->seqno,
         packetbuf_attr(PACKETBUF_ATTR_RSSI),
         packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY),
         (int)(n->avg_seqno_gap / SEQNO_EWMA_UNITY),
         (int)(((100UL * n->avg_seqno_gap) / SEQNO_EWMA_UNITY) % 100));
}

/*---------------------------------------------------------------------------*/
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};;
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/

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

  //broadcast_open(&broadcast, 129, &broadcast_call);
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
	
  while(1) {

    /* Delay 2-4 seconds */
  // etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL((ev==sensors_event) && (data == &button_sensor));

    packetbuf_copyfrom("Eugen", 6);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }
  
 		 etimer_reset(&et);
    	 SENSORS_DEACTIVATE(sht11_sensor);


  PROCESS_END();
}