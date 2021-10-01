#include <axp20x.h>
#include <cache_global.hpp>
#include "esp32-hal-cpu.h"

AXP20X_Class axp;
bool axpIrq = 1;

// variável display foi declarado dentro de screen.h
/**
   Layout
   # esp32_uptime
   # TYPE esp32_uptime gauge
   esp32_uptime 23899
*/
void setMetric(String *p, String metric, String value)
{
    *p += "# " + metric + "\n";
    *p += "# TYPE " + metric + " gauge\n";
    *p += "" + metric + " ";
    *p += value;
    *p += "\n";
}

void power_led_off()
{
    axp.setChgLEDMode(AXP20X_LED_OFF); // LED off
}

void power_led_blink_low_rate()
{
    axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ); // 1blink/sec, low rate
}

void power_led_blink_high_rate()
{
    axp.setChgLEDMode(AXP20X_LED_BLINK_4HZ); // 4blink/sec, high rate
}

void power_led_on()
{
    // Está causando kernel panic, não sei o motivo ainda
    axp.setChgLEDMode(AXP20X_LED_LOW_LEVEL); // LED full on
}

void print_sys_diagnostic()
{
    Serial.printf("axp20x timer status....................: %d\n", axp.getTimerStatus());
    Serial.printf("axp20x default v warning level 1.......: %u mV \n", axp.getVWarningLevel1());
    Serial.printf("axp20x v warning level 1...............: %u mV \n", axp.getVWarningLevel1());
    Serial.printf("axp20x default v warning level 2.......: %u mV \n", axp.getVWarningLevel2());
    Serial.printf("axp20x v warning level 2...............: %u mV \n", axp.getVWarningLevel2());
    Serial.printf("axp20x default power down voltage......: %u mV \n", axp.getPowerDownVoltage());
    Serial.printf("axp20x power down voltage..............: %u mV \n", axp.getPowerDownVoltage());
    Serial.println("axp20x power output dcdc3..............: ON");
    Serial.println("axp20x power output exten..............: ON");
    Serial.println("axp20x power output lora...............: ON");
    Serial.println("axp20x power output gnss...............: ON");
    Serial.println("axp20x power output ld04...............: ON");
    Serial.println("axp20x power output dcdc2..............: ON");
    Serial.println("axp20x power output oled pins..........: ON");
    Serial.printf("axp20x dc2 enabled.....................: %d\n", axp.isDCDC2Enable());
    Serial.printf("axp20x dcdc2 voltage...................: %d mV\n", axp.getDCDC2Voltage());
    Serial.printf("axp20x dc3 enabled.....................: %d\n", axp.isDCDC3Enable());
    Serial.printf("axp20x dcdc3 voltage...................: %d mV\n", axp.getDCDC3Voltage());
    Serial.printf("axp20x ldo2 enabled....................: %d\n", axp.isLDO2Enable());
    Serial.printf("axp20x ldo2 voltage....................: %d mV\n", axp.getLDO2Voltage());
    Serial.printf("axp20x ldo3 enabled (GNSS).............: %d\n", axp.isLDO3Enable());
    Serial.printf("axp20x ldo3 voltage (GNSS).............: %d mV\n", axp.getLDO3Voltage());
    Serial.printf("axp20x ldo4 enabled....................: %d\n", axp.isLDO4Enable());
    Serial.printf("axp20x ldo4 voltage....................: %d mV\n", axp.getLDO4Voltage());
    Serial.printf("axp20x exten enabled...................: %d\n", axp.isExtenEnable());
    Serial.printf("axp20x battery voltage.................: %f\n", axp.getBattVoltage());
    Serial.printf("axp20x battery percentage..............: %i\n", axp.getBattPercentage());
    Serial.printf("axp20x battery charge current set......: %f\n", axp.getSettingChargeCurrent());
    Serial.printf("axp20x temperature......................: %f\n", axp.getTemp());
    Serial.printf("axp20x ts temperature...................: %f\n", axp.getTSTemp());
    Serial.printf("ESP frequency...........................: %d MHz\n", ESP.getCpuFreqMHz());
}

void cache_core_status()
{
    sys_cache.battery_voltage = axp.getBattVoltage();
#ifdef AXP20X
    sys_cache.battery_percentage = axp.getBattPercentage();
#endif
    sys_cache.esp32_temperature = axp.getTemp();
    sys_cache.esp32_ts_temperature = axp.getTSTemp();
    // sys_cache.sketch_size = ESP.getSketchSize();
    // sys_cache.flash_size = ESP.getFreeSketchSpace();
    // int available_size = flash_size - sketch_size;
    sys_cache.uptime = millis();
    sys_cache.power_down_voltage = axp.getPowerDownVoltage();
#if defined(SYSCACHE_LOG_ENABLED)
    Serial.printf("SBV = %f V ", sys_cache.battery_voltage);
#ifdef AXP20X
    Serial.printf("SBP = %d % ", sys_cache.battery_percentage);
#endif
    Serial.printf("ST1 = %f *C ", sys_cache.esp32_temperature);
    Serial.printf("ST2 = %f *C \n", sys_cache.esp32_ts_temperature);
#endif
}

