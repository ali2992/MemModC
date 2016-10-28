from distutils.core import setup, Extension

module1 = Extension('MemModOb',
                    sources = ['MemModOb.c'],
                    extra_link_args = ['-std=c99'],
		    extra_compile_args=['-O0'])

setup (name = 'MemModOb',
       version = '1.0',
       description = 'MemModOb',
       ext_modules = [module1])
