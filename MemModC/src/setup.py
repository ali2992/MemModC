from distutils.core import setup, Extension

module1 = Extension('MemModC',
                    sources = ['MemModC.c'])

setup (name = 'MemMod',
       version = '1.0',
       description = 'MemMod',
       ext_modules = [module1])
