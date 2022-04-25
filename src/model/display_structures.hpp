typedef struct smart_float_type_struct
{
    bool updated = false;
    float _value = 0;
    void setValue(float value)
    {
        if (this->_value != value)
        {
            this->_value = value;
            this->updated = true;
        }
    }
    float getValue(bool outdate_on_read = true)
    {
        if ((this->updated) and (outdate_on_read))
        {
            this->updated = false;
        }
        return this->_value;
    }

    String toString()
    {
        return String(this->getValue(true));
    }

} smart_float_type_struct;

typedef struct smart_int_type_struct
{
    bool updated = false;
    int _value = 0;
    void setValue(int value)
    {
        if (this->_value != value)
        {
            this->_value = value;
            this->updated = true;
        }
    }
    int getValue(bool outdate_on_read = true)
    {
        if ((this->updated) and (outdate_on_read))
        {
            this->updated = false;
        }
        return this->_value;
    }

    String toString()
    {
        return String(this->getValue(true));
    }

} smart_int_type_struct;

struct display_sysinfo_struct
{
    smart_int_type_struct battery_p;
    smart_float_type_struct voltage;
    unsigned long last_data_received_millis;
};

struct display_baro_struct
{
    smart_int_type_struct altitude;
    smart_int_type_struct temperature;
    smart_int_type_struct pressure;
    smart_float_type_struct vario;
};

struct display_gnss_struct
{
    smart_float_type_struct latitude;
    smart_float_type_struct longitude;
    smart_float_type_struct altitude;
    smart_float_type_struct speed;
    smart_int_type_struct course;
    smart_int_type_struct satellites;
};

typedef struct DisplayBuffer
{
    display_baro_struct baro;
    display_gnss_struct gnss;
    display_sysinfo_struct sysinfo;
} DisplayBuffer;
