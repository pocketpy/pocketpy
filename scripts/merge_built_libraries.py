import os, sys
assert sys.platform == 'darwin', sys.platform

if len(sys.argv) == 2:
    build_dir = sys.argv[1]
    output_dir = sys.argv[1]
elif len(sys.argv) == 3:
    build_dir = sys.argv[1]
    output_dir = sys.argv[2]
else:
    print('Usage: python merge_built_libraries.py <build_dir> [output_dir]')
    exit(1)

assert os.path.exists(build_dir), build_dir
assert os.path.exists(output_dir), output_dir

archives = []

# get all .a files in build/3rd recursive
for root, dirs, files in os.walk(build_dir):
    for file in files:
        if file.endswith('.a') and file.startswith('lib'):
            archives.append(os.path.join(root, file))

print('Merging the following static libraries:')
for archive in archives:
    print('- ' + archive)

# libtool -static -o libpocketpy.a
output_archive = os.path.join(output_dir, 'libpocketpy.a')
os.system('libtool -static -o {} {}'.format(output_archive, ' '.join(archives)))
