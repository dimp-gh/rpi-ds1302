#include <Python.h>
#include <time.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

// Pins that are used for RTC clock
static int PinClk=4;
static int PinDat=5;
static int PinRst=6;

#define RTC_SEC 0x81
#define RTC_MIN 0x83
#define RTC_HOUR 0x85
#define RTC_DATE 0x87
#define RTC_MONTH 0x89
#define RTC_DAY 0x8b
#define RTC_YEAR 0x8d

void rtc_write(unsigned char);
void rtc_transm_end(void);
unsigned char rtc_read(void);
unsigned char time_get(unsigned char);
void rtc_wp(void);
void rtc_reset(void);
char *rtc_read_time_str(void);
char *rtc_read_date_str(void);

//посылаем команду или байт данных в часы
void rtc_write(unsigned char cmd) {
	digitalWrite(PinRst,HIGH);
	delayMicroseconds(4);
	for(unsigned char i=0; i<8; i++) {
		if((cmd&(1<<i)) == 1<<i) {
			digitalWrite(PinDat,HIGH);
		} else {
			digitalWrite(PinDat,LOW);
		}
		delayMicroseconds(1);
		digitalWrite(PinClk,HIGH);
		delayMicroseconds(1);
		digitalWrite(PinClk,LOW);
		digitalWrite(PinDat,LOW);
	} 
}

//вызываем после записи байта данных в часы
void rtc_transm_end() {
	digitalWrite(PinRst,LOW);
}

//чтение данных
unsigned char rtc_read() {
	unsigned char readbyte=0;//сюда будем читать 
	pinMode(PinDat,INPUT);
	//читаем побитно, если "1" записываем "1" в соответствующий бит, также с "0"
	for(unsigned char i=0;i<8;i++) {
		if(digitalRead(PinDat)==0) {
			readbyte &= ~(1<<i);
		} else {
			readbyte |= 1<<i;
		}
		digitalWrite(PinClk,HIGH);
		delayMicroseconds(10);//а это и есть большая задержка, чем при передаче
		digitalWrite(PinClk,LOW);
		delayMicroseconds(2);
	}
	delayMicroseconds(4);
	pinMode(PinDat,OUTPUT);
	return readbyte;
}

unsigned char time_get(unsigned char addr) {
	rtc_write(addr);
	unsigned char ret = rtc_read();
	rtc_transm_end();
	return ret;
}

void time_set(unsigned char addr, unsigned char value) {
	rtc_write(addr-1);
	rtc_write(value);
	rtc_transm_end();
}

void rtc_wp() {
	rtc_write(0x8E);
	rtc_write(0x00);
	rtc_transm_end();
}

void rtc_reset() {
	time_set(RTC_SEC, 0x0);
	time_set(RTC_MIN, 0x0);
	time_set(RTC_HOUR, 0x0);
	time_set(RTC_DATE, 0x01);
	time_set(RTC_MONTH, 0x01);
	time_set(RTC_YEAR, 0x1);
}

char *rtc_read_time_str() {
	char *buff = malloc(sizeof(char) * 10);
	sprintf(buff, "%02x:%02x:%02x",time_get(RTC_HOUR),time_get(RTC_MIN),time_get(RTC_SEC));
	return buff;
}

char *rtc_read_date_str() {
	char *buff = malloc(sizeof(char) * 12);
	sprintf(buff, "%02x-%02x-20%02x",time_get(RTC_DATE),time_get(RTC_MONTH),time_get(RTC_YEAR));
	return buff;
}

unsigned char to_bcd(unsigned char hexed) {
	// 11 -> 0x11 -> 17
	// 69 -> 0x69 -> 105
	unsigned char n = hexed, digit = 0, power = 0;
	unsigned char decimal = 0;
	while (n != 0) {
		digit = n % 10;
		decimal += (unsigned char) pow(16, power) * digit;
		power += 1;
		n /= 10;
	}
	return decimal;
}

