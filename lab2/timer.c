#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

uint32_t counter = 0;
int hook_id;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  /* To be implemented by the students */
  //IMPLEMENTED

  //the real frequency is equal to TIMER_FREQ/freq, and must be a value between 2^0 and 2^16 ([1,65536[).

  /*"in mode 3, the timer generates a square wave with a frequency given by the expression clock/div, 
  where clock is the frequency of the Clock input and div is the value loaded initially in the timer."*/
  //clock frequency is TIMER_FREQ

  if(TIMER_FREQ/freq < BIT(0) || TIMER_FREQ/freq >= BIT(16)){
    printf("Invalid frequency");
      return 1;
  }
  //ler timer config
  uint8_t st;
  if(timer_get_conf(timer, &st) !=OK){
    printf("Error getting the config\n");
    return 1;
  }
  uint32_t newFreq = TIMER_FREQ/freq;
  uint8_t lsb, msb;
  //seperate the frequency in two bytes (Most significant (MSB) and least significant (LSB))
  util_get_LSB(newFreq, &lsb);
  util_get_MSB(newFreq, &msb);

  //alter the frequency of the wanted timer
  //word is made of selected counter (2bit), init mode (LSB+MSB, 2bit), op mode (from st, 3bit) and base (from st, 1 bit)

    st = (st & 0x0f) | TIMER_LSB_MSB;

  switch (timer)
  {
  case 0:
    st |= TIMER_SEL0;
    break;
  case 1:
    st |= TIMER_SEL1;
    break;
  case 2:
    st |= TIMER_SEL2;
    break;
  default:
    printf("Invalid timer(timer_set_frequency)");
    return 1;
  }

  sys_outb(TIMER_CTRL,st); 

  //enviar a nova frequencia
  sys_outb(GET_TIMER(timer),lsb)
  sys_outb(GET_TIMER(timer),msb);
  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  //IMPLEMENTED
  // How does the GIH know that a DD is interested in an interrupt?
  //     R: The DD tells it with sys_irqsetpolicy  kernel call

  //use IRQ_REENABLE to inform the GIH that it can give the EOI command
  hook_id = *bit_no;
  return sys_irqsetpolicy(TIMER0_IRQ,IRQ_REENABLE, &hook_id);
}

int (timer_unsubscribe_int)() {
  //IMPLEMENTED
  return sys_irqrmpolicy(&hook_id);
}

void (timer_int_handler)() {
  //IMPLEMENTED
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  //IMPLEMENTED

  if (timer < 0 || timer > 2) {
    printf("Invalid timer(timer_get_conf)");
    return 1;
  }

  uint8_t byte = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);

  //write the uint8_t variable byte (read back command) to the control port
  sys_outb(TIMER_CTRL,byte);

  //read from the port of the timer
  util_sys_inb(GET_TIMER(timer),st);

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  //IMPLEMENTED
  //st is the config of the timer
  union timer_status_field_val conf; //can store any of the config fields

  switch (field)
  {
  case tsf_all: //all of the status byte
    conf.byte = st;
    break;
  
  case tsf_initial: //only the bits containing the initialization mode
    switch (TIMER_LSB_MSB & st)
    {
    case TIMER_LSB:
      conf.in_mode = LSB_only;
      break;
    case TIMER_MSB:
      conf.in_mode = MSB_only;
      break;
    case TIMER_LSB_MSB:
      conf.in_mode = MSB_after_LSB;
      break;
    default:
      conf.in_mode = INVAL_val;
      break;
    }
    break;
  case tsf_mode: //only the bits containing the counting mode
    conf.count_mode = (st & OPERATING_MODE) >> 1;
    
    //110 and 111 (6 and 7) correspond to 010 and 011 (2 and 3) due to compatability issues
    if(conf.count_mode==6 || conf.count_mode==7){
      conf.count_mode-=4;
    }
    break;
  case tsf_base:
    conf.bcd = (st&TIMER_BCD) == TIMER_BCD;
    break;
  default:
    return 1;
  }
  
  return timer_print_config(timer, field, conf);
}
