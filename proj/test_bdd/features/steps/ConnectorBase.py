class ConnectorBase:
  def __init__(self):
    print("init ConnectorBase")

  def Init(self):
    raise NotImplementedError

  def SetSignal(self, signal, value):
    raise NotImplementedError

  def GetSignal(self, signal) -> float:
    raise NotImplementedError

  def SetSetting(self, setting, value):
    raise NotImplementedError

  def GetSetting(self, setting) -> float:
    raise NotImplementedError

  def RunFor(self, seconds):
    raise NotImplementedError

  def GetTrace(self):
    raise NotImplementedError

  def Close(self):
    raise NotImplementedError

