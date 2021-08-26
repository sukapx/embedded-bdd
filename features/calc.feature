Feature: Simple calculator

Scenario: run a simple test
  Given start value of 2
  When I add 4
  Then result is 6

Scenario: run a simple test
  Given start value of 2
  When I add -4
  Then result is -2

Scenario: run a simple test
  Given start value of 2
  When I multiply with 4
  Then result is 8
