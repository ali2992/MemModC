from distutils.core import setup, Extension

module1 = Extension('Example1',
                    sources = ['Example1.c'],
                    extra_link_args = ['-std=c99'])

setup (name = 'Example1',
       version = '1.0',
       description = 'Example1',
       ext_modules = [module1])
