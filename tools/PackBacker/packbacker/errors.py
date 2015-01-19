__author__ = 'Christof Pieloth'


class ParameterError(Exception):
    def __init__(self, msg):
        self._msg = msg

    @property
    def msg(self):
        return self._msg

    def __str__(self):
       return repr(self.msg)