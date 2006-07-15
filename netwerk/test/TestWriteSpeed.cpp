/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "prio.h"
#include "prinrval.h"
#include "prmem.h"
#include <stdio.h>
#include <math.h>

void 
NS_MeanAndStdDev(double n, double sumOfValues, double sumOfSquaredValues,
                 double *meanResult, double *stdDevResult)
{
  double mean = 0.0, var = 0.0, stdDev = 0.0;
  if (n > 0.0 && sumOfValues >= 0) {
    mean = sumOfValues / n;
    double temp = (n * sumOfSquaredValues) - (sumOfValues * sumOfValues);
    if (temp < 0.0 || n <= 1)
      var = 0.0;
    else
      var = temp / (n * (n - 1));
    // for some reason, Windows says sqrt(0.0) is "-1.#J" (?!) so do this:
    stdDev = var != 0.0 ? sqrt(var) : 0.0;
  }
  *meanResult = mean;
  *stdDevResult = stdDev;
}

int
Test(const char* filename, PRInt32 minSize, PRInt32 maxSize, 
     PRInt32 sizeIncrement, PRInt32 iterations)
{
    fprintf(stdout, "      size  write:    mean     stddev      iters  total:    mean     stddev      iters\n");
    for (PRInt32 size = minSize; size <= maxSize; size += sizeIncrement) {
        // create a buffer of stuff to write
        char* buf = (char*)PR_Malloc(size);
        if (buf == NULL)
            return -1;

        // initialize it with a pattern
        PRInt32 i;
        char hex[] = "0123456789ABCDEF";
        for (i = 0; i < size; i++) {
            buf[i] = hex[i & 0xF];
        }

        double writeCount = 0, writeRate = 0, writeRateSquared = 0;
        double totalCount = 0, totalRate = 0, totalRateSquared = 0;
        for (i = 0; i < iterations; i++) {
            PRIntervalTime start = PR_IntervalNow();

            char name[1024];
            sprintf(name, "%s_%d", filename, i);
            PRFileDesc* fd = PR_Open(name, PR_WRONLY | PR_CREATE_FILE | PR_TRUNCATE, 0664);
            if (fd == NULL)
                return -1;

            PRIntervalTime writeStart = PR_IntervalNow();
            PRInt32 rv = PR_Write(fd, buf, size);
            if (rv < 0) return rv;
            if (rv != size) return -1;
            PRIntervalTime writeStop = PR_IntervalNow();

            PRStatus st = PR_Close(fd);
            if (st == PR_FAILURE) return -1; 

            PRIntervalTime stop = PR_IntervalNow();

            PRIntervalTime writeTime = PR_IntervalToMilliseconds(writeStop - writeStart);
            if (writeTime > 0) {
                double wr = size / writeTime;
                writeRate += wr;
                writeRateSquared += wr * wr;
                writeCount++;
            }

            PRIntervalTime totalTime = PR_IntervalToMilliseconds(stop - start);
            if (totalTime > 0) {
                double t = size / totalTime;
                totalRate += t;
                totalRateSquared += t * t;
                totalCount++;
            }
        }

        PR_Free(buf);

        double writeMean, writeStddev;
        double totalMean, totalStddev;
        NS_MeanAndStdDev(writeCount, writeRate, writeRateSquared,
                         &writeMean, &writeStddev);
        NS_MeanAndStdDev(totalCount, totalRate, totalRateSquared,
                         &totalMean, &totalStddev);
        fprintf(stdout, "%10d      %10.2f %10.2f %10d      %10.2f %10.2f %10d\n",
                size, writeMean, writeStddev, (PRInt32)writeCount, 
                totalMean, totalStddev, (PRInt32)totalCount);
    }
    return 0;
}

int
main(int argc, char* argv[])
{
    if (argc != 5) {
        printf("usage: %s <min buf size (K)> <max buf size (K)> <size increment (K)> <iterations>\n", argv[0]);
        return -1;
    }
    Test("y:\\foo",
         atoi(argv[1]) * 1024,
         atoi(argv[2]) * 1024,
         atoi(argv[3]) * 1024,
         atoi(argv[4]));
    return 0;
}
