#!/usr/bin/python3
#
# MS5837 device driver
#
# This code is based on
#  bluerobotics ms5837, jaxxzer
#  RTIMULib MS5837, uutzinger
#
# This driver is implemented to not lock the read command and to return 
# false if no new data was available.
# 
# The main program will need to poll the sensor on regular basis to move 
# it through the measurement sequence: 
# idle -> pressure -> temperature -> compute compensated pressure and temperature data -> valid data available.
#
# Ther recommended polling interval is given based on the sampling resolution:
#
# OVERSAMPLING/Resolution/Approx Conversion Time: 
# 8192 = 0.016mbar 17.20ms
#  256 = 0.11 mbar  0.56ms
#
# A complete reading includes one pressure and one tempreature measurement. 
# Therefore the maximum update rate is twice the approx. conversion time.
#
# Urs Utzinger, University of Arizona, 7/24/2017

import logging
import time

# Models
MODEL_02BA = 0
MODEL_30BA = 1

# Oversampling options
OSR_256  = 0
OSR_512  = 1
OSR_1024 = 2
OSR_2048 = 3
OSR_4096 = 4
OSR_8192 = 5

# kg/m^3 convenience
DENSITY_FRESHWATER = 997
DENSITY_SALTWATER = 1029

# Conversion factors (from native unit, mbar)
UNITS_Pa     = 100.0
UNITS_hPa    = 1.0
UNITS_kPa    = 0.1
UNITS_mbar   = 1.0
UNITS_bar    = 0.001
UNITS_atm    = 0.000986923
UNITS_Torr   = 0.750062
UNITS_psi    = 0.014503773773022

# Valid units
UNITS_Centigrade = 1
UNITS_Farenheit  = 2
UNITS_Kelvin     = 3

# Registers
#####################################################
  
# MS5837 default address.
MS5837_I2CADDR_0          = 0x76
MS5837_I2CADDR_1          = 0x77

# Commands
MS5837_CMD_RESET          =  0x1E # reset
MS5837_CMD_ADC            =  0x00 # adc read
MS5837_CMD_PROM           =  0xA0 # PROM read a0-ae
MS5837_CMD_CONV_D1        =  0x4A # convert D1 8192
MS5837_CMD_CONV_D2        =  0x5A # convert D2 8192
MS5837_CMD_CONV_D1_256    =  0x40 # convert D1 256
MS5837_CMD_CONV_D2_256    =  0x50 # convert D2 256

# State of Sensor
MS5837_STATE_IDLE = 0
MS5837_STATE_TEMPERATURE = 1
MS5837_STATE_PRESSURE = 2

  #####################################################


