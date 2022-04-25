#ifndef BMP280_H_
#define BMP280_H_

#include <CopilotMessages.hpp>

bool initBMP280();

baro_struct_t read_barometer();

#endif