void configure_system()
{
    Serial.println("configuring system");

    const uint8_t i2c_sda = 21;
    const uint8_t i2c_scl = 22;
    Wire.begin(i2c_sda, i2c_scl);

    int ret = 1;
    while (ret != 0)
    {
        ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);
        Serial.printf("axp20x.................................: %d\n", ret);
    }

    axp.setVWarningLevel1(3450);
    axp.setVWarningLevel2(3400);
    // axp.setPowerDownVoltage(2600);
    axp.setDCDC1Voltage(3300); // for the OLED power
    axp.setLDO3Voltage(3500);

    // Liga a energia usando AXP
    axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON); // I foresee similar benefit for restting T-watch
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON); // ???
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);  // LORA power + provides power to GPS backup battery
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);  // GPS main power (NEO chip)
    axp.setPowerOutPut(AXP202_LDO4, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON); // DCDC2 used as DCDC for your needs, in V8
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON); // power ESP + OLED pins + some other
    setCpuFrequencyMhz(240);                     // 80, 160, 240

    if (axpIrq)
    {
        axpIrq = 0;
        axp.readIRQ();
        Serial.println("axp20x irq enter!");
        if (axp.isAcinOverVoltageIRQ())
        {
            Serial.printf("isAcinOverVoltageIRQ\n");
        }
        if (axp.isAcinPlugInIRQ())
        {
            Serial.printf("isAcinPlugInIRQ\n");
        }
        if (axp.isAcinRemoveIRQ())
        {
            Serial.printf("isAcinRemoveIRQ\n");
        }
        if (axp.isVbusOverVoltageIRQ())
        {
            Serial.printf("isVbusOverVoltageIRQ\n");
        }
        if (axp.isVbusPlugInIRQ())
        {
            Serial.printf("isVbusPlugInIRQ\n");
        }
        if (axp.isVbusRemoveIRQ())
        {
            Serial.printf("isVbusRemoveIRQ\n");
        }
        if (axp.isVbusLowVHOLDIRQ())
        {
            Serial.printf("isVbusLowVHOLDIRQ\n");
        }
        if (axp.isBattPlugInIRQ())
        {
            Serial.printf("isBattPlugInIRQ\n");
        }
        if (axp.isBattRemoveIRQ())
        {
            Serial.printf("isBattRemoveIRQ\n");
        }
        if (axp.isBattEnterActivateIRQ())
        {
            Serial.printf("isBattEnterActivateIRQ\n");
        }
        if (axp.isBattExitActivateIRQ())
        {
            Serial.printf("isBattExitActivateIRQ\n");
        }
        if (axp.isChargingIRQ())
        {
            Serial.printf("isChargingIRQ\n");
        }
        if (axp.isChargingDoneIRQ())
        {
            Serial.printf("isChargingDoneIRQ\n");
        }
        if (axp.isBattTempLowIRQ())
        {
            Serial.printf("isBattTempLowIRQ\n");
        }
        if (axp.isBattTempHighIRQ())
        {
            Serial.printf("isBattTempHighIRQ\n");
        }
        if (axp.isChipOvertemperatureIRQ())
        {
            Serial.printf("isChipOvertemperatureIRQ\n");
        }
        if (axp.isChargingCurrentLessIRQ())
        {
            Serial.printf("isChargingCurrentLessIRQ\n");
        }
        if (axp.isDC2VoltageLessIRQ())
        {
            Serial.printf("isDC2VoltageLessIRQ\n");
        }
        if (axp.isDC3VoltageLessIRQ())
        {
            Serial.printf("isDC3VoltageLessIRQ\n");
        }
        if (axp.isLDO3VoltageLessIRQ())
        {
            Serial.printf("isLDO3VoltageLessIRQ\n");
        }
        if (axp.isPEKShortPressIRQ())
        {
            Serial.printf("isPEKShortPressIRQ\n");
        }
        if (axp.isPEKLongtPressIRQ())
        {
            Serial.printf("isPEKLongtPressIRQ\n");
        }
        if (axp.isNOEPowerOnIRQ())
        {
            Serial.printf("isNOEPowerOnIRQ\n");
        }
        if (axp.isNOEPowerDownIRQ())
        {
            Serial.printf("isNOEPowerDownIRQ\n");
        }
        if (axp.isVBUSEffectiveIRQ())
        {
            Serial.printf("isVBUSEffectiveIRQ\n");
        }
        if (axp.isVBUSInvalidIRQ())
        {
            Serial.printf("isVBUSInvalidIRQ\n");
        }
        if (axp.isVUBSSessionIRQ())
        {
            Serial.printf("isVUBSSessionIRQ\n");
        }
        if (axp.isVUBSSessionEndIRQ())
        {
            Serial.printf("isVUBSSessionEndIRQ\n");
        }
        if (axp.isLowVoltageLevel1IRQ())
        {
            Serial.printf("isLowVoltageLevel1IRQ\n");
        }
        if (axp.isLowVoltageLevel2IRQ())
        {
            Serial.printf("isLowVoltageLevel2IRQ\n");
        }
        if (axp.isTimerTimeoutIRQ())
        {
            Serial.printf("isTimerTimeoutIRQ\n");
            axp.offTimer();
            axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
        }
        if (axp.isPEKRisingEdgeIRQ())
        {
            Serial.printf("isPEKRisingEdgeIRQ\n");
        }
        if (axp.isPEKFallingEdgeIRQ())
        {
            Serial.printf("isPEKFallingEdgeIRQ\n");
        }
        if (axp.isGPIO3InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO3InputEdgeTriggerIRQ\n");
        }
        if (axp.isGPIO2InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO2InputEdgeTriggerIRQ\n");
        }
        if (axp.isGPIO1InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO1InputEdgeTriggerIRQ\n");
        }
        if (axp.isGPIO0InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO0InputEdgeTriggerIRQ\n");
        }
        axp.clearIRQ();
    }
}
