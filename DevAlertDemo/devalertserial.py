#!/usr/bin/python3

# Updated version supporting file output.
# To save files locally, run this with python devalertserial.py --upload file --folder output dfm.log
# (dfm.log is the terminal output log, saved e.g. using TeraTerm logging.) 

import argparse
import os
import re
import sys
import time
from binascii import unhexlify
import enum
from typing import Tuple
from DevAlertCommon import DfmEntryParser, DfmEntryParserException
from pathlib import Path

class ChunkResultType(enum.Enum):
    Start = enum.auto()
    Data = enum.auto()
    End = enum.auto()
    NotDfm = enum.auto()


class DataBlockParseState(enum.Enum):
    NotRunning = enum.auto()
    Parsing = enum.auto()


def crc16_ccit(data: bytearray):
    seed = 0x0000
    for i in range(0, len(data)):
        cur_byte = data[i]
        e = (seed ^ cur_byte) & 0xFF
        f = (e ^ (e << 4)) & 0xFF
        seed = (seed >> 8) ^ (f << 8) ^ (f << 3) ^ (f >> 4)

    return seed


def info_log(message):
    sys.stderr.write("{}\n".format(message))


class DatablockParser:

    @staticmethod
    def process_line(line: str) -> Tuple[ChunkResultType, None or bytes or int]:
        """
        Process a line and return what it was so that the state machine can handle it properly
        :param line:
        :return:
        """

        line = line.strip()
        # Normal log line, ignore
        if not (line.startswith("[[ ") and line.endswith(" ]]")):
            return ChunkResultType.NotDfm, None

        line = re.sub(r"^\[\[\s", "", line)
        line = re.sub(r"\s\]\]$", "", line)

        if line == "DevAlert Data Begins":
            return ChunkResultType.Start, None
        elif line.startswith("DATA: "):
            line = re.sub(r"^DATA:\s", "", line)
            return ChunkResultType.Data, unhexlify(line.replace(" ", ""))
        elif matches := re.match(r'^DevAlert\sData\sEnded.\sChecksum:\s(\d{1,5})', line):
            return ChunkResultType.End, int(matches.group(1))
        else:
            return ChunkResultType.NotDfm, None

