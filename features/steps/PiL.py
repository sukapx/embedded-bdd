from behave import *
import threading, queue
import serial
import os, sys
import time
import re
import json
import matplotlib.pyplot as plt
import numpy as np
import csv

def SafeToCSV(filename, dict):
  try:
    with open(filename, 'w') as csvfile:
      csvfile.writelines("idx" + ",".join(dict.keys()))
      for idx in len(dict[dict.keys()[0]]):
        csvfile.write(f"{idx}");
        for value in dict.keys():
          csvfile.write(f",{dict[value][idx]}")

  except IOError:
    print("I/O error")


def PlotAndStore(filename, data):
  num_stats = len(data.items()) -1
  fig, axs = plt.subplots(num_stats, 1, figsize=(6, 2 * num_stats), sharex=True)
  for item, idx in zip(data.items(), range(num_stats + 1)):
    if idx == 0:
      continue
    axs[idx-1].plot(data["Itr"], item[1])
    axs[idx-1].set_title(item[0], fontsize='small', loc='left')

  #SafeToCSV(f"{filename}.csv", data)
  plt.savefig(f"{filename}.png".replace(" ", "_"))



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



@given('device')
def step_impl(context):
  context.aStopEvent = threading.Event()
  context.serialIn = ConnectionThread(context.aStopEvent)
  context.serialIn.start()
  context.serialIn.write("io devicestate HW_MOCK")
  context.serialIn.write("io config 3 1")

@given('sig_{sig} is set to {value}')
@when('sig_{sig} is set to {value}')
def step_impl(context, sig, value):
  context.serialIn.write(f"io set {sig} {value}")

@given('set_{set} is set to {value}')
@when('set_{set} is set to {value}')
def step_impl(context, set, value):
  settid = {
    "temp_min": 1,
    "temp_max": 2
  }
  assert set in settid
  context.serialIn.write(f"io modcom 0 {settid.get(set)} {value}")


@when('running {value}s')
def step_impl(context, value):
  time.sleep(float(value))

@then('pass')
def step_impl(context):
  if context.serialIn:
    context.aStopEvent.set()
    context.serialIn.join(5)
    
    trace = context.serialIn.GetTrace()
    fname=f'{context.feature.name}__{context.scenario.name}'
    PlotAndStore(fname, trace)


