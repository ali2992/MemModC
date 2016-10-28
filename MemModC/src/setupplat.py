from distutils.core import setup, Extension

module1 = Extension('platform',
                    sources = ['fakeplatform.c'],
                    extra_link_args = ['-std=c99'],
		    extra_compile_args=['-O0'])

setup (name = 'platform',
       version = '1.1',
       description = 'fake platform',
       ext_modules = [module1])
