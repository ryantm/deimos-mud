#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "events.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"

int in_event_handler=0;
struct event_info *pending_events = NULL;
struct event_info *prev = NULL;

void run_events()
{
  struct event_info *temp, *prox;

  in_event_handler=1;

  prev=NULL;
  for(temp=pending_events;temp;temp=prox) {
    prox=temp->next;
    temp->ticks_to_go--;
    if (temp->ticks_to_go==0) {

      /* run the event */
      if (!temp->func)
        log("SYSERR: Attempting to run a NULL event. Ignoring");
      else
        (temp->func)(temp->causer,temp->victim,temp->info);

      /* remove the event from the list. */
      if (!prev)
        pending_events=prox;
      else
        prev->next=prox;
      free(temp);
    } else if (temp->ticks_to_go==-1) {
      if (!prev)
        pending_events=prox;
      else
        prev->next=prox;
      free(temp);
    } else
      prev=temp;
  }

  in_event_handler=0;
}

void add_events(int delay,EVENTS(*func),void *causer,void *victim,void
*info)
{
  struct event_info *new;

  CREATE(new,struct event_info,1);

  new->ticks_to_go=delay;
  new->func=func;
  new->causer=causer;
  new->victim=victim;
  new->info=info;

  new->next=pending_events;
  pending_events=new;
  if (in_event_handler && !prev)
    prev=pending_events;
}

void clean_events(void *pointer)
{
  struct event_info *temp,*prox;
  struct event_info *previous;

  if (in_event_handler)
    log("SYSERR: Trying to remove events inside the handler. Attempting to
continue.");
  previous=NULL;
  for(temp=pending_events;temp;temp=prox) {
    prox=temp->next;
    if (temp->causer==pointer || temp->victim==pointer ||
        (void *)(temp->func)==pointer)
      temp->ticks_to_go=0;
  }
}


/* ************************************ *
 *  Pre-defined events.(Just examples)  *
 * ************************************ */

extern struct room_data *world;
EVENT(teleport_event)
{
  int destin=0;
  void death_cry(struct char_data *ch);

  /* Is the character still in the same room? */
  if (GET_LEVEL(VICTIM_CH)<LVL_GOD && VICTIM_CH->in_room==info) {

    /* Tell the other players in the room about the teleport. */

act(world[VICTIM_CH->in_room].teleport->message_you,TRUE,VICTIM_CH,NULL,
        VICTIM_CH,TO_VICT);

act(world[VICTIM_CH->in_room].teleport->message_oth,FALSE,VICTIM_CH,NULL,
        VICTIM_CH,TO_NOTVICT);

    /* Perform the teleport. */
    if ((destin=real_room(world[info].teleport->to_room))!=-1)
    {
      char_from_room(VICTIM_CH);
      char_to_room(VICTIM_CH,destin);
      look_at_room(VICTIM_CH,1);
      if (IS_SET(ROOM_FLAGS(VICTIM_CH->in_room),ROOM_DEATH) &&

GET_LEVEL(VICTIM_CH)<LVL_IMMORT)
      {
        log_death_trap(VICTIM_CH);
        death_cry(VICTIM_CH);
        extract_char(VICTIM_CH);
      }
    } else
      log("SYSERR: Trying to teleport to unexisting room. Ignoring.");
  }
}
