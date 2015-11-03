import sys
import os

command = ("java -jar ./tester.jar -exec ./../bin/nika "
           + " ".join(sys.argv[1:] if len(sys.argv) > 1 else [""]))

os.system(command)