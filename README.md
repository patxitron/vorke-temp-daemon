# Fan speed control for Vorke V1 Plus on Ubuntu 17.10

I recently purchased this mini-PC. After installing Ubuntu 17.10
the fan speed is set to maximum on boot. I've searched for a solution
and found no one.

I found that I can control the fan speed by writting to
```/sys/class/thermal/cooling_device0/cur_state``` an integer value
(in ASCII) from 0 (fan stops) to 11 (fan at full speed).

Also I can read the core temperatures from files
```/sys/class/hwmon/hwmon1/tempX_input``` where X is from 1 to 5.

So I realized that I can write a daemon to control the fan speed
in less time than I can find and test a third party solution.

This is my proposed solution.

## Current status

The daemon is working ok. However the fan speed is constantly changing.

In the future it would be nice to implement a hysteresis so the fan speed
increments as soon as the temperature goes high but not so inmediately
reduce speed when the temperature goes low.
