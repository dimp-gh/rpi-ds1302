#!/usr/bin/python
from datetime import datetime
import ConfigParser
import ds1302
import ntplib
import pytz
import traceback

CONFIG_FILE_PATH = "/etc/time.conf"

CONFIG_DEFAULTS = {
	'timezone': 'UTC',
	'sync_method': 'NTP',
	'RTC_clock': '01/01/2001 12:00',
	'NTP_server': 'pool.ntp.org',
}

class RTCClock:
	def __init__(self):
		self.config = ConfigParser.ConfigParser(CONFIG_DEFAULTS)
		self.read_config()
		self.local_tz = pytz.timezone(self.config.get('time', 'timezone'))

	def save_config(self):
		try:
			with open(CONFIG_FILE_PATH, 'w+') as configfile:
				self.config.write(configfile)
		except:
			print 'saving config file failed'
			traceback.print_exc()

	def read_config(self):
		try:
			self.config.read(CONFIG_FILE_PATH)
		except:
			print 'reading config file failed'
			traceback.print_exc()
		if not self.config.has_section('time'):
			self.config.add_section('time')

	def get_rtc_datetime_utc(self):
		if not self.check_rtc_sanity():
			now = datetime.now()
		else:
			year, month, date = ds1302.get_date()
			hour, minute, second = ds1302.get_time()
			# rangechecks are important
			if year < 2000 or year > 3000:
				ds1302.set_date(2000, month, date)
				return self.get_rtc_datetime_utc()
			if month not in range(1, 13):
				ds1302.set_date(year, 1, date)
				return self.get_rtc_datetime_utc()
			if date not in range(1, 32):
				ds1302.set_date(year, month, 1)
				return self.get_rtc_datetime_utc()
			if hour not in range(0, 24):
				ds1302.set_time(0, minute, second)
				return self.get_rtc_datetime_utc()
			if minute not in range(0, 60):
				ds1302.set_time(hour, 0, second)
				return self.get_rtc_datetime_utc()
			if second not in range(0, 60):
				ds1302.set_time(hour, minute, 0)
				return self.get_rtc_datetime_utc()
			now = datetime(year, month, date,
						   hour, minute, second)
		return pytz.utc.localize(now)

	def get_rtc_datetime(self):
		utc = self.get_rtc_datetime_utc()
		return utc.astimezone(self.local_tz)

	def set_rtc_datetime(self, dt):
		localtime = self.local_tz.localize(dt)
		utctime = localtime.astimezone(pytz.utc)
		if not self.check_rtc_sanity():
			return
		ds1302.set_date(utctime.year, utctime.month, utctime.day)
		ds1302.set_time(utctime.hour, utctime.minute, 0)
	
	def check_rtc_sanity(self):
		# TODO
		return True
	
	def check_ntp_sanity(self, pool):
		# TODO
		return False

	def request_ntp_datetime(self, pool='pool.ntp.org', version=3):
		try:
			c = ntplib.NTPClient()
			response = c.request(pool, version=version)
			now = datetime.fromtimestamp(response.tx_time)
		except:
			now = self.get_rtc_datetime()
		return pytz.utc.localize(now)
	
	def set_rtc_from_ntp(self):
		pool=self.config.get('time', 'NTP_server')
		if not self.check_ntp_sanity(pool):
			localized = self.request_ntp_datetime(pool)
		else:
			utc_dt = self.request_ntp_datetime(pool=self.config.get('time', 'NTP_server'))
			localized = utc_dt.astimezone(self.local_tz)
		self.set_rtc_datetime(localized.replace(tzinfo=None))

	def update_tz(self, tz):
		self.config.set('time', 'timezone', tz)
		self.local_tz = self.config.get('time', 'timezone')
	
	def get_datetime(self):
		return self.get_rtc_datetime()

def format_time(dt):
	fmt = "%m/%d/%Y %H:%M"
	return dt.strftime(fmt)
	
def parse_time(s):
	fmt = "%m/%d/%Y %H:%M"
	return datetime.strptime(s, fmt)












