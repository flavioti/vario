typedef int pressure_t;

struct baro_struct_t
{
  int temperature = 0;
  pressure_t pressure = 0;
  int altitude = 0;
  float vario = 0;
};

typedef struct metrics_data
{
  baro_struct_t baro_data;

  String toString()
  {
    return "baro_data: " + String(baro_data.temperature) + " " + String(baro_data.pressure) + " " + String(baro_data.altitude) + " " + String(baro_data.vario);
  }
} metrics_data;
