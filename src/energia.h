#include <axp20x.h>

AXP20X_Class axp;
bool axpIrq = 0;

// variável display foi declarado dentro de screen.h

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
    axp.setChgLEDMode(AXP20X_LED_LOW_LEVEL); // LED full on
}

void energia_setup()
{
    const uint8_t i2c_sda = 21;
    const uint8_t i2c_scl = 22;
    Wire.begin(i2c_sda, i2c_scl);

    int ret = axp.begin(Wire, AXP192_SLAVE_ADDRESS);

    if (ret == AXP_FAIL)
    {
        Serial.println("AXP Power begin failed");
        while (1)
            ;
    }
    else
    {
        Serial.println("AXP192 OK");
    }

    Serial.println();
    axp.setVWarningLevel1(3450);
    axp.setVWarningLevel2(3400);
    uint16_t level1 = axp.getVWarningLevel1();
    uint16_t level2 = axp.getVWarningLevel2();
    Serial.printf("getVWarningLevel1:%u mV \n", level1);
    Serial.printf("getVWarningLevel2:%u mV \n", level2);
    Serial.printf("getPowerDonwVoltage:%u mV \n", axp.getPowerDownVoltage());
    axp.setPowerDownVoltage(2600);
    Serial.printf("getPowerDonwVoltage:%u mV \n", axp.getPowerDownVoltage());
    Serial.println();

    power_led_off(); // Led azul que doi os olhos

    delay(1000);

    // Liga a energia usando AXP
    axp.setPowerOutPut(AXP202_DCDC3, true); // ???
    axp.setPowerOutPut(AXP192_EXTEN, true); // ???
    axp.setPowerOutPut(AXP192_LDO2, true);  // LORA power
    axp.setPowerOutPut(AXP192_LDO3, true);  // GPS power (NEO chip)
    axp.setPowerOutPut(AXP202_LDO4, true);
    axp.setPowerOutPut(AXP192_DCDC2, true); // DCDC2 used as DCDC for your needs, in V8
    axp.setPowerOutPut(AXP192_DCDC1, true); // power to OLED pins + some other
    axp.setLDO4Voltage(AXP202_LDO4_3300MV);
    axp.setLDO3Voltage(3500);
    axp.setPowerOutPut(AXP202_LDO3, true);

    Serial.print("DC2:");
    Serial.print(axp.isDCDC2Enable() ? String(axp.getDCDC2Voltage()) + " mv" : "DISABLE");
    Serial.println("  ");

    Serial.print("DC3:");
    Serial.print(axp.isDCDC3Enable() ? String(axp.getDCDC3Voltage()) + " mv" : "DISABLE");
    Serial.println("  ");

    Serial.print("LDO2:");
    Serial.print(axp.isLDO2Enable() ? String(axp.getLDO2Voltage()) + " mv" : "DISABLE");
    Serial.println("  ");

    Serial.print("LDO3:");
    Serial.print(axp.isLDO3Enable() ? String(axp.getLDO3Voltage()) + " mv" : "DISABLE");
    Serial.println("  ");

    Serial.print("LDO4:");
    Serial.print(axp.isLDO4Enable() ? "ENABLE" : "DISABLE");
    Serial.println("  ");

    Serial.print("Exten:");
    Serial.print(axp.isExtenEnable() ? "ENABLE" : "DISABLE");
    Serial.println(" ");

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

void print_esp_temp()
{
    float ts = axp.getTemp();
    Serial.printf("ESP Temperadura: %g graus\n", ts);

    digitalWrite(14, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(1000);            // wait for a second
    digitalWrite(14, LOW);  // turn the LED off by making the voltage LOW
    delay(1000);            // wait for a second

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(14, OUTPUT);
}
