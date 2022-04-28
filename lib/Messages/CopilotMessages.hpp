#ifndef _MESSAGES_H_
#define _MESSAGES_H_

typedef int pressure_t;

struct gnss_struct_t
{
  uint32_t sat_count = 0;
  double location_lat = 0;
  double location_lng = 0;
  double altitude_meters = 0;
  double speed_kmph = 0;
  int32_t course_value = 0;
  double course_deg = 0;
  const char *course_cardinal = "";
  uint32_t date = 0;
  uint32_t time = 0;
  int32_t hdop = 0;

  String toString()
  {
    return "sat=" + String(sat_count) +
           " altitude=" + String(altitude_meters) +
           " lat=" + String(location_lat) +
           " lng=" + String(location_lng) +
           " kmph=" + String(speed_kmph) +
           " course_value=" + String(course_value) +
           " course_deg=" + String(course_deg) +
           " course_cardinal=" + String(course_cardinal) +
           " date=" + String(date) +
           " time=" + String(time) +
           " hdop=" + String(hdop);
  }
};

struct baro_struct_t
{
  int temperature = 0;
  pressure_t pressure = 0;
  int altitude = 0;
  float vario = 0;

  String toString()
  {
    return "temperature=" + String(temperature) +
           " pressure=" + String(pressure) +
           " altitude=" + String(altitude) +
           " vario=" + String(vario);
  }
};

typedef struct metrics_data
{
  baro_struct_t baro_data;
  gnss_struct_t gnss_data;

  String toString()
  {
    return baro_data.toString() + " " + gnss_data.toString();
  }
} metrics_data;

#endif
