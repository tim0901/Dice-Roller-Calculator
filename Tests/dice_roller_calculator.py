import ctypes
from ctypes import cdll
import os

# Load the library. Windows needs winmode=0 to be set for this to work
if os.name == 'nt':
    roller = ctypes.CDLL('./dice_roller_calculator1.0.dll', winmode=0)
else:
    roller = ctypes.CDLL('./dice_roller_calculator1.0.so')

class Roller(object):
    def __init__(self):
        self.obj = roller.Lexer_new()
    # Evaluate an expression
    def Roll(self, in_string):
        total = ctypes.c_double(0.0)
        allocation = ctypes.create_string_buffer(128 * 1024)
        roller.Lexer_Lex.restype = ctypes.c_char_p
        out_string = roller.Lexer_Lex(self.obj, bytes(in_string, encoding='utf8'), allocation, 128 * 1024, ctypes.byref(total), ctypes.c_double(0.0))
        return (out_string.decode('utf8'), total.value)
    # This function overrides the random number generator seed for testing purposes.
    # Do not use in production - you will not get random numbers.
    def Roll_Test(self, in_string):
        total = ctypes.c_double(0.0)
        allocation = ctypes.create_string_buffer(128 * 1024)
        roller.Lexer_Lex.restype = ctypes.c_char_p
        out_string = roller.Lexer_Lex(self.obj, bytes(in_string, encoding='utf8'), allocation, 128 * 1024, ctypes.byref(total), ctypes.c_double(1.0))
        return (out_string.decode('utf8'), total.value)