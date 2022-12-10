# RF touch switch

Touch switch on ATmega168/328 + RF24.

## Features

1. Power consumption in sleep mode approximately 10 μA. Up to 2 years of operation on one CR2032 battery (estimated).
2. When triggered transmits battery level charge.
3. Automatic restart in case of a hang-up.
4. Easy installation into any standard touch switch enclosure.

## Note

A gateway is required. For details see "RF - ESP-NOW Gateway".

## Full config example for Home Assistant

```yml
mqtt:
  sensor:
    - name: "NAME"
      device_class: "voltage"
      unit_of_measurement: "V"
      state_topic: "homeassistant/rf_sensor/ID/touch_switch"
      value_template: "{{ value_json.battery }}"
      force_update: true
      qos: 2
```
