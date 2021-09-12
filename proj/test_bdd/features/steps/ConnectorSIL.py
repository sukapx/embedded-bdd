from steps.ConnectorBase import *
from ctypes import *
from ctypes import wintypes

LIB_PATH='../Lib/Build/Debug/lib.dll'


class ConnectorSIL(ConnectorBase):
  def __init__(self):
    super().__init__()
    print("init ConnectorSiL")
    self.dll = cdll.LoadLibrary(LIB_PATH)



  def Close(self):
    handle = self.dll._handle
    del self.dll
    kernel32 = WinDLL('kernel32', use_last_error=True)
    kernel32.FreeLibrary.argtypes = [wintypes.HMODULE]
    kernel32.FreeLibrary(handle)
