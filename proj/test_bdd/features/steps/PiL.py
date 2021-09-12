from behave import *
import os, sys
import time
import json
import matplotlib.pyplot as plt
import numpy as np
import csv
import pandas as pd
from steps import connector


def PlotAndStore(filename, data):
  num_stats = len(data.items()) -1
  fig, axs = plt.subplots(num_stats, 1, figsize=(6, 2 * num_stats), sharex=True)
  for item, idx in zip(data.items(), range(num_stats + 1)):
    if idx == 0:
      continue
    axs[idx-1].plot(data["Itr"], item[1])
    axs[idx-1].set_title(item[0], fontsize='small', loc='left')

  csv = pd.DataFrame(data)
  csv.to_csv(f"{filename}.csv", index=False)
  plt.savefig(f"{filename}.png".replace(" ", "_"))





@given('device')
def step_impl(context):
  context.connector = connector.GetConnector(context.config)
  context.connector.Init()

@given('sig_{sig} is set to {value}')
@when('sig_{sig} is set to {value}')
def step_impl(context, sig, value):
  context.connector.SetSignal(sig, value)

@given('set_{set} is set to {value}')
@when('set_{set} is set to {value}')
def step_impl(context, set, value):
  context.connector.SetSetting(set, int(value))


@when('running {value}s')
def step_impl(context, value):
  context.connector.RunFor(float(value))

@then('pass')
def step_impl(context):
  trace = context.connector.GetTrace()
  fname=f'reports/pil/{context.feature.name}__{context.scenario.name}'
  PlotAndStore(fname, trace)


