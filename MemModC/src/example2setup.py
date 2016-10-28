from distutils.core import setup, Extension

module1 = Extension('Example2',
                    sources = ['Example2.c'],
                    extra_link_args = ['-std=c99'],
		    extra_compile_args=['-O0'])

setup (name = 'Example2',
       version = '1.0',
       description = 'Example2',
       ext_modules = [module1])
