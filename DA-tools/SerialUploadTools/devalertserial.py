import os, sys, time, argparse

# DevAlert Serial Receiver
# This tool receives DevAlert data provided by the DFM library over a serial port or similar (printf outbut).
# This script doesn't listen to the COM port itself, but assumes that a terminal program is used to log the
# serial output to a text file.
#
# Example usage:
# C:\test-da-tools>type teraterm.log | python devalertserial.py | devalerts3 store-trace -c DUMMY
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/1-1_da_header
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/1-1_da_payload1_header
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/1-4_da_payload1
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/2-4_da_payload1
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/3-4_da_payload1
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/4-4_da_payload1
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/1-1_da_payload2_header
# Saved object from <STDIN> to S3 indes-alert-bucket-2/DevAlert/bd-STM32L475-IoT/3355270496/1/1-1_da_payload2
#
# Example input (from the serial console log)
#
# ### DevAlert Data Begins
# ### DATA: A1 1A F9 9F 36 00 0C 00
# ### DATA: 44 65 76 41 6C 65 72 74
# ### DATA: 2F 62 64 2D 53 54 4D 33 32 4C 34 37 35 2D 49 6F
# ### DATA: 54 2F 33 33 35 35 32 37 30 34 39 36 2F 31 2F 34
# ### DATA: 2D 34 5F 64 61 5F 70 61 79 6C 6F 61 64 31
# ### DATA: 0A 0B
# ### DATA: 0C 0D 71 72 73 74 F1 F2 F3 F4
# ### DevAlert Data Ended. Checksum:6155
#
# The output is a data stream intended for the devalerts3 tool, that does the upload to the Amazon S3 bucket.

useDebugLog = 1

data_bytes = []
errors = 0
bytecount = 0
in_data_block = False
dfmdata = None
upload_command = "";

# The internal debug log for the script
dbglog = open("debug.log","w")

def DebugLog(str):
    if (useDebugLog == 1):
        dbglog.write(str + "\n")

def DebugLog2(str):
    if (useDebugLog == 1):
        dbglog.write(str)

def processDfmData(line, checksum):
    global bytecount
    global errors
    global data_bytes
    global in_data_block 
    global dfmdata
    
    DebugLog("[[ " + line + " ]]")   
    if (line.startswith("DevAlert Data Begins")):        
        DebugLog("Found start of data block")
        data_bytes.clear()
        checksum = 0
        in_data_block = True
    
    if (in_data_block and line.startswith("DATA:")):
        data_str = line.split(":")[1].strip()
        byte_strings = data_str.split()
        
        for byte_string in byte_strings:
            b = int(byte_string, 16) #hex encoded data 
            data_bytes.append(b)
            checksum += b
            bytecount += 1
            
        DebugLog("\n")                
                
    if (in_data_block and line.startswith("DevAlert Data Ended. Checksum:")):
        
        checksum_str = line.split(":")[1].strip()
        checksum_stated = int(checksum_str, 10)
                    
        if (int(checksum_stated) == checksum):
            DebugLog("Checksum correct: " + str(checksum) + ", writing data...\n");            
            os.write(dfmdata.fileno(), bytes(data_bytes))           
            data_bytes.clear()
            DebugLog("")
            
        else:
            
            DebugLog("\nERROR, Invalid data block! Calculated checksum: " + str(checksum) + " Provided: " + checksum_str + "\n")
            sys.stderr.write("\nERROR, Invalid data block! Calculated checksum: " + str(checksum) + " Provided: " + checksum_str + "\n")                                
            data_bytes.clear()
            errors+=1
            
        checksum = 0
        in_data_block = False
    
    return checksum

def main():

    global dfmdata
    global upload_command
    
    #old
    # if ((len(sys.argv) < 3) or len(sys.argv) != 4)):
      #  print("Usage...")
       # exit(0)
        
    # options
    # --file filename
    # --upload sandbox/s3
        
    argParser = argparse.ArgumentParser(description="DevAlert Serial Data Parser and Uploader. Learn more in readme.txt.")
    argParser.add_argument("-f", "--file", help="What file to read the DevAlert data from. This is typically a log file created by a terminal program.")
    argParser.add_argument("-u", "--upload", help="What upload tool to use - 'sandbox' uses the devalerthttps tool that uploads the Percepio-hosted 'sandbox' storage, 's3' used the devalerts3 tool to upload to a private Amazon S3 bucket. Note that the devalerthttps and devalerts3 tools must be configured first according to the instructions in readme.txt.")
    argParser.add_argument("-o", "--outfile", help="Where to write the binary data for the devalert upload tool. STDOUT is used if omitted.")

    args = argParser.parse_args()
    
    args = argParser.parse_args(args=None if sys.argv[1:] else ['--help']) 
    
    if (args.outfile is None):
        dfmdata = sys.stdout
    else:
        dfmdata = open(args.outfile,"w")

    if (args.file is None):
        print("Missing file argument, try -h or --help")
        return
   

    if (args.upload is None):
        print("Missing upload argument, try -h or --help")
        return
   
   
        
    sys.stderr.write("Reading from: " + args.file + "\n")
    
    sys.stderr.write("Uploading to: " + args.upload + "\n")
    
    fd = open(args.file, "r")
    
    DebugLog("fd: " + str(fd))    
    
    chk = 0
    
    done = False
    
    last_errors = 0
    alerts = 0
    empty_reads = 0
    
    ts = 0
    
    sys.stderr.write("Note that the alert count is approximate. What is counted is actually \"data bursts\".\n")
    
    while (not done):
    
        line = fd.readline()
        
        DebugLog("Line: " + line)

        if (line == ""):  # EOF (empty lines will be "\n")
            time.sleep(1)  # reduce load when polling for more data
            
        # Need at least two characters read to always detect "[[" (readline might only return the first one in some cases, if at EOF but more data coming)
        while (line.lstrip().startswith("[") and len(line.lstrip()) < 2):
               time.sleep(0.1)
               line = line + fd.readline().replace("\n", "")
        
        if (line.lstrip().startswith("[[")):
        
            DebugLog("Found [[")
        
            while (not line.rstrip().endswith("]]")):
                time.sleep(0.1)
                line = line + fd.readline().replace("\n", "")
            
            if (args.upload == "s3"):    
                # if more than 1 s since last data, it is counted as a new alert
                if (time.time() - ts > 1):
                    dfmdata.close()
                    os.system("devalerts3.exe store-trace --file data.bin -c DUMMY")
                    dfmdata = open("data.bin","w")
           
            chk = processDfmData(line.replace("[[", "").replace("]]", "").strip(), chk)            

            # Store timestamp of last data. 
            ts = time.time() 
           
            # sys.stderr.write("\rAlerts: " + str(alerts) + ", Bytes: " + str(bytecount) + ", Checksum errors: " + str(errors))
        
main()
