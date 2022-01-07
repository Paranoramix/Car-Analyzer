# Car-Analyzer
Boite noire pour analyser votre voiture électrique.


### Configuration du boitier
@todo

### Home Assistant Configuration

La création de l'appareil dans Home Assistant se fait automatiquement, car les 
entités se configurent elles-même.
Pour cela, il est nécessaire de prévoir la découverte dans la configuration de
l'intégration MQTT.

Il est possible d'ajouter un device_tracker, mais pour cela, il est nécessaire
de définir au niveau des automatisations, un trigger qui vérifiera si le boitier
est dans la zone 'Home' ou pas.

```
alias: Device Location Trigger
description: This automation allow to define if the car is located at home or not.
trigger:
  - platform: state
    entity_id: sensor.car_analyzer_<ChipID>_latitude
  - platform: state
    entity_id: sensor.car_analyzer_<ChipID>_longitude
condition: []
action:
  - service: device_tracker.see
    data_template:
      dev_id: car_analyzer_<ChipID>_gpslocation
      gps:
        - '{{ sensor.car_analyzer_<ChipID>_latitude }}'
        - '{{ sensor.car_analyzer_<ChipID>_longitude }}'
mode: restart
```
