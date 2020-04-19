# Autonomous plant watering system

## Introduction

This is a project to water the plants automatically when the soil moisture is low, while running autonomously using a solar panel and a battery.
The system also communicates using Telegram to send information about the watering (watering duration, alerts...), and accept requests from the user such as turning the system OFF or ON, returning the soil moisture or simply telling if the system is up and running.

## Material

Here is a list of the products used to build the system. I must say that I receive no incentives from Amazon, from which all products were bought.

To control the system:
- [NodeMCU board (ESP8266)](https://www.amazon.fr/gp/product/B0754HWZSQ/ref=ppx_od_dt_b_asin_title_s02?ie=UTF8&psc=1) for the brain, 17.99€
- [Relay module](https://www.amazon.fr/gp/product/B07GRW83FR/ref=ppx_od_dt_b_asin_title_s02?ie=UTF8&psc=1), 11.99€
- [120 prototyping jumper wires](https://www.amazon.fr/gp/product/B01JD5WCG2/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1), 6.99€  -> prototyping
- [3 breadboards](https://www.amazon.fr/gp/product/B06XKZYFYN/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1), 8.99€ -> prototyping
- [Waterproof box](https://www.amazon.fr/gp/product/B00HW0OMKU/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1), 10.99€
- [525 pieces resistors kit](https://www.amazon.fr/gp/product/B071LHFQKD/ref=ppx_yo_dt_b_asin_title_o03_s01?ie=UTF8&psc=1), 10.99€
- [Printed PCB](https://www.amazon.fr/gp/product/B07G5CRQXK/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&th=1) with connections similar to a breadboard, 9.27€
- [Electrical stranded wires 20, 22 or 24 AWG](https://www.amazon.fr/gp/product/B07G72DRKC/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&th=1) depending on your preference (20 is more solid but need to be reduced for some connections, 22 is good, 24 is cheaper), 18.99€

For the autonomy in energy:
- [12V Battery](https://www.amazon.fr/gp/product/B009D0KFOO/ref=ppx_yo_dt_b_asin_title_o06_s00?ie=UTF8&psc=1), 21.90€
- [10W wp 12V monocrystalline solar panel](https://www.amazon.fr/gp/product/B007HAZY8Y/ref=ppx_yo_dt_b_asin_title_o05_s02?ie=UTF8&psc=1), 23.90€
- [12/24V charge controller](https://www.amazon.fr/gp/product/B071ZZ2S84/ref=ppx_yo_dt_b_asin_title_o05_s02?ie=UTF8&psc=1), 13.99€

For the water tank:
- [12V water pump](https://www.amazon.fr/gp/product/B07NPN5XBS/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1), 16.99€
- [DC male/female connector](https://www.amazon.fr/gp/product/B07BPSCNSM/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1) (to connect the pump), 6.99€

The sensors:
- [Water level floater](https://www.amazon.fr/gp/product/B01MTYPK9I/ref=ppx_yo_dt_b_asin_title_o05_s02?ie=UTF8&psc=1), 7.99€
- [Capacitive soil moisture sensor](https://www.amazon.fr/gp/product/B07WCB3GCB/ref=ppx_od_dt_b_asin_title_s02?ie=UTF8&psc=1), 9.49€

And the irrigation system:
- [Irrigation system](https://www.amazon.fr/gp/product/B07MSJSMXK/ref=ppx_yo_dt_b_asin_title_o05_s01?ie=UTF8&psc=1), 22.97€

For a total of 229.41€, which is not cheap! But keep in mind that it's still cheaper than a pre-built system with a lot less capabilities. Also, some parts are only for prototyping (15.98€), and I purchased many components in groups of several pieces to re-use the parts for other projects, e.g. 525 resistors is a crazy amount, you don't need 3 NodeMCU boards, nor 6 relays for this project.
