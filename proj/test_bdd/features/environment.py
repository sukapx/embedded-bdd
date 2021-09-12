from behave import *
from ctypes import *
from ctypes import wintypes

from steps import connector


def before_all(context):
  pass


def before_scenario(context, scenario):
  pass

def after_scenario(context, scenario):
  if context.connector:
    context.connector.Close()

