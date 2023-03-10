#include "Arduino.h"
#include "Arduino_FreeRTOS.h"
#include "RF24.h"
#include "avr/interrupt.h"
#include "semphr.h"
#include "EEPROM.h"
#include "ZHConfig.h"

void sendButtonPushing(void *pvParameters);
float getBatteryLevelCharge(void);
void loadId(void);

int16_t id{abs((int16_t)ID)};

RF24 radio(9, 10);
SemaphoreHandle_t buttonSemaphore;

void setup()
{
  loadId();

  EICRA |= (1 << ISC11) | (1 << ISC10);
  EIMSK |= (1 << INT1);
  ADCSRA &= ~(1 << ADEN);
  radio.begin();
  radio.setChannel(120);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setPayloadSize(14);
  radio.setAddressWidth(3);
  radio.setCRCLength(RF24_CRC_8);
  radio.setRetries(15, 15);
  radio.openWritingPipe(0xDDEEFF);
  radio.powerDown();
  buttonSemaphore = xSemaphoreCreateBinary();
  xTaskCreate(sendButtonPushing, "Send Button Pushing", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}

void loop()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  portENTER_CRITICAL();
  sleep_enable();
  portEXIT_CRITICAL();
  sleep_cpu();
  sleep_reset();
}

void sendButtonPushing(void *pvParameters)
{
  (void)pvParameters;
  for (;;)
  {
    xSemaphoreTake(buttonSemaphore, portMAX_DELAY);
    rf_transmitted_data_t sensor{id, RFST_TOUCH_SWITCH};
    sensor.value_1 = getBatteryLevelCharge() * 100;
    radio.powerUp();
    radio.flush_tx();
    radio.write(&sensor, sizeof(rf_transmitted_data_t));
    radio.powerDown();
    sei();
  }
  vTaskDelete(NULL);
}

float getBatteryLevelCharge()
{
  ADMUX = (1 << REFS0) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1);
  ADCSRA |= (1 << ADEN);
  delay(10);
  ADCSRA |= (1 << ADSC);
  while (bit_is_set(ADCSRA, ADSC))
    ;
  ADCSRA &= ~(1 << ADEN);
  return (1024 * 1.1) / (ADCL + ADCH * 256);
}

void loadId()
{
  cli();
  if (EEPROM.read(511) == 254)
    EEPROM.get(0, id);
  else
  {
    EEPROM.write(511, 254);
    EEPROM.put(0, id);
  }
  delay(50);
  sei();
}

ISR(INT1_vect)
{
  cli();
  xSemaphoreGiveFromISR(buttonSemaphore, NULL);
}