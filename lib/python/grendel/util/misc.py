""" Utility functions that don't go anywhere else.
If at all possible, avoid putting things in this module.  If
enough code related in some way gets put into this module,
we should refactor it into another module.  Be careful not
to change something that would mess up a user's code, however.
"""
from __future__ import division, print_function
from abc import ABCMeta
from collections import Iterable
from fractions import Fraction
from itertools import product, permutations, combinations
import os
import sys

from grendel.util.metaclasses import SubscriptableClass

__all__ = [
    'r', 'full_path', "ProgressBar"
]

class r(object):
    """ A 'raw' list, by syntactic analogy to the Python raw string literal.

    >>> r[1,2,3]
    [1, 2, 3]
    >>> r[1, ..., 3]
    [1, Ellipsis, 3]
    >>> r[...]
    [Ellipsis]
    >>> r[...,...]
    [Ellipsis, Ellipsis]
    >>> r[...,...,1:3]
    [Ellipsis, Ellipsis, slice(1, 3, None)]
    >>> r[...,...,1:3:5]
    [Ellipsis, Ellipsis, slice(1, 3, 5)]
    >>> r[...,...,1:3:5,'hello']
    [Ellipsis, Ellipsis, slice(1, 3, 5), 'hello']
    >>> r['hello']
    ['hello']

    """
    __metaclass__ = SubscriptableClass

    def __new__(cls, *args, **kwargs):
        raise SyntaxError("the correct syntax for creating 'raw' lists is r[...], not r(...)")

    @classmethod
    def __class_getitem__(cls, args):
        return listify_args(args)

def distinct(*args):
    return all(a != b for a, b in combinations(args, 2))

def is_near_integer(num, cutoff=1e-14):
    """ `True` if and only if `num` is within `cutoff` of an integer (`False` otherwise).

    :Examples:

    >>> is_near_integer(1)
    True
    >>> is_near_integer(-5)
    True
    >>> is_near_integer(-1.2)
    False

    """
    if abs(num - round(num)) < cutoff:
        return True
    else:
        return False

def is_pretty_fraction(num, largest_denominator=9, cutoff=1e-14):
    """
    `True` if a fraction can be constructed with a denominator less
    than or equal to `largest_denominator` and a value that is
    within cutoff of `num` (`False` otherwise).

    """
    if abs(num - Fraction(num).limit_denominator(largest_denominator)) < cutoff:
        return True
    else:
        return False

def full_path(path):
    ret_val = os.path.expanduser(os.path.expandvars(path))
    if not os.path.isabs(ret_val):
        ret_val = os.path.abspath(ret_val)
    return os.path.normpath(ret_val)

def python_version_at_least(*args):
    return sys.version_info >= args
python_minimum_version = python_version_at_least
python_version_geq = python_version_at_least

have_python3 = python_minimum_version((3,))

class ProgressBar(object):
    def __init__(self, max_val, width=80, out=sys.stdout, fill_char='=', indent='', arrow = True):
        self.max_val = max_val
        self.current_percent = 0
        self.current_value = 0
        self.width = width
        self.out = out
        self.fill_char = fill_char
        self.indent = indent
        self.arrow = arrow
        self.printed = False

    def update(self, new_val=None):
        if new_val is not None:
            self.current_value = new_val
        new_percent = int(float(self.current_value/self.max_val)*100)
        if new_percent != self.current_percent:
            self.current_percent = new_percent
            self.print_bar()

    def print_bar(self):
        bar_width = self.width - 7
        filled = int(round(float(self.current_percent)/100.0 * bar_width))
        filler = self.fill_char * filled
        if self.arrow and filled > 0 and filled < bar_width:
            filler = filler[:-1] + '>'
        space = " " * (bar_width - filled)
        if not self.printed:
            self.out.write("{3}[{0}{1}] {2:>3d}%".format(filler, space, self.current_percent, self.indent))
            self.printed = True
        else:
            self.out.write("\r{3}[{0}{1}] {2:>3d}%".format(filler, space, self.current_percent, self.indent))
        self.out.flush()

    def __iadd__(self, other):
        self.current_value += other
        self.update()
        return self

#####################
# Dependent Imports #
#####################

from grendel.util.overloading import listify_args

