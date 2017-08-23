
from setuptools import setup

NAME = 'pywdbg'

PACKAGES = ['pywdbg', 'pywdbg/K']

DESCRIPTION = "pywdbg --- wdbg's front end for python"

CLASSIFIERS = [
    'Development Status :: 3 - Alpha',
    'Intended Audience :: Developers',
    'Intended Audience :: System Administrators',
    'Intended Audience :: Information Technology',
    'Topic :: Software Development :: Debuggers',
    'License :: OSI Approved :: MIT License',
    'Programming Language :: Python :: 3 :: Only'
]

KEYWORDS = 'debug crack'

AUTHOR = 'luzhlon'

AUTHOR_EMAIL = 'luzhlon@outlook.com'

URL = 'https://github.com/luzhlon/wdbg'

VERSION = "0.2"

LICENSE = "MIT"

INSTALL_REQUIRES = ['u-msgpack-python', 'ptpython', 'prompt_toolkit']

ENTRY_POINTS = {
    'console_scripts': [
        'pywdbg = pywdbg.pywdbg:main',
        'pywdbg64 = pywdbg.pywdbg:main64'
    ]
}

setup(
    name                 = NAME,
    version              = VERSION,
    description          = DESCRIPTION,
    classifiers          = CLASSIFIERS,
    keywords             = KEYWORDS,
    author               = AUTHOR,
    author_email         = AUTHOR_EMAIL,
    url                  = URL,
    license              = LICENSE,
    packages             = PACKAGES,
    include_package_data = True,
    zip_safe             = True,
    install_requires     = INSTALL_REQUIRES,
    entry_points         = ENTRY_POINTS,
    python_requires      = '>=3.3'
)
