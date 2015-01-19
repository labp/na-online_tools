#!/usr/bin/env python3

__author__ = 'Christof Pieloth'

import argparse
from argparse import RawTextHelpFormatter
import signal
import sys

from packbacker.job import Job
from packbacker.utils import UtilsUI


def sigint_handler(signum, frame):
    sys.exit('Installation canceled by user!')

signal.signal(signal.SIGINT, sigint_handler)


def main():
    # Prepare CLI arguments
    parser = argparse.ArgumentParser(formatter_class=RawTextHelpFormatter)
    parser.name = 'PackBacker'
    parser.description = 'PackBacker is a light tool to download and install 3rd party libraries.'
    parser.add_argument("job", help="Job file.")
    parser.epilog = 'PackBacker  Copyright (C) 2014  Christof Pieloth\n' \
                    'This program comes with ABSOLUTELY NO WARRANTY; see LICENSE file.\n' \
                    'This is free software, and you are welcome to redistribute it\n' \
                    'under certain conditions; see LICENSE file.'
    args = parser.parse_args()

    UtilsUI.print('PackBacker started ...')
    # Read job
    job = Job.read_job(args.job)

    # Execute job
    errors = 0
    if job:
        errors += job.execute()
    else:
        UtilsUI.print_error('Could not create job. Cancel installations!')
        errors += 1

    if errors == 0:
        UtilsUI.print('PackBacker finished.')
        return 0
    else:
        UtilsUI.print_error('PackBacker finished with errors: ' + str(errors))
        return 1


if __name__ == '__main__':
    sys.exit(main())
