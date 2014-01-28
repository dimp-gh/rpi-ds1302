rpi-ds1302
==========

Python library for using DS1302 RTC unit with Raspberry Pi.

Depends on `WiringPi` library.

TODO:
- split C / CPython / Python code
- way to set up RTC pins from Python code

Usage
-------

Currently you have to connect RTC unit to the following RPi pins (you can read about RPi pin numbering [here](http://wiringpi.com/pins/), this library uses wiringPi pin numbering):
* CLK pin (also can be named SCLK) goes to RPi pin 4.
* DAT pin (sometimes it is named I/O) goes to RPi pin 5.
* RST pin goes to RPi pin 6.

There are two ways of using RTC unit from Python code.

*Low-level* access procedures are implemented in `ds1302` module.
```python
>>> import ds1302
>>> ds1302.get_date()
(2014, 1, 20)
>>> ds1302.get_time()
(15, 56, 1)
>>> ds1302.set_time(14, 14, 14)
>>> ds1302.get_time()
(14, 14, 18)
```
*High-level* access procedures are implemented in `rpi_time` module (this one includes sanity checks for RTC unit and range checks for clock values).
```python
>>> import rpi_time
>>> rtc = rpi_time.DS1302()
>>> rtc.get_datetime()
datetime.datetime(2014, 1, 20, 14, 16, 27)
>>> rtc.set_datetime(datetime(2014, 1, 20, 13, 42))
True
>>> rtc.get_datetime()
datetime.datetime(2014, 1, 20, 13, 42, 2)
>>> rtc.reset_clock()
>>> rtc.get_datetime()
datetime.datetime(2001, 1, 1, 0, 0, 5)
```
