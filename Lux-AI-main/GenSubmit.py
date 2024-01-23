from datetime import datetime
import json
import sys
import os, errno
from subprocess import call
import shutil

team = sys.argv[1] 

def DelTempFile():

    try:
        os.remove(tempjobfilename)
    except OSError:
        pass
    return

tempjobfilename = "jobfiles7z.txt";
jobfilename = team + "_" + datetime.now().strftime('%Y%m%d_%H%M%S') + ".submit"

DelTempFile()

with open('jobfiles.txt','r') as f:
    filelist = f.read().split(';')

    with open('jobfiles7z.txt','w') as fzip:
        for filename in filelist:
            fzip.write(filename + '\n');
    fzip.closed
f.closed

print("7z.exe a" + " -tzip " + jobfilename + " @" + tempjobfilename)

if os.path.isfile(tempjobfilename):
    call(["7z.exe", "a", "-tzip", jobfilename, "@" + tempjobfilename])    

print(jobfilename + " Generated")

DelTempFile()

