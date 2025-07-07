Light Monitor
A command line application for monitoring changes to a simulated lighting system
Build From Source and Run:
1. Make sure the server is running on localhost port 8080

2. Navigate to build folder

3. Run $ cmake --build .

4. The compiled program is called LightMonitor.
Run it with LightMonitor or ./LightMonitor depending on your shell.

5. Basic unit tests are included.
When the program is run it will display the unit tests outcome at the start of the program.
Add the -no-run flag to run without unit test.

Binaries for Ubuntu 24.04:
https://github.com/ander-theo/theobald_light_monitor/blob/main/build/LightMonitor

Libraries Used:
DocTest: https://github.com/doctest/doctest
nlohmann/json: https://github.com/nlohmann/json
yhirose/cpp-httplib: https://github.com/yhirose/cpp-httplib
