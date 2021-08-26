from behave import *

@given('start value of {value}')
def step_impl(context, value):
  context.value = float(value)

@when('I add {value}')
def step_impl(context, value):
  context.value += float(value)

@when('I multiply with {value}')
def step_impl(context, value):
  context.value *= float(value)

@then('result is {value}')
def step_impl(context, value):
  assert context.value == float(value)
