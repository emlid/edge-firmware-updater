import os, sys, re

# Script for generating path list for 
# all files in this project
# Useful for compile this project as subproject

# Usage: "python gen_paths.py .. > dependecies.pri"

def hidden(filename):
    return filename.startswith('.')

def markdown(filename):
    return filename.endswith('.md')

def pro_user(filename):
    return filename.endswith('.pro.user')

def add_qmake_pwd_prefix(filename):
    return re.sub(r"^../", '$${PWD}/../', filename)

def add_trailing_bslashes(filename):
    return filename + ' \\'

def qmake_abs_path(filename):
    return '$$absolute_path({})'.format(filename)

def qmake_shellpath(filename):
    return '$$shell_path({})'.format(filename)


def get_filename_recursive(path):
    for root, dirs, files in os.walk(path):
        for filename in files:
            if not hidden(filename) and not markdown(filename) and not pro_user(filename):
                joined_path = os.path.join(root, filename)
                res =  qmake_abs_path(
				    add_qmake_pwd_prefix(joined_path)
				)

                print(add_trailing_bslashes(
				    qmake_shellpath(res)
				))


if (len(sys.argv) >= 2):
    print("DEPENDS_SOURCES = \\")
    get_filename_recursive(sys.argv[1])
else: 
    print('too few arguments')

