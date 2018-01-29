/**
 * @file vorketempdaemon.c
 *
 * Simple daemon for controlling Vorke V1 plus fans.
 *
 * @date 2018-01-29
 * @author Francisco J. Lazur
 *
 * This software is distributed under the MIT License:
 * 
Copyright (c) 2018 Francisco José Lazur

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef _DEFAULT_SOURCE
#   define _DEFAULT_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#   define PATH_MAX 128
#endif



/**
 * Reads the core temperatures and return the higher one.
 *
 * @return Higher core temperature.
 */
static double readTemp()
{
    double result = -100.0;
    int error = 0;
    char path[PATH_MAX];
    for (size_t i = 1; i <= 5; i++) {
        int temp;
        sprintf(path, "/sys/class/hwmon/hwmon1/temp%lu_input", i);
        FILE* temp_file = fopen(path, "r");
        if (temp_file) {
            if (1 == fscanf(temp_file, "%d", &temp)) {
                double dtemp = (double)temp / 1000.0;
                if (dtemp > result) result = dtemp;
            } else {
                error = errno;
            }
            fclose(temp_file);
        } else {
            error = errno;
        }
    }
    if (result <= -99.9) {
        syslog(LOG_ERR, "Can not get any temperature: %s", strerror(error));
        return 120.0;
    }
    return result;
}



/**
 * Sets the fan speed to the desired speed.
 * 
 * @param speed Fan speed from 0 (fan stopped) to 11 (max fan speed).
 */
static void setFanSpeed(uint16_t speed)
{
    FILE* fan_file = fopen("/sys/class/thermal/cooling_device0/cur_state", "w");
    if (fan_file) {
        if (0 > fprintf(fan_file, "%hu\n", speed)) {
            syslog(LOG_ERR, "Can not set the fan speed: %s", strerror(errno));
        }
        fclose(fan_file);
    } else {
        syslog(LOG_ERR, "Can not set the fan speed: %s", strerror(errno));
    }
}



/**
 * Temperature control.
 * 
 * This function sets the fan speed as function of read temperature.
 */
static void tempControl()
{
    double temp = readTemp();
    if (temp <= 45.0) {
        setFanSpeed(0);
    } else if (temp <= 50.0) {
        setFanSpeed(1);
    } else if (temp <= 55.0) {
        setFanSpeed(2);
    } else if (temp <= 60.0) {
        setFanSpeed(3);
    } else if (temp <= 62.0) {
        setFanSpeed(4);
    } else if (temp <= 64.0) {
        setFanSpeed(5);
    } else if (temp <= 66.0) {
        setFanSpeed(6);
    } else if (temp <= 68.0) {
        setFanSpeed(7);
    } else if (temp <= 70.0) {
        setFanSpeed(8);
    } else if (temp <= 72.0) {
        setFanSpeed(9);
    } else if (temp <= 74.0) {
        setFanSpeed(10);
    } else {
        setFanSpeed(11);
    }
}



int main(int argc, char* argv[])
{
    if (argc < 2 || 0 != strcmp(argv[1], "-nd")) {
        pid_t pid = fork();

        if (pid < 0) exit(EXIT_FAILURE);
        if (pid > 0) exit(EXIT_SUCCESS);
        if (setsid() < 0) exit(EXIT_FAILURE);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);

        /* Fork off for the second time*/
        pid = fork();
        if (pid < 0) exit(EXIT_FAILURE);
        if (pid > 0) exit(EXIT_SUCCESS);
        umask(0);
        chdir("/");
        for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--) close(x);
    }
    openlog("tempdaemon", LOG_PID, LOG_DAEMON);
    while(1) {
        tempControl();
        usleep(250000);
    }
    return EXIT_SUCCESS;
}
