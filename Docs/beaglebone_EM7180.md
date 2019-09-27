# Using Ultimate Sensor Fusion IMU with Beaglebone

We use the Ultimate Sensor Fusion Solution IMU with pressure sensor from:

https://www.tindie.com/products/onehorse/ultimate-sensor-fusion-solution-mpu9250/

Along with the Python library for using over i2c from:

https://github.com/simondlevy/EM7180/blob/master/extras/python/em7180/__init__.py

Which makes use of python-smbus.  SMBUS doesn't work with Python3 on Beaglebone black, so to change it for use with Python2.7, you must replace all instances of 'bytes' with 'bytearray' in em7180/__init__.py