class LineParser:

    def __init__(self):
        self.line_buffer = ""
        self.parsing = False

    def process(self, line_data) -> bool:
        stripped_line = line_data.replace("\n", "")
        if not self.parsing:
            self.line_buffer = ""
            if stripped_line.startswith("[[") and stripped_line.endswith("]]"):
                self.line_buffer = stripped_line
                return True
            elif stripped_line.startswith("[["):
                self.line_buffer += stripped_line
                self.parsing = True
                return False
            elif stripped_line.startswith("[") and len(stripped_line) < 2:
                self.line_buffer += stripped_line
                self.parsing = True
                return False
            else:
                return False
        elif self.parsing:
            # Apparently we've encountered a new line
            if stripped_line.startswith("[[") and stripped_line.endswith("]]"):
                self.line_buffer = stripped_line
                self.parsing = False
                return True
            elif stripped_line.startswith("[["):
                self.line_buffer = stripped_line
                return False
            elif stripped_line.endswith("]]"):
                self.line_buffer += stripped_line
                self.parsing = False
                return True
            else:
                self.line_buffer += stripped_line
                # Invalid line which started with [, drop it
                if not self.line_buffer.startswith("[["):
                    self.line_buffer = ""
                    self.parsing = False
                return False


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog='DFM Zephyr Serial cloudport log parser',
        description='Parse the log of the Zephyr DfmSerialCloudPort and dump the coredumps as separate files into a separate folder',
    )
    parser.add_argument('serial_file')
    parser.add_argument('--folder', type=str, help='The folder where the dumps should be outputted (only needed when using the s3 upload destination)')
    parser.add_argument('--upload', type=str, help="Whether to use a separate s3 bucket, sandbox or file, valid options: s3, sandbox, file",  required=True)
    args = parser.parse_args()

    if args.upload != "s3" and args.upload != "sandbox" and args.upload != "file":
        info_log("Invalid upload destination specified: {}".format(args.upload))

    if args.upload == "s3":
        if not os.path.isdir(args.folder):
            info_log("Invalid dump folder specified: {}".format(args.folder))

    binary_name_s3 = "devalerts3"
    if sys.platform == "win32":
        binary_name_s3 = "devalerts3.exe"

    line_parser = LineParser()
    block_parse_state = DataBlockParseState.NotRunning
    accumulated_payload = bytes([])
    with open(args.serial_file, "r", buffering=1) as fh:
        while 1:
            line = fh.readline()

            # We've reached the end of the file, just sleep and try reading again.
            # This makes it possible to continuously read the file
            if line == "":
                if args.upload == "file":
                    # We've reached the end of the file, and we just wanted to do that
                    break
                time.sleep(1)
                continue

            # If parsing in the middle of data being written to file, try to get the whole line
            if not line_parser.process(line):
                continue

            parse_result, payload = DatablockParser.process_line(line_parser.line_buffer)

            if block_parse_state == DataBlockParseState.NotRunning:
                if parse_result == ChunkResultType.Start:
                    block_parse_state = DataBlockParseState.Parsing
                    accumulated_payload = bytes([])
                elif parse_result == ChunkResultType.NotDfm:
                    info_log("Got Notdfm data: {}".format(line_parser.line_buffer))
                    continue
                else:
                    info_log("Got {} without having received a start".format(parse_result.name))
            elif block_parse_state == DataBlockParseState.Parsing:

                if parse_result == ChunkResultType.Data:
                    accumulated_payload += payload
                    # print(' '.join(f'{x:02X}' for x in payload))
                    
                elif parse_result == ChunkResultType.End:
                    block_parse_state = DataBlockParseState.NotRunning

                    if len(accumulated_payload) == 0:
                        info_log("Got empty message")
                    else:
                        # No checksum provided, skip check
                        if payload != 0:
                           calculated_crc = crc16_ccit(bytearray(accumulated_payload))
                           if calculated_crc != payload:
                               info_log("Got crc mismatch, calculated: {}, got: {}, payload length: {}".format(
                                   calculated_crc, payload, len(accumulated_payload)))
                               continue

                        # We got a proper data block, let's send it to the sandbox
                        # info_log("Found DFM data block, length: {}".format(len(accumulated_payload)))

                        if args.upload == "file":                        
                            topic = DfmEntryParser.get_topic(accumulated_payload)
                            file_path = args.folder + "/" + topic;
                            
                            folder = str(Path(file_path).parent.resolve())
                            
                            #info_log("Creating folder " + folder)                            
                            Path(folder).mkdir(parents=True, exist_ok=True)

                            info_log("Creating file: " + file_path)
                                                    
                            with open(file_path, 'wb') as dump_fh:
                               dump_fh.write(accumulated_payload)
                        else:
                            # Try to parse the payload to generate a proper devalerts3/devalerthttps payload
                            parsed_payload: bytes
                            try:
                                parsed_payload = DfmEntryParser.parse(accumulated_payload)
                            except DfmEntryParserException as e:
                                info_log("Got DfmParserException: {}".format(e))
                                continue
						    
                            if args.upload == "s3":
                                with open('{}/dumpfile.bin'.format(args.folder), 'wb') as dump_fh:
                                    dump_fh.write(parsed_payload)
                                os.system('./{} store-trace --file {}/dumpfile.bin'.format(binary_name_s3, args.folder))
                            else:
                                os.write(sys.stdout.fileno(), parsed_payload)

                elif parse_result == ChunkResultType.Start:
                    info_log("Got a start while parsing, resetting payload")
                    accumulated_payload = bytes([])
