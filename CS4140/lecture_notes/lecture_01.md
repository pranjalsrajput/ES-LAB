## Protocol

- CRC Checks (wireless)
- variable or constant length messages?

## System architecture

- four equal size parts
- interfaces

### Scheduling

Round-robin with interrupts: most popular

- sense
- control
- actuate
- filter values

Polling: no notion of priority, if unlucky, we have to wait: will not fly,
response time will be slow. Stochastics nature!

Predictable response time, the long things can slow down.

Interrupts with sensors.


Everyone writes code to run on the quadcopter.

Can we write PC software not in C?