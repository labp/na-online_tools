__author__ = 'Christof Pieloth'

import os
from subprocess import call

from packbacker.constants import Parameter
from packbacker.errors import ParameterError
from packbacker.utils import Utils
from packbacker.utils import UtilsUI
from packbacker.installer import Installer


class Eigen3(Installer):
    """
    Downloads Eigen3 library with sparse matrix support.
    WWW: http://eigen.tuxfamily.org
    """

    REPO_FOLDER = "eigen322"

    def __init__(self):
        Installer.__init__(self, 'eigen3', 'Eigen version 3')

    @classmethod
    def instance(cls, params):
        installer = Eigen3()
        if Parameter.DEST_DIR in params:
            installer.arg_dest = params[Parameter.DEST_DIR]
        else:
            raise ParameterError(Parameter.DEST_DIR + ' parameter is missing!')
        return installer

    @classmethod
    def prototype(cls):
        return Eigen3()

    def _pre_install(self):
        success = True
        success = success and Utils.check_program("hg", "--version")
        return success

    def _install(self):
        success = True

        if success and UtilsUI.ask_for_execute("Download " + self.name):
            success = success and self.__download()
        if success and UtilsUI.ask_for_execute("Initialize " + self.name):
            success = success and self.__initialize()

        return success

    def _post_install(self):
        include_dir = os.path.join(self.arg_dest, self.REPO_FOLDER)
        UtilsUI.print_env_var('EIGEN3_INCLUDE_DIR', include_dir)
        return True

    def __download(self):
        UtilsUI.print_step_begin("Downloading")
        repo = "https://bitbucket.org/eigen/eigen/"
        repo_dir = os.path.join(self.arg_dest, self.REPO_FOLDER)
        call("hg clone " + repo + " " + repo_dir, shell=True)
        UtilsUI.print_step_end("Downloading")
        return True

    def __initialize(self):
        UtilsUI.print_step_begin("Initializing")
        repo_dir = os.path.join(self.arg_dest, self.REPO_FOLDER)
        os.chdir(repo_dir)
        version = "3.2.2"
        call("hg update " + version, shell=True)
        UtilsUI.print_step_end("Initializing")
        return True