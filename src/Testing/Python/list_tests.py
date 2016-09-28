import unittest
import sys
DEBUG = True

def list_tests_from(base, path):
    loader = unittest.TestLoader()
    suite = loader.discover(base + path)
    #print("Path: " + path)
    #print("Suite: " + suite)
    for t in suite:
        tests = t._tests
        #print("Tests: " + tests)
        for test in tests:
            #if DEBUG:
                #print("Test: " + test)
            for btest in test._tests:
                btestname = btest.__str__().split()
                #print("Btestname: " + btestname[1] + " -- " + btestname[0])
                print(path + "." + btestname[1][1:-1] + "." + btestname[0])

if __name__ == "__main__": 
  # Include the directories
  list_tests_from(sys.argv[1], "Unit")
  list_tests_from(sys.argv[1], "Regression")
