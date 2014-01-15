from rpi_time import DS1302

DATE_FORMAT_STRING = "%m/%d/%Y %H:%M:%S %Z"

def format_time(dt):
    return dt.strftime(DATE_FORMAT_STRING)

def main():
	rtc = DS1302()
	now = rtc.get_datetime()
	if now is None:
		print "RTC is not installed (fails sanity check)"
	else:
		print format_time(now)

if __name__ == '__main__':
	main()
