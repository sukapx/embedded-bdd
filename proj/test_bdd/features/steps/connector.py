from steps.ConnectorSIL import *
from steps.ConnectorPIL import *


def GetConnector(config) -> ConnectorBase:
  if 'TEST_LEVEL_SIL' in config.userdata:
    return ConnectorSIL()
  elif 'TEST_LEVEL_PIL' in config.userdata:
    return ConnectorPIL()

