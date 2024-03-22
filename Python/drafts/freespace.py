# Copied from https://gist.github.com/rikonor/74ac150b746dfa09190515268c5bfba4

import shutil

KB = 1024
MB = 1024 * KB
GB = 1024 * MB

free_GB = shutil.disk_usage('/').free / GB

print("{} GB of free space.".format(free_GB))