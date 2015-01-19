__author__ = 'Christof Pieloth'

import logging
import os

from packbacker.pluginloader import BaseClassCondition
from packbacker.pluginloader import PluginLoader
from packbacker.utils import UtilsUI


class Installer(object):
    """Abstract installer with default implementations of pre_install and post_install."""

    def __init__(self, name, label):
        self.__name = name
        self.__label = label
        self.__arg_dest = os.path.expanduser('~')
        self.__log = logging.getLogger(self.__name)

    @property
    def name(self):
        """Short name of the installers."""
        return self.__name

    @property
    def label(self):
        """Long name of the installers."""
        return self.__label

    @property
    def arg_dest(self):
        """Destination directory."""
        return self.__arg_dest

    @arg_dest.setter
    def arg_dest(self, dest):
        self.__arg_dest = os.path.expanduser(dest)

    @property
    def log(self):
        """Logger for this installers."""
        return self.__log

    def _pre_install(self):
        """Is called before the installation. It can be used to check for tools which are required."""
        return True

    def _install(self):
        """Abstract method, implements the installation."""
        self.log.debug('No yet implemented: ' + str(self.name))
        return False

    def _post_install(self):
        """Is called after a successful installation. Can be used to test installation or for user instructions."""
        return True

    def install(self):
        """Starts the installation process."""
        UtilsUI.print_install_begin(self.label)

        try:
            success = self._pre_install()
            if success:
                success = self._install()

            if success:
                success = self._post_install()
        except Exception as ex:
            success = False
            self.log.error("Unexpected error:\n" + str(ex))

        UtilsUI.print_install_end(self.label)
        return success

    @classmethod
    def instance(cls, params):
        """
        Abstract method, returns an initialized instance of a specific command.
        Can throw a ParameterError, if parameters are missing.
        """
        raise Exception('Instance method not implemented for: ' + str(cls))

    @classmethod
    def prototype(cls):
        """Abstract method, returns an instance of a specific command, e.g. for matches() or is_available()"""
        raise Exception('Prototype method not implemented for: ' + str(cls))

    def matches(self, installer):
        """Checks if this command should be used for execution."""
        return installer.lower().startswith(self.name)

    @staticmethod
    def load_prototypes(path):
        """Returns prototypes of all known installers."""
        prototypes = []
        loader = PluginLoader()
        loader.load_directory(path, BaseClassCondition(Installer))
        for k in loader.plugins:
            clazz = loader.plugins[k]
            if callable(clazz):
                prototypes.append(clazz().prototype())

        return prototypes