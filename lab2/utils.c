#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  //IMPLEMENTED
  *lsb = (uint8_t) val; //ao dar cast de uint8 vai pegar os 8 bits menos significativos

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  //IMPLEMENTED
  *msb = (uint8_t) (val >> 8);

  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  //IMPLEMENTED
  //implementa-se pq sys_inb() nao aceita variables de 8 bits
  //o sys_inb() vai por em st um valor em 32 bits e dps passamos para value com 8 bits
  uint32_t stub;
  if(sys_inb(port, &stub) != OK){
    return 1;
  }

  *value = (uint8_t) stub;

  return 0;
}
