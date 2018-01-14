# Classic-Car-Signals
A removable turn signal system for vehicles.

General Technical Specs:
1. Five ESP8266-12F processors communicating over UDP.
2. One of the ESPs is an ACCESS POINT (REMOTE/MASTER), the other four are STATIONS (LIGHTS/SLAVES).
3. The SLAVE arduino project requires the user to configurate the "location" and "IP" of the station. Please see Read-Me file in the software folder.
4. Only one system was ever produced and tested.
