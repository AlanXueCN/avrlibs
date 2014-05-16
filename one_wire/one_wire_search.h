/**
 * @file one_wire_search.h
 * Функция поиска устройств на шине 1-wire.
 */

#ifndef ONE_WIRE_SEARCH_H
#define ONE_WIRE_SEARCH_H

#include "one_wire.h"

/**
 * Ищет устройства на шине 1-wire.
 * @param ow Шина 1-wire.
 * @param roms Идентификаторы устройств.
 * @param roms_count Число идентификаторов.
 * @param roms_found Число найденных устройств.
 * @param max_attempts Число повторов при ошибке для каждого устройства.
 * @return Код ошибки.
 */
extern err_t one_wire_search_roms(one_wire_t* ow, one_wire_rom_id_t* roms,
                                  uint8_t roms_count, uint8_t* roms_found,
                                  uint8_t max_attempts);

#endif  //ONE_WIRE_SEARCH_H