unsigned char from_bcd(unsigned char decimal) {
	// 105 -> 0x69 -> 69
	// 17 -> 0x11 -> 11
	unsigned char n = decimal, digit = 0, power = 0;
	unsigned char hexed = 0;
	while (n != 0) {
		digit = n % 16;
		if (digit > 9)
		  return -1;
		hexed += pow(10, power) * digit;
		power += 1;
		n /= 16;
	}
	return hexed;
}

static PyObject *
py_ds1302_get_time_str(PyObject *self, PyObject *args) {
	return PyString_FromString(rtc_read_time_str());
}

static PyObject *
py_ds1302_get_date_str(PyObject *self, PyObject *args) {
	return PyString_FromString(rtc_read_date_str());
}

static PyObject *
py_ds1302_set_date(PyObject *self, PyObject *args) {
	const unsigned char date, month;
	const unsigned int year;
	if (!PyArg_ParseTuple(args, "ibb", &year, &month, &date))
		return NULL;
	time_set(RTC_DATE, to_bcd(date));
	time_set(RTC_MONTH, to_bcd(month));
	time_set(RTC_YEAR, to_bcd((unsigned char)(year - 2000)));
	Py_RETURN_NONE;
}

static PyObject *
py_ds1302_set_time(PyObject *self, PyObject *args) {
	const unsigned char hour, min, sec;
	if (!PyArg_ParseTuple(args, "bbb", &hour, &min, &sec))
		return NULL;
	time_set(RTC_HOUR, to_bcd(hour));
	time_set(RTC_MIN, to_bcd(min));
	time_set(RTC_SEC, to_bcd(sec));
	Py_RETURN_NONE;
}

static PyObject *
py_ds1302_get_date(PyObject *self, PyObject *args) {
	unsigned int year, month, date;
	year = 2000 + from_bcd(time_get(RTC_YEAR));
	month = from_bcd(time_get(RTC_MONTH));
	date = from_bcd(time_get(RTC_DATE));
	return Py_BuildValue("iii", year, month, date);
}

static PyObject *
py_ds1302_get_time(PyObject *self, PyObject *args) {
	unsigned char hour, min, sec;
	hour = from_bcd(time_get(RTC_HOUR));
	min = from_bcd(time_get(RTC_MIN));
	sec = from_bcd(time_get(RTC_SEC));
	return Py_BuildValue("iii", hour, min, sec);
}

static PyObject *
py_ds1302_reset_clock(PyObject *self, PyObject *args) {
	rtc_reset();
	Py_RETURN_NONE;
}

static PyObject *
py_ds1302_init_clock(PyObject *self, PyObject *args) {
	// configure pins
	pinMode(PinRst,OUTPUT);
	digitalWrite(PinRst,LOW);
	pinMode(PinClk,OUTPUT);
	digitalWrite(PinClk,LOW);
	pinMode(PinDat,OUTPUT);
	digitalWrite(PinDat,LOW);
	//remove write protection
	rtc_wp();
	Py_RETURN_NONE;
}

//Method table
static PyMethodDef ds1302Methods[] = {
	{"get_date_str", py_ds1302_get_date_str, METH_VARARGS, "Returns date string"},
	{"get_time_str", py_ds1302_get_time_str, METH_VARARGS, "Returns time string"},
	{"get_date", py_ds1302_get_date, METH_VARARGS, "Returns (y, m, d) tuple (values can be out of range)"},
	{"get_time", py_ds1302_get_time, METH_VARARGS, "Returns (h, m, s) tuple (values can be out of range)"},
	{"set_date", py_ds1302_set_date, METH_VARARGS, "Set date (year, month, date)"},
	{"set_time", py_ds1302_set_time, METH_VARARGS, "Set time (hour, minute, second)"},
	{"reset_clock", py_ds1302_reset_clock, METH_VARARGS, "Reset RTC clock"},
	{"init_clock", py_ds1302_init_clock, METH_VARARGS, "Init RTC clock (disable write protection)"},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initds1302(void) {
	PyObject *module = Py_InitModule("ds1302", ds1302Methods);

	if (wiringPiSetup() == -1) exit (1) ;
}
