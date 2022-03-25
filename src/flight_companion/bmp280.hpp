#ifndef BMP280_H_
#define BMP280_H_

bool init_bmp280();

void baro_task(void *pvParameters);

#endif