class MS5837(object):  
  def __init__(self, model=MODEL_30BA, i2c=None, **kwargs):
        
    self._logger = logging.getLogger('MS5837.MS5837')

    self._fluidDensity = DENSITY_SALTWATER
    self._pressure = 0
    self._temperature = 0
    self._model = model
    self._D1 = 0
    self._D2 = 0
    self._oversampling = OSR_8192
    self._conversionTime = (2.5e-6 * 2**(8+OSR_8192))
    
    # Create I2C device.
    if i2c is None:
      import Adafruit_GPIO.I2C as I2C
      i2c = I2C

    self._device = i2c.get_i2c_device(MS5837_I2CADDR_0, **kwargs)
    
    # Reset
    cmd = MS5837_CMD_RESET
    self._device.writeRaw8(cmd)   # Reset device
    time.sleep(0.01)              # give some time to recover
        
    # Get Calibration Data
    cmd = MS5837_CMD_PROM
    for i in range(0,6):
      self._calData[i] = self._device.readU16BE(cmd)
      cmd = cmd + 2
      
    self._CRC = (self._calData[0] & 0xF000) >> 12 
    self._CRCCalculated = self.crc4(self._calData)
    
    if (self._CRCCalculated != self._CRC ):
      print("PROM read error, CRC failed!")
      return False
      
    self._C1 = self._calData[1]   # INT16
    self._C2 = self._calData[2]   # INT16
    self._C3 = self._calData[3]   # INT16
    self._C4 = self._calData[4]   # INT16
    self._C5 = self._calData[5]   # INT16
    self._C6 = self._calData[6]   # INT16
    
    self._logger.debug('CRC = {0:6d}'.format(self._CRC))
    self._logger.debug('C1 = {0:6d}'.format(self._C1))
    self._logger.debug('C2 = {0:6d}'.format(self._C2))
    self._logger.debug('C3 = {0:6d}'.format(self._C3))
    self._logger.debug('C4 = {0:6d}'.format(self._C4))
    self._logger.debug('C5 = {0:6d}'.format(self._C5))
    self._logger.debug('C6 = {0:6d}'.format(self._C6))
  
    self._state = MS5837_STATE_IDLE
    self._validReadings = False
    
    return True

  def setOSR(self, oversampling = OSR_8192):
    if oversampling < OSR_256 or oversampling > OSR_8192:
      print("Invalid oversampling option!")
      return False
    self._oversampling = oversampling
    # Maximum conversion time increases linearly with oversampling
    # max time (seconds) ~= 2.2e-6(x) where x = OSR = (2^8, 2^9, ..., 2^13)
    # We use 2.5e-6 for some overhead
    self._conversionTime = (2.5e-6 * 2**(8+oversampling))
    return True
    
  def getPollInterval(self):
    return (self._conversionTime/3)

  def read(self):
    self._validReadings = False
    
    # IDLE
    if self._state == MS5837_STATE_IDLE:
      # Initiate Pressure Reading
      self._device.writeRaw8(MS5837_CMD_CONV_D1_256 + 2*self._oversampling )
      self.state = MS5837_STATE_PRESSURE;
      self._timer = time.monotonic()

    # PRESSURE
    elif self._state == MS5837_STATE_PRESSURE:
      
      if ((time.monotonic() - self._timer) < self._conversionTime):
        return False                    # not time yet
      d = self._device.readList(MS5837_CMD_ADC, 3)
      self._D1 = ((d[0] << 16) | (d[1] << 8) | d[2])
      self._logger.debug('Raw pressure 0x{0:04X} ({1})'.format(self._D1 &v0xFFFF, self._D1))
      # start temperature conversion
      self._device.writeRaw8(MS5837_CMD_CONV_D2_256 + 2*self._oversampling )
      self._state = MS5837_STATE_TEMPERATURE;
      self._conversionTime = (2.5e-6 * 2**(8+oversampling))
      self._timer = time.monotonic();
  
    # TEMPERATURE
    elif self._state == MS5837_STATE_TEMPERATURE:
    
      # read temperature
      if ((time.monotonic() - self._timer) < self._conversionTime):
        return False # not time yet
      d = self._device.readList(MS5837_CMD_ADC,3)
      self._D2 = ((d[0] << 16) | (d[1] << 8) | d[2])
      self._logger.debug('Raw temperature 0x{0:04X} ({1})'.format(self._D2 &v0xFFFF, self._D2))
      
      # call this function for testing only
      # should give T = 2000 (20.00C) and pressure 110002 (1100.02hPa)
      # setTestData();
      
      # NOW CALCULATE THE REAL VALUES
      dT = self._D2 - (self._C5 * 256)
      if self._model == MODEL_02BA:
        sens   = (self._C1 * 65536) + ((self._C3 * dT) / 128)
        offset = (self._C2 * 131072) + ((self._C4 * dT) / 64)
        self._pressure = (self._D1*sens/(2097152)-offset)/(32768)
      else:
        sens   = (self._C1 * 32768) + ((self._C3 * dT) / 256)
        offset = (self._C2 * 65536) + ((self._C4 * dT) / 128)
        self._pressure = (self._D1*sens/(2097152)-offset)/(8192)
      
      self._temperature = 2000 + ((dT * self._C6) / 8388608) 
      
      # Second order temperature compensation
      if self._model == MODEL_02BA:
        if (self._temperature < 2000): # Low temp
          T2 = (11*dT*dT)/(34359738368)
          offset2 = (31*(self._temperature-2000)*(self._temperature-2000))/8
          sens2 = (63*(self._temperature-2000)*(self._temperature-2000))/32
      else:
        if (self._temperature < 2000): 
          # low temp
          T2 = (3 * (dT * dT))/(8589934592);
          offset2 = (3 * ((self._temperature - 2000) * (self._temperature - 2000))) /2
          sens2 = (5 * ((self._temperature - 2000) * (self._temperature - 2000))) /8
          if (self._temperature < -1500): 
            # very low temp
            offset2 = offset2 + 7 * (self._temperature + 1500) * (self._temperature + 1500)
            sens2 = sens2 + 4 * (self._temperature + 1500) * (self._temperature + 1500)
        else:
          # high temp
          T2 = 2*(dT*dT)/(137438953472)
          offset2 = (self._temperature - 2000) * (self._temperature - 2000) / 16
          sens2 = 0;
        
      offset = offset - offset2
      sens = sens-sens2
      self._temperature = self._temperature - T2
      
      if (self._model == MODEL_02BA):
        self._pressure = ((((self._D1*sens)/2097152)-offset)/32768)/100.0
      else:
        self._pressure = ((((self._D1*sens)/2097152)-offset)/8192)/10.0 # mbar

      self._logger.debug('T = {0:6d}'.format(self._temperature))
      self._logger.debug('P = {0:6d}'.format(self._pressure))
    
      self._validReadings = True
      self._state = MS5837_STATE_IDLE
    
    if self._validReadings == True:
      return True
    else: 
      return False
    
  def crc4(self, n_prom):
    n_rem = 0
    n_prom[0] = ((n_prom[0]) & 0x0FFF)
    n_prom.append(0)
    
    for cnt in range(0,15, 1):
      if cnt%2 == 1 :
        n_rem ^= ((n_prom[cnt>>1]) & 0X00FF)
      else:
        n_rem ^= (n_prom[cnt>>1] >> 8)
        
      for n_bit in range(8,1, -1):
        if n_rem & 0x8000:
          n_rem = (n_rem << 1) ^ 0x3000
        else:
          n_rem = (n_rem << 1)
          
    n_rem = ((n_rem >> 12) & 0x000F)
  
    self.n_prom = n_prom
    self.n_rem  = n_rem
  
    return n_rem ^ 0x00

  def setTestData(self):
    self._CRC = 0
    self._C1 = 34982
    self._C2 = 36352
    self._C3 = 20328
    self._C4 = 22354
    self._C5 = 26646
    self._C6 = 26146
    self._D1 = 4958179
    self._D2 = 6815414
    
  def setFluidDensity(self, denisty):
    self._fluidDensity = denisty
    
  # Pressure in requested units
  # mbar * conversion
  def pressure(self, conversion=UNITS_mbar):
    return self._pressure * conversion
        
  # Temperature in requested units
  # default degrees C
  def temperature(self, conversion=UNITS_Centigrade):
    degC = self._temperature / 100.0
    if conversion == UNITS_Farenheit:
      return (9/5) * degC + 32
    elif conversion == UNITS_Kelvin:
      return degC - 273
    return degC

  #  Altitude() - the conversion uses the formula:
  #  h = (T0 / L0) * ((p / P0)**(-(R* * L0) / (g0 * M)) - 1)
  #  where:
  #  h  = height above sea level
  #  T0 = standard temperature at sea level = 288.15
  #  L0 = standard temperatur elapse rate = -0.0065
  #  p  = measured pressure
  #  P0 = static pressure = 1013.25
  #  g0 = gravitational acceleration = 9.80665
  #  M  = mloecular mass of earth's air = 0.0289644
  #  R* = universal gas constant = 8.31432
  #  Given the constants, this works out to:
  #  h = 44330.8 * (1 - (p / P0)**0.190263)
  #
  def altidue(self, staticPressure=1013.25):
    if (staticPressure > 0):
      return 44330.8 * (1 - pow(self.pressure() / staticPressure, 0.190263))
    else:
      return 0.0

  #   
  # Compute Depth
  # Saltwater density is 1.025 kg/l
  # Freshwater density is 1.0 kg/l
  # Gravity is 9.80665 m/s/s
  # delta P = rho g h
  # see level pressure in average: 1013.25
  # 1 millibar = 100 pascal [kg/m/s/s]
  # 100/9.8/1.025
  #
  # http://www.seabird.com/document/an69-conversion-pressure-depth
  #
  #The gravity variation with latitude and pressure is computed as:
  # g (m/sec2) = 9.780318 * (1.0 + ( 5.2788E-3  + 2.36E-5  * x) * x ) + 1.092E-6  * p
  #where
  # x = (sin (latitude / 57.29578) ) ^ 2
  # p = pressure (decibars)
  # San Diego: 32.72  p=1013.25/1000/10 x=0.2922 g=9.7954
  #Then, depth is calculated from pressure:
  # depth (meters) = ((((-1.82e-15  * p + 2.279e-10 ) * p - 2.2512e-5 ) * p + 9.72659) * p) / g
  #where
  #p = pressure (decibars)
  #g = gravity (m/sec2)
  #

  def depthSalt(self, staticPressure=1013.25):
    p=(self.pressure() - staticPressure) / 100
    return ((( (-1.82e-15*p + 2.279e-10 )*p - 2.2512e-5 )*p + 9.72659) * p) / 9.7954

  # Depth relative to MSL pressure in given fluid density
  def depth(self, staticPressure=1013.25):
    return (self.pressure(UNITS_Pa)-(staticPressure*100))/(self._fluidDensity*9.80665)
