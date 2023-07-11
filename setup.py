from setuptools import setup, find_packages
import os

VERSION = '0.0.1'

lib_folder = os.path.dirname(os.path.realpath(__file__))
requirement_path = lib_folder + '/requirements.txt'
install_requires = []
if os.path.isfile(requirement_path):
    with open(requirement_path) as f:
        install_requires = f.read().splitlines()


setup(
    name='eldorado-environment',
    packages = find_packages(),
    install_requires = install_requires,
    version=VERSION,
    author = "Aapo Kössi",
    author_email = "aapokossi@gmail.com",
    description = "A pettingzoo environment",
    long_description = "A pettingzoo envinronment for the board game \"The Quest for El Dorado\".",
)