import re
import serial
import threading
import os, sys
import time

from steps.ConnectorBase import *


class ConnectionThread(threading.Thread):
  myStopEvent = 0

  def __init__(self,args):
    threading.Thread.__init__(self)
    self.myStopEvent = args
    self.InitTrace()
    self.ser = serial.Serial('COM3', 115200, timeout=10)
    self.config = {}

    self.reg_ctrl_loop = re.compile('\[controlloop\] Itr: (\d+), Enabled: (\d+), Heating: (\d+), Temperature: ([\d\.]+)')
    self.reg_mock_loop = re.compile('\[hw_mock\] Itr: (\d+), Enabled: (\d+), Heating: (\d+), Temperature: ([\d\.]+)')
    self.config = re.compile('config\s+(\d+):\s+([\d\.]+)')

  def Init(self):
    pass

  def write(self, line):
    self.ser.write(f"{line}\n".encode('utf-8'))
  
  def CheckLine_CtrlLoop(self, line) -> bool:
    ctrl_loop=self.reg_ctrl_loop.match(line)
    if ctrl_loop:
      self.traceData["Itr"].append(float(ctrl_loop.group(1)))
      self.traceData["Enabled"].append(float(ctrl_loop.group(2)))
      self.traceData["Heating"].append(float(ctrl_loop.group(3)))
      self.traceData["Temperature"].append(float(ctrl_loop.group(4)))
      return True
    return False

  def CheckLine_MockLoop(self, line) -> bool:
    mock_loop=self.reg_mock_loop.match(line)
    if mock_loop:
      self.traceData["Itr"].append(float(mock_loop.group(1)))
      self.traceData["Enabled"].append(float(mock_loop.group(2)))
      self.traceData["Heating"].append(float(mock_loop.group(3)))
      self.traceData["Temperature"].append(float(mock_loop.group(4)))
      return True
    return False

  def CheckLine_ConfigChange(self, line) -> bool:
    conf = self.config.match(line)
    if conf:
      self.config[conf.group(1)] = float(conf.group(2))
      return True
    return False


  def run(self):
    if not self.ser:
      print("Failed opening Serialport", file=sys.stderr)
    else:
      while not self.myStopEvent.wait(0):
        try:
          line = self.ser.readline().decode('utf-8').replace("\r", "").replace("\n", "")
          print(f"--> {line}")

          if self.CheckLine_CtrlLoop(line):
            continue

          if self.CheckLine_MockLoop(line):
            continue

          if self.CheckLine_ConfigChange(line):
            continue
        except:
          pass

  def GetTrace(self):
    return self.traceData

  def InitTrace(self):
    self.traceData = {
      "Itr": [],
      "Enabled": [],
      "Heating": [],
      "Temperature": []
    }



class ConnectorPIL(ConnectorBase):
  def __init__(self):
    super().__init__()
    print("init ConnectorPIL")

  def Init(self):
    self.aStopEvent = threading.Event()
    self.serialIn = ConnectionThread(self.aStopEvent)
    self.serialIn.start()
    self.serialIn.write("io devicestate HW_MOCK")
    self.serialIn.write("io config 3 1")

  def SetSignal(self, signal, value):
    self.serialIn.write(f"io set {signal} {value}")

  def SetSetting(self, setting, value):
    settid = {
      "temp_min": 1,
      "temp_max": 2
    }
    assert setting in settid
    self.serialIn.write(f"io modcom 0 {settid.get(setting)} {value}")

  def RunFor(self, seconds):
    time.sleep(seconds)

  def GetTrace(self):
    return self.serialIn.GetTrace()

  def Close(self):
    self.aStopEvent.set()
    self.serialIn.join(5)
