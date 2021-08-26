#include <stdio.h>

#include <gtest/gtest.h>

#include <heater.h>
#include <MockHardware.h>

struct HeaterSettings {
  float min;
  float max;
};


class TemperatureRegulation : public ::testing::TestWithParam<HeaterSettings> {
 protected:
  MockHardware mock;
  Heater heater;
  bool isEnabled;

  TemperatureRegulation():
    isEnabled(false)
  {}

  void SetUp() override {
  }

  bool WhenMinTemperatureIsReached(float min, size_t maxCycles) {
    for(int iteration = 0; iteration < maxCycles; iteration++) {
      if(mock.GetTemperature() >= min) return true;
      Step();
    }
    return false;
  }

  void Step() {
    heater.SetCurrentTemperature(mock.GetTemperature());
    heater.Process();

    mock.SetHeating(heater.GetHeaterState());
    mock.Process();
  }
};


TEST_P(TemperatureRegulation, Base) {
  HeaterSettings settings = GetParam();
  heater.SetEnabled(true);

  heater.SetConfigTemperatureMin(settings.min);
  heater.SetConfigTemperatureMax(settings.max);

  ASSERT_TRUE(WhenMinTemperatureIsReached(settings.min, 1000));

  for(int iteration = 0; iteration < 1000; iteration++) {
    Step();
    ASSERT_GE(mock.GetTemperature(), settings.min * 0.9F);
    ASSERT_LE(mock.GetTemperature(), settings.max * 1.1F);

    if(iteration%100 == 0) {
      printf("Itr: %d, Enabled: %d, Heating: %d, Temperature: %.3f\n", 
            iteration, isEnabled, heater.GetHeaterState(), mock.GetTemperature());
    }
  }

	EXPECT_EQ(false, false);
}


INSTANTIATE_TEST_CASE_P(
        InRange,
        TemperatureRegulation,
        ::testing::Values(
          HeaterSettings{0.1F, 0.7F},
          HeaterSettings{0.5F, 0.7F},
          HeaterSettings{0.1F, 0.5F}
        ));
