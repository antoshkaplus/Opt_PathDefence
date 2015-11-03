import sys
import os

command = ("java -jar ./tester.jar -exec ./../bin/romka "
           + " ".join(sys.argv[1:] if len(sys.argv) > 1 else [""]))

os.system(command)