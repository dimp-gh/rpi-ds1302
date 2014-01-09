#!/usr/bin/env python
import os
from distutils.core import setup, Extension

gcc_bin = os.environ.get('CC', 'gcc')
includes = os.environ.get('INCLUDE', '/usr/include')
libgccfilename = os.popen(gcc_bin + " -print-libgcc-file-name").read().strip()

c_nviro = Extension('ds1302',
					sources = ['python.c'],
					extra_link_args = [libgccfilename,],
					libraries = ['wiringPi', 'm'],
					extra_compile_args = ['-std=c99'],
					include_dirs=[includes],
				)

setup(name='rpi_rtc',
	  version="0.1",
	  description='Python library for handling DS1302 RTC with Raspberry Pi',
	  ext_modules=[c_nviro],
	  py_modules=['rpi_time'],
	  )
