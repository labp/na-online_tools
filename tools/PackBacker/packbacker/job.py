__author__ = 'Christof Pieloth'

import logging
import os

from packbacker.errors import ParameterError
from packbacker.installer import Installer
from packbacker.utils import UtilsUI


class Job(object):

    log = logging.getLogger(__name__)

    def __init__(self):
        self._installers = []

    def add_installer(self, installer):
        self._installers.append(installer)

    def execute(self):
        errors = 0
        for i in self._installers:
            if not UtilsUI.ask_for_execute('Install ' + i.label):
                continue

            try:
                if i.install():
                    Job.log.info(i.name + ' executed.')
                else:
                    errors += 1
                    Job.log.error('Error on executing ' + i.name + '!')
            except Exception as ex:
                errors += 1
                Job.log.error('Unknown error:\n' + str(ex))
        return errors

    @staticmethod
    def read_job(fname):
        path = os.path.dirname(os.path.realpath(__file__))
        prototypes = Installer.load_prototypes(os.path.join(path, 'installers'))

        job = None
        try:
            job_file = open(fname, 'r')
        except IOError as err:
            Job.log.critical('Error on reading job file:\n' + str(err))
        else:
            with job_file:
                job = Job()
                for line in job_file:
                    if line[0] == '#':
                        continue
                    for p in prototypes:
                        if p.matches(line):
                            try:
                                params = Job.read_parameter(line)
                                cmd = p.instance(params)
                                job.add_installer(cmd)
                            except ParameterError as err:
                                Job.log.error("Installer '" + p.name + "' is skipped: " + str(err))
                            except Exception as ex:
                                Job.log.critical('Unknown error: \n' + str(ex))
                            continue

        return job

    @staticmethod
    def read_parameter(line):
        params = {}
        i = line.find(': ') + 2
        line = line[i:]
        pairs = line.split(';')
        for pair in pairs:
            pair = pair.strip()
            par = pair.split('=')
            if len(par) == 2:
                params[par[0]] = par[1]
        return params