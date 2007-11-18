/*  The macros provide the type casting useful when writing event drivers. */
#define VICTIM_CH  ((struct char_data *)victim)
#define CAUSER_CH  ((struct char_data *)causer)
#define VICTIM_OBJ ((struct obj_data *)victim)
#define CAUSER_OBJ ((struct obj_data *)causer)

void add_event(int delay,EVENT(*func),void *causer,void *victim,void *info);
void run_events();
void clean_events(void *pointer);
