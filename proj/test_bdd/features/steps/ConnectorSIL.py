from steps.ConnectorBase import *
from ctypes import *
from ctypes import wintypes

LIB_PATH='../../build/codetest/Debug/common_shared.dll'


class ConnectorSIL(ConnectorBase):
  def __init__(self):
    super().__init__()
    print("init ConnectorSiL")
    self.dll = cdll.LoadLibrary(LIB_PATH)


  def Init(self):
    pass

  def SetSignal(self, signal, value):
    sigid = {
      "led2": 1
    }
    assert signal in sigid
    self.dll.SetInput(sigid.get(signal), value)

  def GetSignal(self, signal) -> float:
    pass

  def SetSetting(self, setting, value):
    settid = {
      "temp_min": 1,
      "temp_max": 2
    }
    assert setting in settid
    self.dll.SetSetting(settid.get(setting), value)

  def GetSetting(self, setting) -> float:
    pass


  def RunFor(self, seconds):
    while seconds > 0:
      seconds -= 0.1
      self.dll.DoStep()


  def GetTrace(self):
    return {
      "Itr": [],
      "Enabled": [],
      "Heating": [],
      "Temperature": []
    }


  def Close(self):
    handle = self.dll._handle
    del self.dll
    kernel32 = WinDLL('kernel32', use_last_error=True)
    kernel32.FreeLibrary.argtypes = [wintypes.HMODULE]
    kernel32.FreeLibrary(handle)
