#pragma once

#ifdef LIBRARY_EXPORTS
# define LIBRARY_API __declspec(dllexport)
#else
# define LIBRARY_API __declspec(dllimport)
#endif

#ifdef __cplusplus 
# define EXT_C extern "C" 
#else
# define EXT_C 
#endif


EXT_C LIBRARY_API void api_say_hello();

EXT_C LIBRARY_API void DoStep();
EXT_C LIBRARY_API void SetSetting(unsigned int setting, int value);
EXT_C LIBRARY_API void SetInput(unsigned int input, float value);
EXT_C LIBRARY_API float GetOutput(unsigned int input);

