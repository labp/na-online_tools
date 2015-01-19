# -*- coding: utf-8 -*-

__version__ = '2.0.0'
__description__ = 'Library to manage plugins/extensions in your applications.'

import os
import logging


class PluginFactory(object):

    logger = logging.getLogger(__name__)

    def __init__(self, clazz):
        self._clazz = clazz

    def __call__(self, *args, **kwargs):
        return self._clazz(*args, **kwargs)


class PluginLoader(object):

    logger = logging.getLogger(__name__)

    def __init__(self):
        self.plugins = {}

    def load_file(self, filename, onlyif=None, context=None):
        try:
            onlyif = self._default_condition if onlyif is None else onlyif
            context = context or {}
            with open(filename) as fd:
                exec(fd.read(), context)

            for name, clazz in context.items():
                if self._apply_condition(onlyif, name, clazz, filename):
                    self.plugins[name] = PluginFactory(clazz)
        except:
            self.logger.exception('Error loading file %s. Ignored' % filename)

    def load_directory(self, path, onlyif=None, recursive=False, context=None):
        for filename in os.listdir(path):
            full_path = os.path.join(path, filename)

            if os.path.isfile(full_path) and full_path.endswith('.py'):
                self.load_file(full_path, onlyif, context)
                continue
            if os.path.isdir(full_path):
                if recursive:
                    self.load_directory(full_path, onlyif, recursive, context)

    def _apply_condition(self, condition, obj_name, class_name, file_name):
        if callable(condition):
            return condition(obj_name, class_name, file_name)
        return condition

    def _default_condition(self, obj_name, class_name, file_name):
        return isinstance(class_name, type)


class BaseClassCondition(object):
    def __init__(self, clazz):
        self.clazz = clazz

    def __call__(self, obj_name, class_name, file_name):
        try:
            instance = PluginFactory(class_name)()
            for b in instance.__class__.__bases__:
                if self.clazz == b:
                    return True
            return False
        except:
            return False