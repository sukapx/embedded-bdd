Feature: Processor in the Loop

Scenario: Run Enabled 250-750
  Given device
  And set_temp_min is set to 250
  And set_temp_max is set to 750
  And sig_led2 is set to 1
  When running 30s
  Then pass

Scenario: Run Enabled 500-600 
  Given device
  And set_temp_min is set to 500
  And set_temp_max is set to 600
  And sig_led2 is set to 1
  When running 30s
  Then pass

Scenario: Run 2s Disabled
  Given device
  And sig_led2 is set to 0
  When running 10s
  Then pass
