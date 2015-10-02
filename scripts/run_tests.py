
import sys
import os

scores_file_name = "./../output/scores.txt"

N = 100
for i in range(N):
    command = ("java -jar ./tester.jar -exec ./../bin/path_defence_online "
           + " ".join(sys.argv[1:] if len(sys.argv) > 1 else [""]) + " > " + scores_file_name)           
    os.system(command)

scores_file = open(scores_file_name) 


