#include <lcom/lcf.h>
#include <lcom/lab2.h>
#include <stdbool.h>
#include <stdint.h>

extern int counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
  //IMPLEMENTED
  uint8_t st;

  if(timer<0 || timer>2){
    printf("invalid timer");
    return 1;
  }
  if(timer_get_conf(timer, &st)){
    printf("error in timer_get_config");
    return 1;
  }
  if(timer_display_conf(timer, st, field)){
    printf("error in timer_display_conf");
    return 1; //false
  }
  return 0; //true - success

}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
  //IMPLEMENTED
  if(timer<0 || timer>2){
    printf("invalid timer");
    return 1;
  }
  timer_set_frequency(timer, freq);

  return 0;
}

int(timer_test_int)(uint8_t time) { //time in seconds
  //IMPLEMENTED

  int ipc_status;
  message msg;
  int r;

  //subscribe
  uint8_t bit_no = 0; //timer irq line
  if(timer_subscribe_int(&bit_no)) return 1;

  //counter/60 = number of seconds that have passed -- 60Hz
  while((counter/60) < time){
    //get a request message 
    if((r=driver_receive(ANY, &msg, &ipc_status)) != 0){
      printf("driver_receive failed with %d", r);
      continue;
    }
    if(is_ipc_notify(ipc_status)){ //received notification
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE: //hardware interrupt notification
        if(msg.m_notify.interrupts & BIT(bit_no)){ //subscribed interrupt, If a given interrupt is pending then the corresponding hook_id bit of msg.m_notify.interrupts is set.
          timer_int_handler();
          if(counter%60 == 0){
            timer_print_elapsed_time();
          }
        }
        break;
      
      default:
        break;
      }
    } 

  }
  timer_unsubscribe_int();

  return 0;
}
